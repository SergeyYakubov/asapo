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
#include "../../src/monitoring/receiver_monitoring_client_noop.h"

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

    class MonitoringClientTest : public Test {
    public:
        std::shared_ptr<NiceMock<asapo::MockDataCache>> mock_cache;
        NiceMock<asapo::MockReceiverMonitoringClientImpl_ToBeSendData>* mock_toBeSend;
        std::unique_ptr<asapo::ReceiverMonitoringClientImpl> monitoring;
        void SetUp() override {
            mock_cache.reset(new NiceMock<asapo::MockDataCache>);
            monitoring.reset(new asapo::ReceiverMonitoringClientImpl{mock_cache});
            mock_toBeSend = new NiceMock<asapo::MockReceiverMonitoringClientImpl_ToBeSendData>;
            monitoring->toBeSendData__.reset(mock_toBeSend);
        }
        void TearDown() override {
        }
    };

    TEST_F(MonitoringClientTest, DefaultGenerator) {
        std::shared_ptr<StrictMock<asapo::MockDataCache>> mock_cache;
        asapo::SharedReceiverMonitoringClient monitoring_l = asapo::GenerateDefaultReceiverMonitoringClient(mock_cache, false);
        asapo::ReceiverMonitoringClientImpl* monitoring_l_impl = dynamic_cast<asapo::ReceiverMonitoringClientImpl*>(monitoring_l.get());
        EXPECT_THAT(monitoring_l_impl, Ne(nullptr));
        EXPECT_THAT(monitoring_l_impl->log__, Ne(nullptr));
        EXPECT_THAT(monitoring_l_impl->io__, Ne(nullptr));
        EXPECT_THAT(monitoring_l_impl->toBeSendData__.get(), Ne(nullptr));
        EXPECT_THAT(monitoring_l_impl->sendingThreadRunning__, Eq(false));
    }

    TEST_F(MonitoringClientTest, DefaultGenerator_Noop) {
        std::shared_ptr<StrictMock<asapo::MockDataCache>> mock_cache;
        asapo::SharedReceiverMonitoringClient monitoring_l = asapo::GenerateDefaultReceiverMonitoringClient(mock_cache, true);

        asapo::ReceiverMonitoringClientNoop* monitoring_l_noop = dynamic_cast<asapo::ReceiverMonitoringClientNoop*>(monitoring_l.get());

        EXPECT_THAT(monitoring_l_noop, Ne(nullptr));
    }

    TEST_F(MonitoringClientTest, GetProducerToReceiverTransfer_Init) {
        asapo::ReceiverMonitoringClientImpl monitoring_l{nullptr};

        EXPECT_THAT(monitoring_l.log__, Ne(nullptr));
        EXPECT_THAT(monitoring_l.io__, Ne(nullptr));
        EXPECT_THAT(monitoring_l.toBeSendData__.get(), Ne(nullptr));
        EXPECT_THAT(monitoring_l.sendingThreadRunning__, Eq(false));
    }

    TEST_F(MonitoringClientTest, GetProducerToReceiverTransfer_SendProducerToReceiverTransferDataPoint) {
        auto x = new ProducerToReceiverTransferDataPoint;
        x->set_totalfilesize(100);
        x->set_totaltransferreceivetimeinmicroseconds(200);
        x->set_totalwriteiotimeinmicroseconds(300);
        x->set_totaldbtimeinmicroseconds(400);
        EXPECT_CALL(*mock_toBeSend, GetProducerToReceiverTransfer(
                "p1", "i1", "b1", "so1", "st1"
                )).WillOnce(Return(x));

        monitoring->sendingThreadRunning__ = true; // to trick client and initiata data transfer
        monitoring->SendProducerToReceiverTransferDataPoint("p1", "i1", "b1", "so1", "st1", 1, 2, 3, 4);

        EXPECT_THAT(x->totalfilesize(), Eq(101));
        EXPECT_THAT(x->totaltransferreceivetimeinmicroseconds(), Eq(202));
        EXPECT_THAT(x->totalwriteiotimeinmicroseconds(), Eq(303));
        EXPECT_THAT(x->totaldbtimeinmicroseconds(), Eq(404));
    }

    TEST_F(MonitoringClientTest, GetProducerToReceiverTransfer_SendRdsRequestWasMissDataPoint) {
        auto x = new RdsToConsumerDataPoint;
        x->set_totalfilesize(100);
        x->set_hits(200);
        x->set_misses(300);
        x->set_totaltransfersendtimeinmicroseconds(400);
        EXPECT_CALL(*mock_toBeSend, GetReceiverDataServerToConsumer(
                "p1", "i1", "b1", "so1", "st1"
                )).WillOnce(Return(x));

        monitoring->sendingThreadRunning__ = true; // to trick client and initiata data transfer
        monitoring->SendRdsRequestWasMissDataPoint("p1", "i1", "b1", "so1", "st1");

        EXPECT_THAT(x->totalfilesize(), Eq(100));
        EXPECT_THAT(x->hits(), Eq(200));
        EXPECT_THAT(x->misses(), Eq(301));
        EXPECT_THAT(x->totaltransfersendtimeinmicroseconds(), Eq(400));
    }

    TEST_F(MonitoringClientTest, GetProducerToReceiverTransfer_SendReceiverRequestDataPoint) {
        auto x = new RdsToConsumerDataPoint;
        x->set_totalfilesize(100);
        x->set_hits(200);
        x->set_misses(300);
        x->set_totaltransfersendtimeinmicroseconds(400);
        EXPECT_CALL(*mock_toBeSend, GetReceiverDataServerToConsumer(
                "p1", "i1", "b1", "so1", "st1"
                )).WillOnce(Return(x));

        monitoring->sendingThreadRunning__ = true; // to trick client and initiata data transfer
        monitoring->SendReceiverRequestDataPoint("p1", "i1", "b1", "so1", "st1", 2, 3);

        EXPECT_THAT(x->totalfilesize(), Eq(102));
        EXPECT_THAT(x->hits(), Eq(201));
        EXPECT_THAT(x->misses(), Eq(300));
        EXPECT_THAT(x->totaltransfersendtimeinmicroseconds(), Eq(403));
    }

    TEST_F(MonitoringClientTest, FillMemoryStats) {
        // using mock_toBeSend->container.mutable_groupedmemorystats()->Add();
        // and not "new" here, since the FillMemoryStats will access the container directly

        auto x1 = mock_toBeSend->container.mutable_groupedmemorystats()->Add();
        x1->set_totalbytes(100);
        x1->set_usedbytes(200);
        EXPECT_CALL(*mock_toBeSend, GetMemoryDataPoint(
                "b1", "so1", "st1"
                )).WillOnce(Return(x1));

        auto x2 = mock_toBeSend->container.mutable_groupedmemorystats()->Add();
        x2->set_totalbytes(300);
        x2->set_usedbytes(400);
        EXPECT_CALL(*mock_toBeSend, GetMemoryDataPoint(
                "b1", "so1", "st2"
                )).WillRepeatedly(Return(x2));

        auto x3 = mock_toBeSend->container.mutable_groupedmemorystats()->Add();
        x3->set_totalbytes(500);
        x3->set_usedbytes(600);
        EXPECT_CALL(*mock_toBeSend, GetMemoryDataPoint(
                "b3", "so3", "st3"
                )).WillOnce(Return(x3));

        std::vector<std::shared_ptr<const asapo::CacheMeta>> result;
        result.emplace_back(new asapo::CacheMeta{1, nullptr,5,1,"b1","so1","st1"});
        result.emplace_back(new asapo::CacheMeta{1, nullptr,6,1,"b1","so1","st2"});
        result.emplace_back(new asapo::CacheMeta{1, nullptr,7,1,"b3","so3","st3"});
        result.emplace_back(new asapo::CacheMeta{1, nullptr,8,1,"b1","so1","st2"});

        EXPECT_CALL(*mock_cache, AllMetaInfosAsVector()).WillRepeatedly(Return(result));
        EXPECT_CALL(*mock_cache, GetCacheSize()).WillRepeatedly(Return(123456));

        monitoring->FillMemoryStats();

        EXPECT_THAT(x1->totalbytes(), Eq(123456));
        EXPECT_THAT(x1->usedbytes(), Eq(205));
        EXPECT_THAT(x2->totalbytes(), Eq(123456));
        EXPECT_THAT(x2->usedbytes(), Eq(414));
        EXPECT_THAT(x3->totalbytes(), Eq(123456));
        EXPECT_THAT(x3->usedbytes(), Eq(607));
    }
}
