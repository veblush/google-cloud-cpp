// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "google/cloud/storage/internal/curl_client.h"
#include "google/cloud/storage/internal/bucket_access_control_parser.h"
#include "google/cloud/storage/internal/bucket_metadata_parser.h"
#include "google/cloud/storage/internal/curl_request_builder.h"
#include "google/cloud/storage/internal/generate_message_boundary.h"
#include "google/cloud/storage/internal/hmac_key_metadata_parser.h"
#include "google/cloud/storage/internal/notification_metadata_parser.h"
#include "google/cloud/storage/internal/object_access_control_parser.h"
#include "google/cloud/storage/internal/object_metadata_parser.h"
#include "google/cloud/storage/internal/object_read_streambuf.h"
#include "google/cloud/storage/internal/object_write_streambuf.h"
#include "google/cloud/storage/internal/service_account_parser.h"
#include "google/cloud/storage/version.h"
#include "google/cloud/internal/absl_str_cat_quiet.h"
#include "google/cloud/internal/auth_header_error.h"
#include "google/cloud/internal/getenv.h"
#include "absl/memory/memory.h"
#include "absl/strings/match.h"
#include <sstream>

namespace google {
namespace cloud {
namespace storage {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN
namespace internal {

using ::google::cloud::internal::AuthHeaderError;
using ::google::cloud::internal::CurrentOptions;

namespace {

std::shared_ptr<rest_internal::CurlHandleFactory> CreateHandleFactory(
    Options const& options) {
  auto const pool_size = options.get<ConnectionPoolSizeOption>();
  if (pool_size == 0) {
    return std::make_shared<rest_internal::DefaultCurlHandleFactory>(options);
  }
  return std::make_shared<rest_internal::PooledCurlHandleFactory>(pool_size,
                                                                  options);
}

std::string UrlEscapeString(std::string const& value) {
  CurlHandle handle;
  return std::string(handle.MakeEscapedString(value).get());
}

template <typename ReturnType>
StatusOr<ReturnType> ParseFromString(StatusOr<HttpResponse> response) {
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return ReturnType::ParseFromString(response->payload);
}

template <typename Parser>
auto CheckedFromString(StatusOr<HttpResponse> response)
    -> decltype(Parser::FromString(response->payload)) {
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return Parser::FromString(response->payload);
}

StatusOr<EmptyResponse> ReturnEmptyResponse(StatusOr<HttpResponse> response) {
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return EmptyResponse{};
}

template <typename ReturnType>
StatusOr<ReturnType> ParseFromHttpResponse(StatusOr<HttpResponse> response) {
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return ReturnType::FromHttpResponse(response->payload);
}

}  // namespace

std::string HostHeader(Options const& options, char const* service) {
  // If this function returns an empty string libcurl will fill out the `Host: `
  // header based on the URL. In most cases this is the correct value. The main
  // exception are applications using `VPC-SC`:
  //     https://cloud.google.com/vpc/docs/configure-private-google-access
  // In those cases the application would target a URL like
  // `https://restricted.googleapis.com`, or `https://private.googleapis.com`,
  // or their own proxy, and need to provide the target's service host.
  auto const& auth = options.get<AuthorityOption>();
  if (!auth.empty()) return absl::StrCat("Host: ", auth);
  auto const& endpoint = options.get<RestEndpointOption>();
  if (absl::StrContains(endpoint, "googleapis.com")) {
    return absl::StrCat("Host: ", service, ".googleapis.com");
  }
  return {};
}

Status CurlClient::SetupBuilderCommon(CurlRequestBuilder& builder,
                                      char const* method, char const* service) {
  auto const& current = CurrentOptions();
  auto auth_header =
      current.get<Oauth2CredentialsOption>()->AuthorizationHeader();
  if (!auth_header) return AuthHeaderError(std::move(auth_header).status());
  builder.SetMethod(method)
      .ApplyClientOptions(current)
      .AddHeader(auth_header.value())
      .AddHeader(HostHeader(current, service))
      .AddHeader(x_goog_api_client_header_);
  return Status();
}

template <typename Request>
void SetupBuilderUserIp(CurlRequestBuilder& builder, Request const& request) {
  if (request.template HasOption<UserIp>()) {
    std::string value = request.template GetOption<UserIp>().value();
    if (value.empty()) {
      value = builder.LastClientIpAddress();
    }
    if (!value.empty()) {
      builder.AddQueryParameter(UserIp::name(), value);
    }
  }
}

template <typename Request>
Status CurlClient::SetupBuilder(CurlRequestBuilder& builder,
                                Request const& request, char const* method) {
  auto status = SetupBuilderCommon(builder, method);
  if (!status.ok()) {
    return status;
  }
  request.AddOptionsToHttpRequest(builder);
  SetupBuilderUserIp(builder, request);
  return Status();
}

CurlClient::CurlClient(google::cloud::Options options)
    : opts_(std::move(options)),
      backwards_compatibility_options_(
          MakeBackwardsCompatibleClientOptions(opts_)),
      x_goog_api_client_header_("x-goog-api-client: " + x_goog_api_client()),
      storage_endpoint_(JsonEndpoint(opts_)),
      upload_endpoint_(JsonUploadEndpoint(opts_)),
      xml_endpoint_(XmlEndpoint(opts_)),
      iam_endpoint_(IamEndpoint(opts_)),
      generator_(google::cloud::internal::MakeDefaultPRNG()),
      storage_factory_(CreateHandleFactory(opts_)),
      upload_factory_(CreateHandleFactory(opts_)),
      xml_upload_factory_(CreateHandleFactory(opts_)),
      xml_download_factory_(CreateHandleFactory(opts_)) {
  rest_internal::CurlInitializeOnce(opts_);
}

Options CurlClient::options() const { return opts_; }

StatusOr<ListBucketsResponse> CurlClient::ListBuckets(
    ListBucketsRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b", storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  builder.AddQueryParameter("project", request.project_id());
  return ParseFromHttpResponse<ListBucketsResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<BucketMetadata> CurlClient::CreateBucket(
    CreateBucketRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b", storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) return status;
  builder.AddQueryParameter("project", request.project_id());
  builder.AddHeader("Content-Type: application/json");
  auto response = CheckedFromString<BucketMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.json_payload()));
  // GCS returns a 409 when buckets already exist:
  //     https://cloud.google.com/storage/docs/json_api/v1/status-codes#409-conflict
  // This seems to be the only case where kAlreadyExists is a better match
  // for 409 than kAborted.
  if (!response && response.status().code() == StatusCode::kAborted) {
    return Status(StatusCode::kAlreadyExists, response.status().message(),
                  response.status().error_info());
  }
  return response;
}

StatusOr<BucketMetadata> CurlClient::GetBucketMetadata(
    GetBucketMetadataRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<BucketMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<EmptyResponse> CurlClient::DeleteBucket(
    DeleteBucketRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<BucketMetadata> CurlClient::UpdateBucket(
    UpdateBucketRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.metadata().name(), storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<BucketMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.json_payload()));
}

StatusOr<BucketMetadata> CurlClient::PatchBucket(
    PatchBucketRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PATCH");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<BucketMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

StatusOr<NativeIamPolicy> CurlClient::GetNativeBucketIamPolicy(
    GetBucketIamPolicyRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/iam",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  auto response = std::move(builder).BuildRequest().MakeRequest(std::string{});
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return NativeIamPolicy::CreateFromJson(response->payload);
}

StatusOr<NativeIamPolicy> CurlClient::SetNativeBucketIamPolicy(
    SetNativeBucketIamPolicyRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/iam",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  auto response =
      std::move(builder).BuildRequest().MakeRequest(request.json_payload());
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return NativeIamPolicy::CreateFromJson(response->payload);
}

StatusOr<TestBucketIamPermissionsResponse> CurlClient::TestBucketIamPermissions(
    TestBucketIamPermissionsRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/iam/testPermissions",
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  for (auto const& perm : request.permissions()) {
    builder.AddQueryParameter("permissions", perm);
  }
  auto response = std::move(builder).BuildRequest().MakeRequest(std::string{});
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return TestBucketIamPermissionsResponse::FromHttpResponse(response->payload);
}

StatusOr<BucketMetadata> CurlClient::LockBucketRetentionPolicy(
    LockBucketRetentionPolicyRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/lockRetentionPolicy",
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("content-type: application/json");
  builder.AddHeader("content-length: 0");
  builder.AddOption(IfMetagenerationMatch(request.metageneration()));
  return CheckedFromString<BucketMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectMetadata> CurlClient::InsertObjectMedia(
    InsertObjectMediaRequest const& request) {
  // If the object metadata is specified, then we need to do a multipart upload.
  if (request.HasOption<WithObjectMetadata>()) {
    return InsertObjectMediaMultipart(request);
  }

  // If the application has set an explicit hash value we need to use multipart
  // uploads. `DisableMD5Hash` and `DisableCrc32cChecksum` should not be
  // dependent on each other.
  if (!request.GetOption<DisableMD5Hash>().value_or(false) ||
      !request.GetOption<DisableCrc32cChecksum>().value_or(false) ||
      request.HasOption<MD5HashValue>() ||
      request.HasOption<Crc32cChecksumValue>()) {
    return InsertObjectMediaMultipart(request);
  }

  // Otherwise do a simple upload.
  return InsertObjectMediaSimple(request);
}

StatusOr<ObjectMetadata> CurlClient::CopyObject(
    CopyObjectRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.source_bucket() + "/o/" +
          UrlEscapeString(request.source_object()) + "/copyTo/b/" +
          request.destination_bucket() + "/o/" +
          UrlEscapeString(request.destination_object()),
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  std::string json_payload("{}");
  if (request.HasOption<WithObjectMetadata>()) {
    json_payload = ObjectMetadataJsonForCopy(
                       request.GetOption<WithObjectMetadata>().value())
                       .dump();
  }
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(json_payload));
}

StatusOr<ObjectMetadata> CurlClient::GetObjectMetadata(
    GetObjectMetadataRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" + UrlEscapeString(request.object_name()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<std::unique_ptr<ObjectReadSource>> CurlClient::ReadObject(
    ReadObjectRangeRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" + UrlEscapeString(request.object_name()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  builder.AddQueryParameter("alt", "media");
  if (request.RequiresRangeHeader()) {
    builder.AddHeader(request.RangeHeader());
  }
  if (request.RequiresNoCache()) {
    builder.AddHeader("Cache-Control: no-transform");
  }

  auto download = std::move(builder).BuildDownloadRequest();
  if (!download) return std::move(download).status();
  return std::unique_ptr<ObjectReadSource>(*std::move(download));
}

StatusOr<ListObjectsResponse> CurlClient::ListObjects(
    ListObjectsRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/o",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  builder.AddQueryParameter("pageToken", request.page_token());
  return ParseFromHttpResponse<ListObjectsResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<EmptyResponse> CurlClient::DeleteObject(
    DeleteObjectRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" + UrlEscapeString(request.object_name()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectMetadata> CurlClient::UpdateObject(
    UpdateObjectRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" + UrlEscapeString(request.object_name()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.json_payload()));
}

StatusOr<ObjectMetadata> CurlClient::PatchObject(
    PatchObjectRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" + UrlEscapeString(request.object_name()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PATCH");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

StatusOr<ObjectMetadata> CurlClient::ComposeObject(
    ComposeObjectRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/o/" +
          UrlEscapeString(request.object_name()) + "/compose",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.JsonPayload()));
}

StatusOr<RewriteObjectResponse> CurlClient::RewriteObject(
    RewriteObjectRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.source_bucket() + "/o/" +
          UrlEscapeString(request.source_object()) + "/rewriteTo/b/" +
          request.destination_bucket() + "/o/" +
          UrlEscapeString(request.destination_object()),
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  if (!request.rewrite_token().empty()) {
    builder.AddQueryParameter("rewriteToken", request.rewrite_token());
  }
  builder.AddHeader("Content-Type: application/json");
  std::string json_payload("{}");
  if (request.HasOption<WithObjectMetadata>()) {
    json_payload = ObjectMetadataJsonForRewrite(
                       request.GetOption<WithObjectMetadata>().value())
                       .dump();
  }
  auto response = std::move(builder).BuildRequest().MakeRequest(json_payload);
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  // This one does not use the common "ParseFromHttpResponse" function because
  // it takes different arguments.
  return RewriteObjectResponse::FromHttpResponse(response->payload);
}

StatusOr<CreateResumableUploadResponse> CurlClient::CreateResumableUpload(
    ResumableUploadRequest const& request) {
  CurlRequestBuilder builder(
      upload_endpoint_ + "/b/" + request.bucket_name() + "/o", upload_factory_);
  auto status = SetupBuilderCommon(builder, "POST");
  if (!status.ok()) return status;
  SetupBuilderUserIp(builder, request);
  AddOptionsWithSkip<CurlRequestBuilder, ContentType> no_content_type{builder};
  request.ForEachOption(no_content_type);

  builder.AddQueryParameter("uploadType", "resumable");
  builder.AddHeader("Content-Type: application/json; charset=UTF-8");
  nlohmann::json resource;
  if (request.HasOption<WithObjectMetadata>()) {
    resource = ObjectMetadataJsonForInsert(
        request.GetOption<WithObjectMetadata>().value());
  }
  if (request.HasOption<ContentEncoding>()) {
    resource["contentEncoding"] = request.GetOption<ContentEncoding>().value();
  }
  if (request.HasOption<ContentType>()) {
    resource["contentType"] = request.GetOption<ContentType>().value();
  }
  if (request.HasOption<Crc32cChecksumValue>()) {
    resource["crc32c"] = request.GetOption<Crc32cChecksumValue>().value();
  }
  if (request.HasOption<MD5HashValue>()) {
    resource["md5Hash"] = request.GetOption<MD5HashValue>().value();
  }

  if (resource.empty()) {
    builder.AddQueryParameter("name", request.object_name());
  } else {
    resource["name"] = request.object_name();
  }

  std::string request_payload;
  if (!resource.empty()) request_payload = resource.dump();

  builder.AddHeader("Content-Length: " +
                    std::to_string(request_payload.size()));
  auto response =
      std::move(builder).BuildRequest().MakeRequest(request_payload);
  if (!response.ok()) return std::move(response).status();
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return CreateResumableUploadResponse::FromHttpResponse(*std::move(response));
}

StatusOr<QueryResumableUploadResponse> CurlClient::QueryResumableUpload(
    QueryResumableUploadRequest const& request) {
  CurlRequestBuilder builder(request.upload_session_url(), upload_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) return status;
  builder.AddHeader("Content-Range: bytes */*");
  builder.AddHeader("Content-Type: application/octet-stream");
  builder.AddHeader("Content-Length: 0");
  auto response = std::move(builder).BuildRequest().MakeRequest(std::string{});
  if (!response.ok()) return std::move(response).status();
  if (response->status_code < HttpStatusCode::kMinNotSuccess ||
      response->status_code == HttpStatusCode::kResumeIncomplete) {
    return QueryResumableUploadResponse::FromHttpResponse(*std::move(response));
  }
  return AsStatus(*response);
}

StatusOr<EmptyResponse> CurlClient::DeleteResumableUpload(
    DeleteResumableUploadRequest const& request) {
  CurlRequestBuilder builder(request.upload_session_url(), upload_factory_);
  auto status = SetupBuilderCommon(builder, "DELETE");
  if (!status.ok()) {
    return status;
  }
  auto response = std::move(builder).BuildRequest().MakeRequest(std::string{});
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess &&
      response->status_code != 499) {
    return AsStatus(*response);
  }
  return EmptyResponse{};
}

StatusOr<QueryResumableUploadResponse> CurlClient::UploadChunk(
    UploadChunkRequest const& request) {
  CurlRequestBuilder builder(request.upload_session_url(), upload_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) return status;
  builder.AddHeader(request.RangeHeader());
  builder.AddHeader("Content-Type: application/octet-stream");
  builder.AddHeader("Content-Length: " +
                    std::to_string(request.payload_size()));
  // We need to explicitly disable chunked transfer encoding. libcurl uses is by
  // default (at least in this case), and that wastes bandwidth as the content
  // length is known.
  builder.AddHeader("Transfer-Encoding:");
  auto offset = request.offset();
  for (auto const& b : request.payload()) {
    request.hash_function().Update(offset,
                                   absl::string_view{b.data(), b.size()});
    offset += b.size();
  }
  auto response =
      std::move(builder).BuildRequest().MakeUploadRequest(request.payload());
  if (!response.ok()) return std::move(response).status();
  if (response->status_code < HttpStatusCode::kMinNotSuccess ||
      response->status_code == HttpStatusCode::kResumeIncomplete) {
    return QueryResumableUploadResponse::FromHttpResponse(*std::move(response));
  }
  return AsStatus(*response);
}

StatusOr<ListBucketAclResponse> CurlClient::ListBucketAcl(
    ListBucketAclRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/acl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  auto response = std::move(builder).BuildRequest().MakeRequest(std::string{});
  if (!response.ok()) {
    return std::move(response).status();
  }
  if (response->status_code >= HttpStatusCode::kMinNotSuccess) {
    return AsStatus(*response);
  }
  return internal::ListBucketAclResponse::FromHttpResponse(response->payload);
}

StatusOr<BucketAccessControl> CurlClient::GetBucketAcl(
    GetBucketAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<internal::BucketAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<BucketAccessControl> CurlClient::CreateBucketAcl(
    CreateBucketAclRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/acl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  nlohmann::json object;
  object["entity"] = request.entity();
  object["role"] = request.role();
  return CheckedFromString<internal::BucketAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(object.dump()));
}

StatusOr<EmptyResponse> CurlClient::DeleteBucketAcl(
    DeleteBucketAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<BucketAccessControl> CurlClient::UpdateBucketAcl(
    UpdateBucketAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  nlohmann::json patch;
  patch["entity"] = request.entity();
  patch["role"] = request.role();
  return CheckedFromString<internal::BucketAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(patch.dump()));
}

StatusOr<BucketAccessControl> CurlClient::PatchBucketAcl(
    PatchBucketAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PATCH");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<internal::BucketAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

StatusOr<ListObjectAclResponse> CurlClient::ListObjectAcl(
    ListObjectAclRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/o/" +
          UrlEscapeString(request.object_name()) + "/acl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return ParseFromHttpResponse<ListObjectAclResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::CreateObjectAcl(
    CreateObjectAclRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/o/" +
          UrlEscapeString(request.object_name()) + "/acl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  nlohmann::json object;
  object["entity"] = request.entity();
  object["role"] = request.role();
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(object.dump()));
}

StatusOr<EmptyResponse> CurlClient::DeleteObjectAcl(
    DeleteObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" +
                                 UrlEscapeString(request.object_name()) +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::GetObjectAcl(
    GetObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" +
                                 UrlEscapeString(request.object_name()) +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::UpdateObjectAcl(
    UpdateObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" +
                                 UrlEscapeString(request.object_name()) +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  nlohmann::json object;
  object["entity"] = request.entity();
  object["role"] = request.role();
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(object.dump()));
}

StatusOr<ObjectAccessControl> CurlClient::PatchObjectAcl(
    PatchObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/o/" +
                                 UrlEscapeString(request.object_name()) +
                                 "/acl/" + UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PATCH");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

StatusOr<ListDefaultObjectAclResponse> CurlClient::ListDefaultObjectAcl(
    ListDefaultObjectAclRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/defaultObjectAcl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return ParseFromHttpResponse<ListDefaultObjectAclResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::CreateDefaultObjectAcl(
    CreateDefaultObjectAclRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/b/" + request.bucket_name() + "/defaultObjectAcl",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  nlohmann::json object;
  object["entity"] = request.entity();
  object["role"] = request.role();
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(object.dump()));
}

StatusOr<EmptyResponse> CurlClient::DeleteDefaultObjectAcl(
    DeleteDefaultObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/defaultObjectAcl/" +
                                 UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::GetDefaultObjectAcl(
    GetDefaultObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/defaultObjectAcl/" +
                                 UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectAccessControl> CurlClient::UpdateDefaultObjectAcl(
    UpdateDefaultObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/defaultObjectAcl/" +
                                 UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  nlohmann::json object;
  object["entity"] = request.entity();
  object["role"] = request.role();
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(object.dump()));
}

StatusOr<ObjectAccessControl> CurlClient::PatchDefaultObjectAcl(
    PatchDefaultObjectAclRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/defaultObjectAcl/" +
                                 UrlEscapeString(request.entity()),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PATCH");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<ObjectAccessControlParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

StatusOr<ServiceAccount> CurlClient::GetServiceAccount(
    GetProjectServiceAccountRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/projects/" +
                                 request.project_id() + "/serviceAccount",
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<ServiceAccountParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ListHmacKeysResponse> CurlClient::ListHmacKeys(
    ListHmacKeysRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/projects/" + request.project_id() + "/hmacKeys",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return ParseFromHttpResponse<ListHmacKeysResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<CreateHmacKeyResponse> CurlClient::CreateHmacKey(
    CreateHmacKeyRequest const& request) {
  CurlRequestBuilder builder(
      storage_endpoint_ + "/projects/" + request.project_id() + "/hmacKeys",
      storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddQueryParameter("serviceAccountEmail", request.service_account());
  builder.AddHeader("content-length: 0");
  return ParseFromHttpResponse<CreateHmacKeyResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<EmptyResponse> CurlClient::DeleteHmacKey(
    DeleteHmacKeyRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/projects/" +
                                 request.project_id() + "/hmacKeys/" +
                                 request.access_id(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<HmacKeyMetadata> CurlClient::GetHmacKey(
    GetHmacKeyRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/projects/" +
                                 request.project_id() + "/hmacKeys/" +
                                 request.access_id(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<HmacKeyMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<HmacKeyMetadata> CurlClient::UpdateHmacKey(
    UpdateHmacKeyRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/projects/" +
                                 request.project_id() + "/hmacKeys/" +
                                 request.access_id(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "PUT");
  if (!status.ok()) {
    return status;
  }
  nlohmann::json payload;
  if (!request.resource().state().empty()) {
    payload["state"] = request.resource().state();
  }
  if (!request.resource().etag().empty()) {
    payload["etag"] = request.resource().etag();
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<HmacKeyMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(payload.dump()));
}

StatusOr<SignBlobResponse> CurlClient::SignBlob(
    SignBlobRequest const& request) {
  CurlRequestBuilder builder(iam_endpoint_ + "/projects/-/serviceAccounts/" +
                                 request.service_account() + ":signBlob",
                             storage_factory_);
  auto status = SetupBuilderCommon(builder, "POST", "iamcredentials");
  if (!status.ok()) {
    return status;
  }
  nlohmann::json payload;
  payload["payload"] = request.base64_encoded_blob();
  if (!request.delegates().empty()) {
    payload["delegates"] = request.delegates();
  }
  builder.AddHeader("Content-Type: application/json");
  return ParseFromHttpResponse<SignBlobResponse>(
      std::move(builder).BuildRequest().MakeRequest(payload.dump()));
}

StatusOr<ListNotificationsResponse> CurlClient::ListNotifications(
    ListNotificationsRequest const& request) {
  // Assume the bucket name is validated by the caller.
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/notificationConfigs",
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return ParseFromHttpResponse<ListNotificationsResponse>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<NotificationMetadata> CurlClient::CreateNotification(
    CreateNotificationRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/notificationConfigs",
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  builder.AddHeader("Content-Type: application/json");
  return CheckedFromString<NotificationMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.json_payload()));
}

StatusOr<NotificationMetadata> CurlClient::GetNotification(
    GetNotificationRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/notificationConfigs/" +
                                 request.notification_id(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "GET");
  if (!status.ok()) {
    return status;
  }
  return CheckedFromString<NotificationMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<EmptyResponse> CurlClient::DeleteNotification(
    DeleteNotificationRequest const& request) {
  CurlRequestBuilder builder(storage_endpoint_ + "/b/" + request.bucket_name() +
                                 "/notificationConfigs/" +
                                 request.notification_id(),
                             storage_factory_);
  auto status = SetupBuilder(builder, request, "DELETE");
  if (!status.ok()) {
    return status;
  }
  return ReturnEmptyResponse(
      std::move(builder).BuildRequest().MakeRequest(std::string{}));
}

StatusOr<ObjectMetadata> CurlClient::InsertObjectMediaMultipart(
    InsertObjectMediaRequest const& request) {
  // To perform a multipart upload we need to separate the parts as described
  // in:
  //   https://cloud.google.com/storage/docs/uploading-objects#rest-upload-objects
  // This function is structured as follows:

  // 1. Create a request builder, as done elsewhere in this class, but manually
  //    configure the options because we need to skip one (groan).
  CurlRequestBuilder builder(
      upload_endpoint_ + "/b/" + request.bucket_name() + "/o", upload_factory_);
  auto status = SetupBuilderCommon(builder, "POST");
  if (!status.ok()) return status;
  SetupBuilderUserIp(builder, request);
  AddOptionsWithSkip<CurlRequestBuilder, ContentType> no_content_type{builder};
  request.ForEachOption(no_content_type);

  // 2. create a random separator which is unlikely to exist in the payload.
  auto const boundary = MakeBoundary();
  builder.AddHeader("content-type: multipart/related; boundary=" + boundary);
  builder.AddQueryParameter("uploadType", "multipart");
  builder.AddQueryParameter("name", request.object_name());

  // 3. Perform a streaming upload because computing the size upfront is more
  //    complicated than it is worth.
  std::ostringstream writer;

  nlohmann::json metadata = nlohmann::json::object();
  if (request.HasOption<WithObjectMetadata>()) {
    metadata = ObjectMetadataJsonForInsert(
        request.GetOption<WithObjectMetadata>().value());
  }
  request.hash_function().Update(/*offset=*/0, request.payload());
  auto hashes = storage::internal::FinishHashes(request);
  if (!hashes.crc32c.empty()) metadata["crc32c"] = hashes.crc32c;
  if (!hashes.md5.empty()) metadata["md5Hash"] = hashes.md5;

  std::string crlf = "\r\n";
  std::string marker = "--" + boundary;

  // 4. Format the first part, including the separators and the headers.
  writer << marker << crlf << "content-type: application/json; charset=UTF-8"
         << crlf << crlf << metadata.dump() << crlf << marker << crlf;

  // 5. Format the second part, which includes all the contents and a final
  //    separator.
  if (request.HasOption<ContentType>()) {
    writer << "content-type: " << request.GetOption<ContentType>().value()
           << crlf;
  } else if (metadata.count("contentType") != 0) {
    writer << "content-type: "
           << metadata.value("contentType", "application/octet-stream") << crlf;
  } else {
    writer << "content-type: application/octet-stream" << crlf;
  }
  writer << crlf << request.payload() << crlf << marker << "--" << crlf;

  // 6. Return the results as usual.
  auto contents = std::move(writer).str();
  builder.AddHeader("Content-Length: " + std::to_string(contents.size()));
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(contents));
}

std::string CurlClient::MakeBoundary() {
  std::unique_lock<std::mutex> lk(mu_);
  return GenerateMessageBoundaryCandidate(generator_);
}

StatusOr<ObjectMetadata> CurlClient::InsertObjectMediaSimple(
    InsertObjectMediaRequest const& request) {
  CurlRequestBuilder builder(
      upload_endpoint_ + "/b/" + request.bucket_name() + "/o", upload_factory_);
  auto status = SetupBuilder(builder, request, "POST");
  if (!status.ok()) {
    return status;
  }
  // Set the content type to a sensible value, the application can override this
  // in the options for the request.
  if (!request.HasOption<ContentType>()) {
    builder.AddHeader("content-type: application/octet-stream");
  }
  builder.AddQueryParameter("uploadType", "media");
  builder.AddQueryParameter("name", request.object_name());
  builder.AddHeader("Content-Length: " +
                    std::to_string(request.payload().size()));
  return CheckedFromString<ObjectMetadataParser>(
      std::move(builder).BuildRequest().MakeRequest(request.payload()));
}

}  // namespace internal
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace storage
}  // namespace cloud
}  // namespace google
