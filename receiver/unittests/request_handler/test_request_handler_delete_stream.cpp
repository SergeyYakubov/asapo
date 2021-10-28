#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <asapo/database/db_error.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_delete_stream.h"
#include "../../../common/cpp/src/database/mongodb_client.h"

#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"
#include "asapo/common/networking.h"
#include "../receiver_mocking.h"

using namespace testing;
using namespace asapo;

namespace {

class DbMetaDeleteStreamTests : public Test {
  public:
    RequestHandlerDbDeleteStream handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string expected_stream = "stream";
    CustomRequestData expected_custom_data {0, 0, 0};

    void SetUp() override {
        GenericRequestHeader request_header;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void TearDown() override {
        handler.db_client__.release();
    }
    void ExpectDelete(uint64_t flag, const asapo::DBErrorTemplate* errorTemplate) {
        expected_custom_data[0] = flag;
        SetReceiverConfig(config, "none");
        EXPECT_CALL(*mock_request, GetCustomData_t()).WillOnce(Return(expected_custom_data));
        EXPECT_CALL(*mock_request, GetDataSource()).WillOnce(ReturnRef(expected_data_source));
        EXPECT_CALL(*mock_request, GetStream()).WillOnce(Return(expected_stream));

        asapo::DeleteStreamOptions opt;
        opt.Decode(flag);
        if (!opt.delete_meta) {
            EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("skipped deleting stream meta"),
                                                 HasSubstr(config.database_uri),
                                                 HasSubstr(expected_data_source),
                                                 HasSubstr(expected_stream),
                                                 HasSubstr(expected_beamtime_id)
                                                )
                                          )
                       );
            return;
        }

        EXPECT_CALL(mock_db, Connect_t(config.database_uri, expected_beamtime_id + "_" + expected_data_source)).
        WillOnce(testing::Return(nullptr));
        EXPECT_CALL(mock_db, DeleteStream_t(expected_stream)).
        WillOnce(testing::Return(errorTemplate == nullptr ? nullptr : errorTemplate->Generate().release()));
        if (errorTemplate == nullptr) {
            EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("deleted stream meta"),
                                                 HasSubstr(config.database_uri),
                                                 HasSubstr(expected_data_source),
                                                 HasSubstr(expected_stream),
                                                 HasSubstr(expected_beamtime_id)
                                                )
                                          )
                       );
        }

    }
};



TEST_F(DbMetaDeleteStreamTests, CallsDeleteOk) {

    ExpectDelete(3, nullptr);

    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));

}

TEST_F(DbMetaDeleteStreamTests, CallsDeleteErrorAlreadyExist) {

    ExpectDelete(3, &asapo::DBErrorTemplates::kNoRecord);
    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(asapo::ReceiverErrorTemplates::kBadRequest));
}

TEST_F(DbMetaDeleteStreamTests, CallsDeleteNoErrorAlreadyExist) {

    ExpectDelete(1, &asapo::DBErrorTemplates::kNoRecord);
    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbMetaDeleteStreamTests, CallsDeleteNoOp) {

    ExpectDelete(0, &asapo::DBErrorTemplates::kNoRecord);
    auto err = handler.ProcessRequest(mock_request.get());

    ASSERT_THAT(err, Eq(nullptr));
}

}
