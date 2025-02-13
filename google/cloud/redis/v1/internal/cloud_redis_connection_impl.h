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
// source: google/cloud/redis/v1/cloud_redis.proto

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_REDIS_V1_INTERNAL_CLOUD_REDIS_CONNECTION_IMPL_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_REDIS_V1_INTERNAL_CLOUD_REDIS_CONNECTION_IMPL_H

#include "google/cloud/redis/v1/cloud_redis_connection.h"
#include "google/cloud/redis/v1/cloud_redis_connection_idempotency_policy.h"
#include "google/cloud/redis/v1/cloud_redis_options.h"
#include "google/cloud/redis/v1/internal/cloud_redis_retry_traits.h"
#include "google/cloud/redis/v1/internal/cloud_redis_stub.h"
#include "google/cloud/background_threads.h"
#include "google/cloud/backoff_policy.h"
#include "google/cloud/future.h"
#include "google/cloud/options.h"
#include "google/cloud/polling_policy.h"
#include "google/cloud/status_or.h"
#include "google/cloud/stream_range.h"
#include "google/cloud/version.h"
#include <google/longrunning/operations.grpc.pb.h>
#include <memory>

namespace google {
namespace cloud {
namespace redis_v1_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

class CloudRedisConnectionImpl : public redis_v1::CloudRedisConnection {
 public:
  ~CloudRedisConnectionImpl() override = default;

  CloudRedisConnectionImpl(
      std::unique_ptr<google::cloud::BackgroundThreads> background,
      std::shared_ptr<redis_v1_internal::CloudRedisStub> stub, Options options);

  Options options() override { return options_; }

  StreamRange<google::cloud::redis::v1::Instance> ListInstances(
      google::cloud::redis::v1::ListInstancesRequest request) override;

  StatusOr<google::cloud::redis::v1::Instance> GetInstance(
      google::cloud::redis::v1::GetInstanceRequest const& request) override;

  StatusOr<google::cloud::redis::v1::InstanceAuthString> GetInstanceAuthString(
      google::cloud::redis::v1::GetInstanceAuthStringRequest const& request)
      override;

  future<StatusOr<google::cloud::redis::v1::Instance>> CreateInstance(
      google::cloud::redis::v1::CreateInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> UpdateInstance(
      google::cloud::redis::v1::UpdateInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> UpgradeInstance(
      google::cloud::redis::v1::UpgradeInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> ImportInstance(
      google::cloud::redis::v1::ImportInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> ExportInstance(
      google::cloud::redis::v1::ExportInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> FailoverInstance(
      google::cloud::redis::v1::FailoverInstanceRequest const& request)
      override;

  future<StatusOr<google::cloud::redis::v1::OperationMetadata>> DeleteInstance(
      google::cloud::redis::v1::DeleteInstanceRequest const& request) override;

  future<StatusOr<google::cloud::redis::v1::Instance>> RescheduleMaintenance(
      google::cloud::redis::v1::RescheduleMaintenanceRequest const& request)
      override;

 private:
  std::unique_ptr<redis_v1::CloudRedisRetryPolicy> retry_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<redis_v1::CloudRedisRetryPolicyOption>()) {
      return options.get<redis_v1::CloudRedisRetryPolicyOption>()->clone();
    }
    return options_.get<redis_v1::CloudRedisRetryPolicyOption>()->clone();
  }

  std::unique_ptr<BackoffPolicy> backoff_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<redis_v1::CloudRedisBackoffPolicyOption>()) {
      return options.get<redis_v1::CloudRedisBackoffPolicyOption>()->clone();
    }
    return options_.get<redis_v1::CloudRedisBackoffPolicyOption>()->clone();
  }

  std::unique_ptr<redis_v1::CloudRedisConnectionIdempotencyPolicy>
  idempotency_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<redis_v1::CloudRedisConnectionIdempotencyPolicyOption>()) {
      return options
          .get<redis_v1::CloudRedisConnectionIdempotencyPolicyOption>()
          ->clone();
    }
    return options_
        .get<redis_v1::CloudRedisConnectionIdempotencyPolicyOption>()
        ->clone();
  }

  std::unique_ptr<PollingPolicy> polling_policy() {
    auto const& options = internal::CurrentOptions();
    if (options.has<redis_v1::CloudRedisPollingPolicyOption>()) {
      return options.get<redis_v1::CloudRedisPollingPolicyOption>()->clone();
    }
    return options_.get<redis_v1::CloudRedisPollingPolicyOption>()->clone();
  }

  std::unique_ptr<google::cloud::BackgroundThreads> background_;
  std::shared_ptr<redis_v1_internal::CloudRedisStub> stub_;
  Options options_;
};

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace redis_v1_internal
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_REDIS_V1_INTERNAL_CLOUD_REDIS_CONNECTION_IMPL_H
