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
#include "../../src/request_handler/request_handler_db_get_meta.h"
#include "../../../common/cpp/src/database/mongodb_client.h"

#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"
#include "asapo/common/networking.h"
#include "../receiver_mocking.h"

using asapo::MockRequest;
using asapo::MessageMeta;
using ::testing::Test;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::SetArgPointee;
using ::testing::AllOf;
using ::testing::HasSubstr;


using ::asapo::Error;
using ::asapo::ErrorInterface;
using ::asapo::FileDescriptor;
using ::asapo::SocketDescriptor;
using ::asapo::MockIO;
using asapo::Request;
using asapo::RequestHandlerDbGetMeta;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;


namespace {

class DbMetaGetMetaTests : public Test {
  public:
    RequestHandlerDbGetMeta handler{asapo::kDBDataCollectionNamePrefix};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string expected_stream = "stream";
    std::string expected_meta = "meta";
    void SetUp() override {
        GenericRequestHeader request_header;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr, nullptr});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void TearDown() override {
        handler.db_client__.release();
    }
    void ExpectGet(bool stream, const asapo::DBErrorTemplate* errorTemplate) {
        SetReceiverConfig(config, "none");
        EXPECT_CALL(*mock_request, GetDataSource()).WillOnce(ReturnRef(expected_data_source));
        if (stream) {
            EXPECT_CALL(*mock_request, GetStream()).WillOnce(Return(expected_stream));
        }

        EXPECT_CALL(mock_db, Connect_t(config.database_uri, expected_beamtime_id + "_" + expected_data_source)).
        WillOnce(testing::Return(nullptr));
        EXPECT_CALL(mock_db, GetMetaFromDb_t("meta", stream ? "st_" + expected_stream : "bt", _)).
        WillOnce(DoAll(
                     SetArgPointee<2>(expected_meta),
                     testing::Return(errorTemplate == nullptr ? nullptr : errorTemplate->Generate().release())
                 ));
        if (errorTemplate == nullptr) {
            EXPECT_CALL(*mock_request, SetResponseMessage(expected_meta, asapo::ResponseMessageType::kInfo));
            EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("meta"),
                                                 HasSubstr(config.database_uri),
                                                 HasSubstr(expected_data_source),
                                                 HasSubstr(stream ? expected_stream : "beamtime"),
                                                 HasSubstr(expected_beamtime_id)
                                                )
                                          )
                       );
        }

    }
};



TEST_F(DbMetaGetMetaTests, GetBeamtimeMetaOk) {
    ExpectGet(false, nullptr);
    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}


TEST_F(DbMetaGetMetaTests, GetStreamMetaOk) {
    ExpectGet(true, nullptr);
    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

TEST_F(DbMetaGetMetaTests, GetStreamMetaError) {
    ExpectGet(true, &asapo::DBErrorTemplates::kDBError);
    auto err = handler.ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Ne(nullptr));
}

}
