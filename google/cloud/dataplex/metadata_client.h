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
// source: google/cloud/dataplex/v1/metadata.proto

#ifndef GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAPLEX_METADATA_CLIENT_H
#define GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAPLEX_METADATA_CLIENT_H

#include "google/cloud/dataplex/metadata_connection.h"
#include "google/cloud/future.h"
#include "google/cloud/options.h"
#include "google/cloud/polling_policy.h"
#include "google/cloud/status_or.h"
#include "google/cloud/version.h"
#include <memory>

namespace google {
namespace cloud {
namespace dataplex {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

///
/// Metadata service manages metadata resources such as tables, filesets and
/// partitions.
///
/// @par Equality
///
/// Instances of this class created via copy-construction or copy-assignment
/// always compare equal. Instances created with equal
/// `std::shared_ptr<*Connection>` objects compare equal. Objects that compare
/// equal share the same underlying resources.
///
/// @par Performance
///
/// Creating a new instance of this class is a relatively expensive operation,
/// new objects establish new connections to the service. In contrast,
/// copy-construction, move-construction, and the corresponding assignment
/// operations are relatively efficient as the copies share all underlying
/// resources.
///
/// @par Thread Safety
///
/// Concurrent access to different instances of this class, even if they compare
/// equal, is guaranteed to work. Two or more threads operating on the same
/// instance of this class is not guaranteed to work. Since copy-construction
/// and move-construction is a relatively efficient operation, consider using
/// such a copy when using this class from multiple threads.
///
class MetadataServiceClient {
 public:
  explicit MetadataServiceClient(
      std::shared_ptr<MetadataServiceConnection> connection, Options opts = {});
  ~MetadataServiceClient();

  ///@{
  // @name Copy and move support
  MetadataServiceClient(MetadataServiceClient const&) = default;
  MetadataServiceClient& operator=(MetadataServiceClient const&) = default;
  MetadataServiceClient(MetadataServiceClient&&) = default;
  MetadataServiceClient& operator=(MetadataServiceClient&&) = default;
  ///@}

  ///@{
  // @name Equality
  friend bool operator==(MetadataServiceClient const& a,
                         MetadataServiceClient const& b) {
    return a.connection_ == b.connection_;
  }
  friend bool operator!=(MetadataServiceClient const& a,
                         MetadataServiceClient const& b) {
    return !(a == b);
  }
  ///@}

  ///
  /// Create a metadata entity.
  ///
  /// @param parent  Required. The resource name of the parent zone:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}`.
  /// @param entity  Required. Entity resource.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.CreateEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L114}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StatusOr<google::cloud::dataplex::v1::Entity> CreateEntity(
      std::string const& parent,
      google::cloud::dataplex::v1::Entity const& entity, Options opts = {});

  ///
  /// Create a metadata entity.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::CreateEntityRequest,google/cloud/dataplex/v1/metadata.proto#L114}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.CreateEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L114}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StatusOr<google::cloud::dataplex::v1::Entity> CreateEntity(
      google::cloud::dataplex::v1::CreateEntityRequest const& request,
      Options opts = {});

  ///
  /// Update a metadata entity. Only supports full resource update.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::UpdateEntityRequest,google/cloud/dataplex/v1/metadata.proto#L134}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.UpdateEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L134}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StatusOr<google::cloud::dataplex::v1::Entity> UpdateEntity(
      google::cloud::dataplex::v1::UpdateEntityRequest const& request,
      Options opts = {});

  ///
  /// Delete a metadata entity.
  ///
  /// @param name  Required. The resource name of the entity:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}`.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  ///
  /// [google.cloud.dataplex.v1.DeleteEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L144}
  ///
  Status DeleteEntity(std::string const& name, Options opts = {});

  ///
  /// Delete a metadata entity.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::DeleteEntityRequest,google/cloud/dataplex/v1/metadata.proto#L144}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  ///
  /// [google.cloud.dataplex.v1.DeleteEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L144}
  ///
  Status DeleteEntity(
      google::cloud::dataplex::v1::DeleteEntityRequest const& request,
      Options opts = {});

