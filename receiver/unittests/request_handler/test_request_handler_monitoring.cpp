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
#include "../../src/request_handler/request_handler_monitoring.h"

#include "../mock_receiver_config.h"
#include "asapo/common/data_structs.h"
#include "asapo/common/networking.h"
#include "../receiver_mocking.h"
#include "../monitoring/receiver_monitoring_mocking.h"

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
using ::testing::StrictMock;
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

class RequestHandlerMonitoringTests : public Test {
  public:
    std::shared_ptr<StrictMock<asapo::MockReceiverMonitoringClient>> mock_monitoring;
    std::shared_ptr<StrictMock<asapo::MockInstancedStatistics>> mock_instanced_statistics;
    std::unique_ptr<asapo::RequestHandlerMonitoring> handler;
    std::unique_ptr<NiceMock<MockRequest>> mock_request;
    NiceMock<MockDatabase> mock_db;
    NiceMock<asapo::MockLogger> mock_logger;
    ReceiverConfig config;
    std::string expected_pipeline_step_id = "pipeline_step_id";
    std::string expected_producer_instance_id = "producer_instance_id";
    std::string expected_beamtime_id = "beamtime_id";
    std::string expected_data_source = "source";
    std::string expected_stream = "stream";
    std::string expected_file_name = "fname";

    void SetUp() override {
        asapo::SharedCache cache;
        mock_monitoring.reset(new StrictMock<asapo::MockReceiverMonitoringClient>{cache});
        handler.reset(new asapo::RequestHandlerMonitoring(mock_monitoring));
        mock_instanced_statistics.reset(new StrictMock<asapo::MockInstancedStatistics>);

        GenericRequestHeader request_header;
        mock_request.reset(new NiceMock<MockRequest> {request_header, 1, "", nullptr, mock_instanced_statistics});
        ON_CALL(*mock_request, GetPipelineStepId()).WillByDefault(ReturnRef(expected_pipeline_step_id));
        ON_CALL(*mock_request, GetProducerInstanceId()).WillByDefault(ReturnRef(expected_producer_instance_id));
        ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(ReturnRef(expected_beamtime_id));
        ON_CALL(*mock_request, GetDataSource()).WillByDefault(ReturnRef(expected_data_source));
        ON_CALL(*mock_request, GetStream()).WillByDefault(Return(expected_stream));
        ON_CALL(*mock_request, GetFileName()).WillByDefault(Return(expected_file_name));
        ON_CALL(*mock_request, GetInstancedStatistics()).WillByDefault(Return(mock_instanced_statistics));
    }
    void TearDown() override {
    }
};

TEST_F(RequestHandlerMonitoringTests, ExpectThatMonitoringFunctionIsCalled) {

    EXPECT_CALL(*mock_monitoring, SendProducerToReceiverTransferDataPoint(
            expected_pipeline_step_id, expected_producer_instance_id,
            expected_beamtime_id, expected_data_source, expected_stream,
            2, 3, 4, 5
            )).Times(1);

    EXPECT_CALL(*mock_instanced_statistics, GetIncomingBytes()).WillOnce(
            Return(2)
            );

    EXPECT_CALL(*mock_instanced_statistics, GetElapsedMicrosecondsCount(asapo::StatisticEntity::kNetworkIncoming)).WillOnce(
            Return(3)
            );
    EXPECT_CALL(*mock_instanced_statistics, GetElapsedMicrosecondsCount(asapo::StatisticEntity::kDisk)).WillOnce(
            Return(4)
            );
    EXPECT_CALL(*mock_instanced_statistics, GetElapsedMicrosecondsCount(asapo::StatisticEntity::kDatabase)).WillOnce(
            Return(5)
            );

    auto err = handler->ProcessRequest(mock_request.get());
    ASSERT_THAT(err, Eq(nullptr));
}

}
