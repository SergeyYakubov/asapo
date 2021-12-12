#include <functional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "asapo/database/db_error.h"

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_check_request.h"
#include "asapo/common/networking.h"
#include "../../../common/cpp/src/database/mongodb_client.h"
#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"

#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

namespace {

class ReceiverTests : public Test {
 public:
  std::string expected_collection_name = "test";
  RequestHandlerDb handler{expected_collection_name};
  std::unique_ptr<NiceMock<MockRequest>> mock_request;
  NiceMock<MockDatabase> mock_db;
  NiceMock<asapo::MockLogger> mock_logger;
  ReceiverConfig config;
  std::string expected_beamtime_id = "beamtime_id";
  std::string expected_stream = "source";
  std::string expected_default_source = "detector";
  std::string expected_discovery_server = "discovery";
  std::string expected_database_server = "127.0.0.1:27017";
  GenericRequestHeader request_header;
  virtual void SetUp() override {
      handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
      handler.log__ = &mock_logger;
      mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
      SetDefaultRequestCalls(mock_request.get(),expected_beamtime_id);
  }
  virtual void TearDown() override {
      handler.db_client__.release();
  }

};

