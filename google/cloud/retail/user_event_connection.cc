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
// source: google/cloud/retail/v2/user_event_service.proto

#include "google/cloud/retail/user_event_connection.h"
#include "google/cloud/retail/internal/user_event_option_defaults.h"
#include "google/cloud/retail/internal/user_event_stub_factory.h"
#include "google/cloud/retail/user_event_options.h"
#include "google/cloud/background_threads.h"
#include "google/cloud/common_options.h"
#include "google/cloud/grpc_options.h"
#include "google/cloud/internal/async_long_running_operation.h"
#include "google/cloud/internal/retry_loop.h"
#include <memory>

namespace google {
namespace cloud {
namespace retail {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

UserEventServiceConnection::~UserEventServiceConnection() = default;

StatusOr<google::cloud::retail::v2::UserEvent>
UserEventServiceConnection::WriteUserEvent(
    google::cloud::retail::v2::WriteUserEventRequest const&) {
  return Status(StatusCode::kUnimplemented, "not implemented");
}

StatusOr<google::api::HttpBody> UserEventServiceConnection::CollectUserEvent(
    google::cloud::retail::v2::CollectUserEventRequest const&) {
  return Status(StatusCode::kUnimplemented, "not implemented");
}

future<StatusOr<google::cloud::retail::v2::PurgeUserEventsResponse>>
UserEventServiceConnection::PurgeUserEvents(
    google::cloud::retail::v2::PurgeUserEventsRequest const&) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::retail::v2::PurgeUserEventsResponse>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

future<StatusOr<google::cloud::retail::v2::ImportUserEventsResponse>>
UserEventServiceConnection::ImportUserEvents(
    google::cloud::retail::v2::ImportUserEventsRequest const&) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::retail::v2::ImportUserEventsResponse>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

future<StatusOr<google::cloud::retail::v2::RejoinUserEventsResponse>>
UserEventServiceConnection::RejoinUserEvents(
    google::cloud::retail::v2::RejoinUserEventsRequest const&) {
  return google::cloud::make_ready_future<
      StatusOr<google::cloud::retail::v2::RejoinUserEventsResponse>>(
      Status(StatusCode::kUnimplemented, "not implemented"));
}

namespace {
class UserEventServiceConnectionImpl : public UserEventServiceConnection {
 public:
  UserEventServiceConnectionImpl(
      std::unique_ptr<google::cloud::BackgroundThreads> background,
      std::shared_ptr<retail_internal::UserEventServiceStub> stub,
      Options const& options)
      : background_(std::move(background)),
        stub_(std::move(stub)),
        retry_policy_prototype_(
            options.get<UserEventServiceRetryPolicyOption>()->clone()),
        backoff_policy_prototype_(
            options.get<UserEventServiceBackoffPolicyOption>()->clone()),
        polling_policy_prototype_(
            options.get<UserEventServicePollingPolicyOption>()->clone()),
        idempotency_policy_(
            options.get<UserEventServiceConnectionIdempotencyPolicyOption>()
                ->clone()) {}

  ~UserEventServiceConnectionImpl() override = default;

  StatusOr<google::cloud::retail::v2::UserEvent> WriteUserEvent(
      google::cloud::retail::v2::WriteUserEventRequest const& request)
      override {
    return google::cloud::internal::RetryLoop(
        retry_policy(), backoff_policy(),
        idempotency_policy()->WriteUserEvent(request),
        [this](
            grpc::ClientContext& context,
            google::cloud::retail::v2::WriteUserEventRequest const& request) {
          return stub_->WriteUserEvent(context, request);
        },
        request, __func__);
  }

  StatusOr<google::api::HttpBody> CollectUserEvent(
      google::cloud::retail::v2::CollectUserEventRequest const& request)
      override {
    return google::cloud::internal::RetryLoop(
        retry_policy(), backoff_policy(),
        idempotency_policy()->CollectUserEvent(request),
        [this](
            grpc::ClientContext& context,
            google::cloud::retail::v2::CollectUserEventRequest const& request) {
          return stub_->CollectUserEvent(context, request);
        },
        request, __func__);
  }