  ///
  /// Get a metadata entity.
  ///
  /// @param name  Required. The resource name of the entity:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}.`
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.GetEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L216}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StatusOr<google::cloud::dataplex::v1::Entity> GetEntity(
      std::string const& name, Options opts = {});

  ///
  /// Get a metadata entity.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::GetEntityRequest,google/cloud/dataplex/v1/metadata.proto#L216}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.GetEntityRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L216}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StatusOr<google::cloud::dataplex::v1::Entity> GetEntity(
      google::cloud::dataplex::v1::GetEntityRequest const& request,
      Options opts = {});

  ///
  /// List metadata entities in a zone.
  ///
  /// @param parent  Required. The resource name of the parent zone:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}`.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.ListEntitiesRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L158}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StreamRange<google::cloud::dataplex::v1::Entity> ListEntities(
      std::string const& parent, Options opts = {});

  ///
  /// List metadata entities in a zone.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::ListEntitiesRequest,google/cloud/dataplex/v1/metadata.proto#L158}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Entity,google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  /// [google.cloud.dataplex.v1.ListEntitiesRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L158}
  /// [google.cloud.dataplex.v1.Entity]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L347}
  ///
  StreamRange<google::cloud::dataplex::v1::Entity> ListEntities(
      google::cloud::dataplex::v1::ListEntitiesRequest request,
      Options opts = {});

  ///
  /// Create a metadata partition.
  ///
  /// @param parent  Required. The resource name of the parent zone:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}`.
  /// @param partition  Required. Partition resource.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.CreatePartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L288}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StatusOr<google::cloud::dataplex::v1::Partition> CreatePartition(
      std::string const& parent,
      google::cloud::dataplex::v1::Partition const& partition,
      Options opts = {});

  ///
  /// Create a metadata partition.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::CreatePartitionRequest,google/cloud/dataplex/v1/metadata.proto#L288}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.CreatePartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L288}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StatusOr<google::cloud::dataplex::v1::Partition> CreatePartition(
      google::cloud::dataplex::v1::CreatePartitionRequest const& request,
      Options opts = {});

  ///
  /// Delete a metadata partition.
  ///
  /// @param name  Required. The resource name of the partition.
  ///  format:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}/partitions/{partition_value_path}`.
  ///  The {partition_value_path} segment consists of an ordered sequence of
  ///  partition values separated by "/". All values must be provided.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  ///
  /// [google.cloud.dataplex.v1.DeletePartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L305}
  ///
  Status DeletePartition(std::string const& name, Options opts = {});

  ///
  /// Delete a metadata partition.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::DeletePartitionRequest,google/cloud/dataplex/v1/metadata.proto#L305}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  ///
  /// [google.cloud.dataplex.v1.DeletePartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L305}
  ///
  Status DeletePartition(
      google::cloud::dataplex::v1::DeletePartitionRequest const& request,
      Options opts = {});

  ///
  /// Get a metadata partition of an entity.
  ///
  /// @param name  Required. The resource name of the partition:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}/partitions/{partition_value_path}`.
  ///  The {partition_value_path} segment consists of an ordered sequence of
  ///  partition values separated by "/". All values must be provided.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.GetPartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L333}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StatusOr<google::cloud::dataplex::v1::Partition> GetPartition(
      std::string const& name, Options opts = {});

  ///
  /// Get a metadata partition of an entity.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::GetPartitionRequest,google/cloud/dataplex/v1/metadata.proto#L333}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.GetPartitionRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L333}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StatusOr<google::cloud::dataplex::v1::Partition> GetPartition(
      google::cloud::dataplex::v1::GetPartitionRequest const& request,
      Options opts = {});

  ///
  /// List metadata partitions of an entity.
  ///
  /// @param parent  Required. The resource name of the parent entity:
  ///  `projects/{project_number}/locations/{location_id}/lakes/{lake_id}/zones/{zone_id}/entities/{entity_id}`.
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.ListPartitionsRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L245}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StreamRange<google::cloud::dataplex::v1::Partition> ListPartitions(
      std::string const& parent, Options opts = {});

  ///
  /// List metadata partitions of an entity.
  ///
  /// @param request
  /// @googleapis_link{google::cloud::dataplex::v1::ListPartitionsRequest,google/cloud/dataplex/v1/metadata.proto#L245}
  /// @param opts Optional. Override the class-level options, such as retry and
  ///     backoff policies.
  /// @return
  /// @googleapis_link{google::cloud::dataplex::v1::Partition,google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  /// [google.cloud.dataplex.v1.ListPartitionsRequest]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L245}
  /// [google.cloud.dataplex.v1.Partition]:
  /// @googleapis_reference_link{google/cloud/dataplex/v1/metadata.proto#L473}
  ///
  StreamRange<google::cloud::dataplex::v1::Partition> ListPartitions(
      google::cloud::dataplex::v1::ListPartitionsRequest request,
      Options opts = {});

 private:
  std::shared_ptr<MetadataServiceConnection> connection_;
  Options options_;
};

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace dataplex
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GOOGLE_CLOUD_DATAPLEX_METADATA_CLIENT_H
