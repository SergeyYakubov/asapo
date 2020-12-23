#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "asapo/unittests/MockIO.h"
#include "asapo/unittests/MockDatabase.h"
#include "asapo/unittests/MockLogger.h"

#include "../../src/receiver_error.h"
#include "../../src/request.h"
#include "../../src/request_handler/request_factory.h"
#include "../../src/request_handler/request_handler.h"
#include "../../src/request_handler/request_handler_db_meta_write.h"
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
using asapo::RequestHandlerDbMetaWrite;
using ::asapo::GenericRequestHeader;

using asapo::MockDatabase;
using asapo::RequestFactory;
using asapo::SetReceiverConfig;
using asapo::ReceiverConfig;


namespace {

class DbMetaWriterHandlerTests : public Test {
  public:
    std::string expected_collection_name = asapo::kDBMetaCollectionName;
    RequestHandlerDbMetaWrite handler{expected_collection_name};
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string meta_str =
        R"("info":"stat","int_data":0,"float_data":0.1,"bool":false)";
    const uint8_t* expected_meta = reinterpret_cast<const uint8_t*>(meta_str.c_str());
    uint64_t expected_meta_size = meta_str.size();
    uint64_t expected_meta_id = 0;
    void SetUp() override {
        GenericRequestHeader request_header;
        request_header.data_id = 0;
        handler.db_client__ = std::unique_ptr<asapo::Database> {&mock_db};
        handler.log__ = &mock_logger;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr});
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
    }
    void TearDown() override {
        handler.db_client__.release();
    }


};


TEST_F(DbMetaWriterHandlerTests, CallsUpdate) {
    SetReceiverConfig(config, "none");

    EXPECT_CALL(*mock_request, GetBeamtimeId())
    .WillOnce(ReturnRef(expected_beamtime_id))
    ;

    EXPECT_CALL(*mock_request, GetDataSource())
    .WillOnce(ReturnRef(expected_data_source))
    ;


    EXPECT_CALL(mock_db, Connect_t(config.database_uri, expected_beamtime_id + "_" + expected_data_source)).
    WillOnce(testing::Return(nullptr));


    EXPECT_CALL(*mock_request, GetDataSize())
    .WillOnce(Return(expected_meta_size))
    ;

    EXPECT_CALL(*mock_request, GetData())
    .WillOnce(Return((void*)expected_meta))
    ;

    EXPECT_CALL(mock_db, Upsert_t(expected_collection_name, expected_meta_id, expected_meta, expected_meta_size)).
    WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mock_logger, Debug(AllOf(HasSubstr("insert beamtime meta"),
                                         HasSubstr(config.database_uri),
                                         HasSubstr(expected_beamtime_id),
                                         HasSubstr(expected_collection_name)
                                        )
                                  )
               );

    handler.ProcessRequest(mock_request.get());
}

}