  future<StatusOr<google::cloud::retail::v2::PurgeUserEventsResponse>>
  PurgeUserEvents(google::cloud::retail::v2::PurgeUserEventsRequest const&
                      request) override {
    auto stub = stub_;
    return google::cloud::internal::AsyncLongRunningOperation<
        google::cloud::retail::v2::PurgeUserEventsResponse>(
        background_->cq(), request,
        [stub](
            google::cloud::CompletionQueue& cq,
            std::unique_ptr<grpc::ClientContext> context,
            google::cloud::retail::v2::PurgeUserEventsRequest const& request) {
          return stub->AsyncPurgeUserEvents(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::GetOperationRequest const& request) {
          return stub->AsyncGetOperation(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::CancelOperationRequest const& request) {
          return stub->AsyncCancelOperation(cq, std::move(context), request);
        },
        &google::cloud::internal::ExtractLongRunningResultResponse<
            google::cloud::retail::v2::PurgeUserEventsResponse>,
        retry_policy(), backoff_policy(),
        idempotency_policy()->PurgeUserEvents(request), polling_policy(),
        __func__);
  }

  future<StatusOr<google::cloud::retail::v2::ImportUserEventsResponse>>
  ImportUserEvents(google::cloud::retail::v2::ImportUserEventsRequest const&
                       request) override {
    auto stub = stub_;
    return google::cloud::internal::AsyncLongRunningOperation<
        google::cloud::retail::v2::ImportUserEventsResponse>(
        background_->cq(), request,
        [stub](
            google::cloud::CompletionQueue& cq,
            std::unique_ptr<grpc::ClientContext> context,
            google::cloud::retail::v2::ImportUserEventsRequest const& request) {
          return stub->AsyncImportUserEvents(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::GetOperationRequest const& request) {
          return stub->AsyncGetOperation(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::CancelOperationRequest const& request) {
          return stub->AsyncCancelOperation(cq, std::move(context), request);
        },
        &google::cloud::internal::ExtractLongRunningResultResponse<
            google::cloud::retail::v2::ImportUserEventsResponse>,
        retry_policy(), backoff_policy(),
        idempotency_policy()->ImportUserEvents(request), polling_policy(),
        __func__);
  }

  future<StatusOr<google::cloud::retail::v2::RejoinUserEventsResponse>>
  RejoinUserEvents(google::cloud::retail::v2::RejoinUserEventsRequest const&
                       request) override {
    auto stub = stub_;
    return google::cloud::internal::AsyncLongRunningOperation<
        google::cloud::retail::v2::RejoinUserEventsResponse>(
        background_->cq(), request,
        [stub](
            google::cloud::CompletionQueue& cq,
            std::unique_ptr<grpc::ClientContext> context,
            google::cloud::retail::v2::RejoinUserEventsRequest const& request) {
          return stub->AsyncRejoinUserEvents(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::GetOperationRequest const& request) {
          return stub->AsyncGetOperation(cq, std::move(context), request);
        },
        [stub](google::cloud::CompletionQueue& cq,
               std::unique_ptr<grpc::ClientContext> context,
               google::longrunning::CancelOperationRequest const& request) {
          return stub->AsyncCancelOperation(cq, std::move(context), request);
        },
        &google::cloud::internal::ExtractLongRunningResultResponse<
            google::cloud::retail::v2::RejoinUserEventsResponse>,
        retry_policy(), backoff_policy(),
        idempotency_policy()->RejoinUserEvents(request), polling_policy(),
        __func__);
  }

 private:
  std::unique_ptr<UserEventServiceRetryPolicy> retry_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<UserEventServiceRetryPolicyOption>()) {
      return options.get<UserEventServiceRetryPolicyOption>()->clone();
    }
    return retry_policy_prototype_->clone();
  }

  std::unique_ptr<BackoffPolicy> backoff_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<UserEventServiceBackoffPolicyOption>()) {
      return options.get<UserEventServiceBackoffPolicyOption>()->clone();
    }
    return backoff_policy_prototype_->clone();
  }

  std::unique_ptr<PollingPolicy> polling_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<UserEventServicePollingPolicyOption>()) {
      return options.get<UserEventServicePollingPolicyOption>()->clone();
    }
    return polling_policy_prototype_->clone();
  }

  std::unique_ptr<UserEventServiceConnectionIdempotencyPolicy>
  idempotency_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<UserEventServiceConnectionIdempotencyPolicyOption>()) {
      return options.get<UserEventServiceConnectionIdempotencyPolicyOption>()
          ->clone();
    }
    return idempotency_policy_->clone();
  }

  std::unique_ptr<google::cloud::BackgroundThreads> background_;
  std::shared_ptr<retail_internal::UserEventServiceStub> stub_;
  std::unique_ptr<UserEventServiceRetryPolicy const> retry_policy_prototype_;
  std::unique_ptr<BackoffPolicy const> backoff_policy_prototype_;
  std::unique_ptr<PollingPolicy const> polling_policy_prototype_;
  std::unique_ptr<UserEventServiceConnectionIdempotencyPolicy>
      idempotency_policy_;
};
}  // namespace

std::shared_ptr<UserEventServiceConnection> MakeUserEventServiceConnection(
    Options options) {
  internal::CheckExpectedOptions<CommonOptionList, GrpcOptionList,
                                 UserEventServicePolicyOptionList>(options,
                                                                   __func__);
  options = retail_internal::UserEventServiceDefaultOptions(std::move(options));
  auto background = internal::MakeBackgroundThreadsFactory(options)();
  auto stub = retail_internal::CreateDefaultUserEventServiceStub(
      background->cq(), options);
  return std::make_shared<UserEventServiceConnectionImpl>(
      std::move(background), std::move(stub), options);
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace retail
}  // namespace cloud
}  // namespace google

namespace google {
namespace cloud {
namespace retail_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

std::shared_ptr<retail::UserEventServiceConnection>
MakeUserEventServiceConnection(std::shared_ptr<UserEventServiceStub> stub,
                               Options options) {
  options = UserEventServiceDefaultOptions(std::move(options));
  return std::make_shared<retail::UserEventServiceConnectionImpl>(
      internal::MakeBackgroundThreadsFactory(options)(), std::move(stub),
      std::move(options));
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace retail_internal
}  // namespace cloud
}  // namespace google
