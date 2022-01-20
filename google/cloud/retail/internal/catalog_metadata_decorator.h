// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Generated by the Codegen C++ plugin.
// If you make any local changes, they will be lost.
// source: google/cloud/retail/v2/catalog_service.proto

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_RETAIL_INTERNAL_CATALOG_METADATA_DECORATOR_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_RETAIL_INTERNAL_CATALOG_METADATA_DECORATOR_H

#include "google/cloud/retail/internal/catalog_stub.h"
#include "google/cloud/version.h"
#include <memory>
#include <string>

namespace google {
namespace cloud {
namespace retail_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

class CatalogServiceMetadata : public CatalogServiceStub {
 public:
  ~CatalogServiceMetadata() override = default;
  explicit CatalogServiceMetadata(std::shared_ptr<CatalogServiceStub> child);

  StatusOr<google::cloud::retail::v2::ListCatalogsResponse> ListCatalogs(
      grpc::ClientContext& context,
      google::cloud::retail::v2::ListCatalogsRequest const& request) override;

  StatusOr<google::cloud::retail::v2::Catalog> UpdateCatalog(
      grpc::ClientContext& context,
      google::cloud::retail::v2::UpdateCatalogRequest const& request) override;

  Status SetDefaultBranch(
      grpc::ClientContext& context,
      google::cloud::retail::v2::SetDefaultBranchRequest const& request)
      override;

  StatusOr<google::cloud::retail::v2::GetDefaultBranchResponse>
  GetDefaultBranch(grpc::ClientContext& context,
                   google::cloud::retail::v2::GetDefaultBranchRequest const&
                       request) override;

 private:
  void SetMetadata(grpc::ClientContext& context,
                   std::string const& request_params);
  std::shared_ptr<CatalogServiceStub> child_;
  std::string api_client_header_;
};  // CatalogServiceMetadata

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace retail_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_RETAIL_INTERNAL_CATALOG_METADATA_DECORATOR_H
