// Copyright 2021 Google LLC
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
// source: generator/integration_tests/test.proto
#ifndef GOOGLE_CLOUD_CPP_GENERATOR_INTEGRATION_TESTS_GOLDEN_MOCKS_MOCK_IAM_CREDENTIALS_CONNECTION_GCPCXX_PB_H
#define GOOGLE_CLOUD_CPP_GENERATOR_INTEGRATION_TESTS_GOLDEN_MOCKS_MOCK_IAM_CREDENTIALS_CONNECTION_GCPCXX_PB_H

#include "generator/integration_tests/golden/iam_credentials_connection.gcpcxx.pb.h"
#include <gmock/gmock.h>

namespace google {
namespace cloud {
inline namespace GOOGLE_CLOUD_CPP_NS {
namespace golden_mocks {

class MockIAMCredentialsConnection : public golden::IAMCredentialsConnection {
 public:
  MOCK_METHOD(StatusOr<::google::test::admin::database::v1::GenerateAccessTokenResponse>,
  GenerateAccessToken,
  (::google::test::admin::database::v1::GenerateAccessTokenRequest const& request), (override));

  MOCK_METHOD(StatusOr<::google::test::admin::database::v1::GenerateIdTokenResponse>,
  GenerateIdToken,
  (::google::test::admin::database::v1::GenerateIdTokenRequest const& request), (override));

  MOCK_METHOD(StatusOr<::google::test::admin::database::v1::WriteLogEntriesResponse>,
  WriteLogEntries,
  (::google::test::admin::database::v1::WriteLogEntriesRequest const& request), (override));

  MOCK_METHOD(golden::ListLogsRange,
  ListLogs,
  (::google::test::admin::database::v1::ListLogsRequest request), (override));

  MOCK_METHOD(golden::TailLogEntriesStream,
  TailLogEntries,
  (::google::test::admin::database::v1::TailLogEntriesRequest request), (override));

};

}  // namespace golden_mocks
}  // namespace GOOGLE_CLOUD_CPP_NS
}  // namespace cloud
}  // namespace google

#endif  // GOOGLE_CLOUD_CPP_GENERATOR_INTEGRATION_TESTS_GOLDEN_MOCKS_MOCK_IAM_CREDENTIALS_CONNECTION_GCPCXX_PB_H
