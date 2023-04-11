// Copyright 2023 Google LLC
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

#include "google/cloud/storage/internal/grpc_split_write_object_data.h"
#include <google/storage/v2/storage.pb.h>
#include <algorithm>

namespace google {
namespace cloud {
namespace storage_internal {
GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_BEGIN

template <>
std::string SplitObjectWriteData<std::string>::Next() {
  auto constexpr kMax = static_cast<std::size_t>(
      google::storage::v2::ServiceConstants::MAX_WRITE_CHUNK_BYTES);
  std::string result;
  auto start = offset_;
  for (auto const& b : buffers_) {
    if (start >= b.size()) {
      start -= b.size();
      continue;
    }
    auto n = (std::min)(kMax - result.size(), b.size() - start);
    result.append(b.begin() + start, b.begin() + start + n);
    offset_ += n;
    start = 0;
    if (result.size() >= kMax) return result;
  }
  return result;
}

template <>
absl::Cord SplitObjectWriteData<absl::Cord>::Next() {
  auto constexpr kMax = static_cast<std::size_t>(
      google::storage::v2::ServiceConstants::MAX_WRITE_CHUNK_BYTES);
  absl::Cord result;
  auto start = offset_;
  for (auto const& b : buffers_) {
    if (start >= b.size()) {
      start -= b.size();
      continue;
    }
    auto n = (std::min)(kMax - result.size(), b.size() - start);
    if (n != 0) {
      // We need a container which guarantees the pointer is stable under move
      // construction. `std::vector<>` does not provide that guarantee. Use
      // `std::unique_ptr<char[]>`. Do not use `std::make_unique<char[]> as that
      // will zero-initialize the array, and we are just going to overwrite the
      // bytes.
      auto buffer = std::unique_ptr<char[]>(new char[n]);
      std::copy_n(b.begin() + start, n, buffer.get());
      // Get `buffer.get()` before we std::move-from.
      auto contents = absl::string_view{buffer.get(), n};
      result.Append(absl::MakeCordFromExternal(
          contents, [b = std::move(buffer)]() mutable {}));
    }
    offset_ += n;
    start = 0;
    if (result.size() >= kMax) return result;
  }
  return result;
}

GOOGLE_CLOUD_CPP_INLINE_NAMESPACE_END
}  // namespace storage_internal
}  // namespace cloud
}  // namespace google
