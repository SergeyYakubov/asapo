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

    class MonitoringClient_ToBeSendDataTest : public Test {
        void SetUp() override {
        }
        void TearDown() override {
        }
    };
    TEST_F(MonitoringClient_ToBeSendDataTest, GetProducerToReceiverTransfer_InitCorrectly) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto p2r = data.GetProducerToReceiverTransfer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");

        ASSERT_THAT(p2r, Ne(nullptr));
        ASSERT_THAT(p2r->pipelinestepid(), Eq("pipeline1"));
        ASSERT_THAT(p2r->producerinstanceid(), Eq("instance1"));
        ASSERT_THAT(p2r->beamtime(), Eq("test-beamtime"));
        ASSERT_THAT(p2r->source(), Eq("test-source"));
        ASSERT_THAT(p2r->stream(), Eq("test-stream"));

        ASSERT_THAT(p2r->filecount(), Eq(0));
        ASSERT_THAT(p2r->totalfilesize(), Eq(0));
        ASSERT_THAT(p2r->totaltransferreceivetimeinmicroseconds(), Eq(0));
        ASSERT_THAT(p2r->totalwriteiotimeinmicroseconds(), Eq(0));
        ASSERT_THAT(p2r->totaldbtimeinmicroseconds(), Eq(0));
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetReceiverDataServerToConsumer_InitCorrectly) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto r2c = data.GetReceiverDataServerToConsumer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");

        ASSERT_THAT(r2c, Ne(nullptr));
        ASSERT_THAT(r2c->pipelinestepid(), Eq("pipeline1"));
        ASSERT_THAT(r2c->consumerinstanceid(), Eq("instance1"));
        ASSERT_THAT(r2c->beamtime(), Eq("test-beamtime"));
        ASSERT_THAT(r2c->source(), Eq("test-source"));
        ASSERT_THAT(r2c->stream(), Eq("test-stream"));

        ASSERT_THAT(r2c->totaltransfersendtimeinmicroseconds(), Eq(0));
        ASSERT_THAT(r2c->totalfilesize(), Eq(0));
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetMemoryDataPoint_InitCorrectly) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto m = data.GetMemoryDataPoint(
                "test-beamtime",
                "test-source",
                "test-stream");

        ASSERT_THAT(m, Ne(nullptr));
        ASSERT_THAT(m->beamtime(), Eq("test-beamtime"));
        ASSERT_THAT(m->source(), Eq("test-source"));
        ASSERT_THAT(m->stream(), Eq("test-stream"));

        ASSERT_THAT(m->totalbytes(), Eq(0));
        ASSERT_THAT(m->usedbytes(), Eq(0));
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetProducerToReceiverTransfer_PointsAreSame) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto p2r1 = data.GetProducerToReceiverTransfer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(p2r1, Ne(nullptr));
        p2r1->set_filecount(2);
        p2r1->set_totalfilesize(3);
        p2r1->set_totaltransferreceivetimeinmicroseconds(4);
        p2r1->set_totalwriteiotimeinmicroseconds(5);
        p2r1->set_totaldbtimeinmicroseconds(6);

        auto p2r2 = data.GetProducerToReceiverTransfer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(p2r2, Ne(nullptr));
        ASSERT_THAT(p2r2->filecount(), Eq(2));
        ASSERT_THAT(p2r2->totalfilesize(), Eq(3));
        ASSERT_THAT(p2r2->totaltransferreceivetimeinmicroseconds(), Eq(4));
        ASSERT_THAT(p2r2->totalwriteiotimeinmicroseconds(), Eq(5));
        ASSERT_THAT(p2r2->totaldbtimeinmicroseconds(), Eq(6));
        p2r2->set_filecount(10);
        p2r2->set_totalfilesize(11);
        p2r2->set_totaltransferreceivetimeinmicroseconds(12);
        p2r2->set_totalwriteiotimeinmicroseconds(13);
        p2r2->set_totaldbtimeinmicroseconds(14);

        auto p2r3 = data.GetProducerToReceiverTransfer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(p2r3, Ne(nullptr));
        ASSERT_THAT(p2r3->filecount(), Eq(10));
        ASSERT_THAT(p2r3->totalfilesize(), Eq(11));
        ASSERT_THAT(p2r3->totaltransferreceivetimeinmicroseconds(), Eq(12));
        ASSERT_THAT(p2r3->totalwriteiotimeinmicroseconds(), Eq(13));
        ASSERT_THAT(p2r3->totaldbtimeinmicroseconds(), Eq(14));
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetReceiverDataServerToConsumer_PointsAreSame) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto r2c1 = data.GetReceiverDataServerToConsumer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(r2c1, Ne(nullptr));
        r2c1->set_totaltransfersendtimeinmicroseconds(2);
        r2c1->set_totalfilesize(3);

        auto r2c2 = data.GetReceiverDataServerToConsumer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(r2c2, Ne(nullptr));
        ASSERT_THAT(r2c2->totaltransfersendtimeinmicroseconds(), Eq(2));
        ASSERT_THAT(r2c2->totalfilesize(), Eq(3));
        r2c2->set_totaltransfersendtimeinmicroseconds(5);
        r2c2->set_totalfilesize(6);

        auto r2c3 = data.GetReceiverDataServerToConsumer(
                "pipeline1",
                "instance1",
                "test-beamtime",
                "test-source",
                "test-stream");
        ASSERT_THAT(r2c3, Ne(nullptr));
        ASSERT_THAT(r2c3->totaltransfersendtimeinmicroseconds(), Eq(5));
        ASSERT_THAT(r2c3->totalfilesize(), Eq(6));
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetMemoryDataPoint_PointsAreSame) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        auto m1 = data.GetMemoryDataPoint("test-beamtime", "test-source", "test-stream");
        ASSERT_THAT(m1, Ne(nullptr));
        m1->set_usedbytes(10);
        m1->set_totalbytes(20);

        auto m2 = data.GetMemoryDataPoint("test-beamtime", "test-source", "test-stream");
        ASSERT_THAT(m2, Ne(nullptr));
        ASSERT_THAT(m2->usedbytes(), Eq(10));
        ASSERT_THAT(m2->totalbytes(), Eq(20));
        m1->set_usedbytes(30);
        m1->set_totalbytes(40);

        auto m3 = data.GetMemoryDataPoint("test-beamtime", "test-source", "test-stream");
        ASSERT_THAT(m3, Ne(nullptr));
        ASSERT_THAT(m3->usedbytes(), Eq(30));
        ASSERT_THAT(m3->totalbytes(), Eq(40));
    }


    TEST_F(MonitoringClient_ToBeSendDataTest, GetProducerToReceiverTransfer_Grouping) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        {
            auto p2r1 = data.GetProducerToReceiverTransfer("p1","i1","b1","so1","st1");
            ASSERT_THAT(p2r1->filecount(), Eq(0));
            p2r1->set_filecount(1);

            auto p2r2 = data.GetProducerToReceiverTransfer("p2","i1","b1","so1","st1");
            ASSERT_THAT(p2r2->filecount(), Eq(0));
            p2r2->set_filecount(2);

            auto p2r3 = data.GetProducerToReceiverTransfer("p1","i2","b1","so1","st1");
            ASSERT_THAT(p2r3->filecount(), Eq(0));
            p2r3->set_filecount(3);

            auto p2r4 = data.GetProducerToReceiverTransfer("p1","i1","b2","so1","st1");
            ASSERT_THAT(p2r4->filecount(), Eq(0));
            p2r4->set_filecount(4);

            auto p2r5 = data.GetProducerToReceiverTransfer("p1","i1","b1","so2","st1");
            ASSERT_THAT(p2r5->filecount(), Eq(0));
            p2r5->set_filecount(5);

            auto p2r6 = data.GetProducerToReceiverTransfer("p1","i1","b1","so1","st2");
            ASSERT_THAT(p2r6->filecount(), Eq(0));
            p2r6->set_filecount(6);
        }
        {

            auto p2r1 = data.GetProducerToReceiverTransfer("p1","i1","b1","so1","st1");
            ASSERT_THAT(p2r1->filecount(), Eq(1));

            auto p2r2 = data.GetProducerToReceiverTransfer("p2","i1","b1","so1","st1");
            ASSERT_THAT(p2r2->filecount(), Eq(2));

            auto p2r3 = data.GetProducerToReceiverTransfer("p1","i2","b1","so1","st1");
            ASSERT_THAT(p2r3->filecount(), Eq(3));

            auto p2r4 = data.GetProducerToReceiverTransfer("p1","i1","b2","so1","st1");
            ASSERT_THAT(p2r4->filecount(), Eq(4));

            auto p2r5 = data.GetProducerToReceiverTransfer("p1","i1","b1","so2","st1");
            ASSERT_THAT(p2r5->filecount(), Eq(5));

            auto p2r6 = data.GetProducerToReceiverTransfer("p1","i1","b1","so1","st2");
            ASSERT_THAT(p2r6->filecount(), Eq(6));
        }
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetReceiverDataServerToConsumer_Grouping) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        {
            auto r2c1 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so1","st1");
            ASSERT_THAT(r2c1->hits(), Eq(0));
            r2c1->set_hits(1);

            auto r2c2 = data.GetReceiverDataServerToConsumer("p2","i1","b1","so1","st1");
            ASSERT_THAT(r2c2->hits(), Eq(0));
            r2c2->set_hits(2);

            auto r2c3 = data.GetReceiverDataServerToConsumer("p1","i2","b1","so1","st1");
            ASSERT_THAT(r2c3->hits(), Eq(0));
            r2c3->set_hits(3);

            auto r2c4 = data.GetReceiverDataServerToConsumer("p1","i1","b2","so1","st1");
            ASSERT_THAT(r2c4->hits(), Eq(0));
            r2c4->set_hits(4);

            auto r2c5 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so2","st1");
            ASSERT_THAT(r2c5->hits(), Eq(0));
            r2c5->set_hits(5);

            auto r2c6 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so1","st2");
            ASSERT_THAT(r2c6->hits(), Eq(0));
            r2c6->set_hits(6);
        }
        {

            auto r2c1 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so1","st1");
            ASSERT_THAT(r2c1->hits(), Eq(1));

            auto r2c2 = data.GetReceiverDataServerToConsumer("p2","i1","b1","so1","st1");
            ASSERT_THAT(r2c2->hits(), Eq(2));

            auto r2c3 = data.GetReceiverDataServerToConsumer("p1","i2","b1","so1","st1");
            ASSERT_THAT(r2c3->hits(), Eq(3));

            auto r2c4 = data.GetReceiverDataServerToConsumer("p1","i1","b2","so1","st1");
            ASSERT_THAT(r2c4->hits(), Eq(4));

            auto r2c5 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so2","st1");
            ASSERT_THAT(r2c5->hits(), Eq(5));

            auto r2c6 = data.GetReceiverDataServerToConsumer("p1","i1","b1","so1","st2");
            ASSERT_THAT(r2c6->hits(), Eq(6));
        }
    }

    TEST_F(MonitoringClient_ToBeSendDataTest, GetMemoryDataPoint_Grouping) {
        asapo::ReceiverMonitoringClient::ToBeSendData data;

        {
            auto m1 = data.GetMemoryDataPoint("b1", "so1", "st1");
            ASSERT_THAT(m1->totalbytes(), Eq(0));
            m1->set_totalbytes(1);

            auto m2 = data.GetMemoryDataPoint("b2", "so1", "st1");
            ASSERT_THAT(m2->totalbytes(), Eq(0));
            m2->set_totalbytes(2);

            auto m3 = data.GetMemoryDataPoint("b1", "so2", "st1");
            ASSERT_THAT(m3->totalbytes(), Eq(0));
            m3->set_totalbytes(3);

            auto m4 = data.GetMemoryDataPoint("b1", "so2", "st2");
            ASSERT_THAT(m4->totalbytes(), Eq(0));
            m4->set_totalbytes(4);
        }
        {
            auto m1 = data.GetMemoryDataPoint("b1", "so1", "st1");
            ASSERT_THAT(m1->totalbytes(), Eq(1));

            auto m2 = data.GetMemoryDataPoint("b2", "so1", "st1");
            ASSERT_THAT(m2->totalbytes(), Eq(2));

            auto m3 = data.GetMemoryDataPoint("b1", "so2", "st1");
            ASSERT_THAT(m3->totalbytes(), Eq(3));

            auto m4 = data.GetMemoryDataPoint("b1", "so2", "st2");
            ASSERT_THAT(m4->totalbytes(), Eq(4));
        }
    }

}
