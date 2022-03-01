#ifndef ASAPO_RECEIVER_MOCKING_H
#define ASAPO_RECEIVER_MOCKING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/statistics/receiver_statistics.h"
#include "../src/request.h"
#include "../src/data_cache.h"
#include "../src/request_handler/file_processors/file_processor.h"
#include "../src/request_handler/request_handler_db_check_request.h"
#include "../src/request_handler/authorization_client.h"

namespace asapo {

class MockStatistics : public asapo::ReceiverStatistics {
  public:
    void SendIfNeeded(bool send_always) noexcept override {
        SendIfNeeded_t(send_always);
    }

    void IncreaseRequestCounter() noexcept override {
        IncreaseRequestCounter_t();
    }
    void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept override {
        IncreaseRequestDataVolume_t(transferred_data_volume);
    }

    MOCK_METHOD1(SendIfNeeded_t, void(bool send_always));
    MOCK_METHOD0(IncreaseRequestCounter_t, void());
    MOCK_METHOD1(IncreaseRequestDataVolume_t, void (uint64_t transferred_data_volume));

};

class MockInstancedStatistics : public asapo::RequestStatistics {
public:
    MOCK_METHOD1(StartTimer, void(StatisticEntity entity));
    MOCK_METHOD0(StopTimer, void());

    MOCK_METHOD1(AddIncomingBytes, void(uint64_t incomingByteCount));
    MOCK_METHOD1(AddOutgoingBytes, void(uint64_t outgoingByteCount));

    MOCK_CONST_METHOD0(GetIncomingBytes, uint64_t());
    MOCK_CONST_METHOD0(GetOutgoingBytes, uint64_t());

    MOCK_CONST_METHOD1(GetOutgoingBytes, std::chrono::nanoseconds(StatisticEntity entity));
    MOCK_CONST_METHOD1(GetElapsedMicrosecondsCount, uint64_t(StatisticEntity entity));

    MOCK_METHOD1(SendIfNeeded, void(bool send_always));
    MOCK_METHOD0(IncreaseRequestCounter_t, void());
    MOCK_METHOD1(IncreaseRequestDataVolume_t, void (uint64_t transferred_data_volume));
};

class MockHandlerDbCheckRequest : public asapo::RequestHandlerDbCheckRequest {
  public:
    MockHandlerDbCheckRequest(std::string collection_name_prefix): RequestHandlerDbCheckRequest(collection_name_prefix) {};

    Error ProcessRequest(Request* request) const override {
        return Error{ProcessRequest_t(*request)};
    }

    StatisticEntity GetStatisticEntity() const override {
        return StatisticEntity::kDatabase;
    }

    MOCK_METHOD(ErrorInterface *, ProcessRequest_t, (const Request& request), (const));

};


class MockRequest: public Request {
  public:
    MockRequest(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
                const RequestHandlerDbCheckRequest* db_check_handler, RequestStatisticsPtr statistics):
        Request(request_header, socket_fd, std::move(origin_uri), nullptr, db_check_handler, std::move(statistics)) {};

//    MOCK_METHOD(, ), (const,override), (override));
    MOCK_METHOD(std::string, GetFileName, (), (const, override));
    MOCK_METHOD(std::string, GetStream, (), (const, override));
    MOCK_METHOD(std::string, GetApiVersion, (), (const, override));
    MOCK_METHOD(const std::string &, GetOriginUri, (), (const, override));
    MOCK_METHOD(const std::string &, GetOriginHost, (), (const, override));
    MOCK_METHOD(uint64_t, GetDataSize, (), (const, override));
    MOCK_METHOD(uint64_t, GetDataID, (), (const, override));
    MOCK_METHOD(uint64_t, GetSlotId, (), (const, override));
    MOCK_METHOD(void*, GetData, (), (const, override));
    MOCK_CONST_METHOD0(GetPipelineStepId, const std::string & ());
    MOCK_CONST_METHOD0(GetProducerInstanceId, const std::string & ());
    MOCK_METHOD(const std::string &, GetBeamtimeId, (), (const, override));
    MOCK_METHOD(const std::string &, GetDataSource, (), (const, override));
    MOCK_METHOD(const std::string &, GetMetaData, (), (const, override));
    MOCK_METHOD(const std::string &, GetBeamline, (), (const, override));
    MOCK_METHOD(asapo::Opcode, GetOpCode, (), (const, override));
    MOCK_METHOD(asapo::SocketDescriptor, GetSocket, (), (const, override));
    MOCK_METHOD(uint64_t, GetIngestMode, (), (const, override));
    MOCK_METHOD(const std::string &, GetOnlinePath, (), (const, override));
    MOCK_METHOD(const std::string &, GetOfflinePath, (), (const, override));

    MOCK_METHOD0(GetStatistics, RequestStatistics*());

    // not nice casting, but mocking GetCustomData directly does not compile on Windows.
    const CustomRequestData& GetCustomData() const override {
        return (CustomRequestData&) * GetCustomData_t();
    };

    MOCK_METHOD(const uint64_t*, GetCustomData_t, (), (const));
    MOCK_METHOD(const char*, GetMessage, (), (const)); //override does not compile on windows, not clear why ()
    MOCK_METHOD1(SetProducerInstanceId, void (std::string));
    MOCK_METHOD1(SetPipelineStepId, void (std::string));
    MOCK_METHOD(void, SetBeamtimeId, (std::string), (override));
    MOCK_METHOD(void, SetDataSource, (std::string), (override));
    MOCK_METHOD(void, SetBeamline, (std::string), (override));
    MOCK_METHOD(void, SetOnlinePath, (std::string), (override));
    MOCK_METHOD(void, SetOfflinePath, (std::string), (override));

    MOCK_METHOD(void, SetSourceType, (SourceType), (override));
    MOCK_METHOD(SourceType, GetSourceType, (), (const, override));

    MOCK_METHOD(bool, WasAlreadyProcessed, (), (const, override));
    MOCK_METHOD(void, SetAlreadyProcessedFlag, (), (override));
    MOCK_METHOD(void, SetResponseMessage, (std::string, ResponseMessageType), (override));
    MOCK_METHOD(const std::string &, GetResponseMessage, (), (const, override));
    MOCK_METHOD(ResponseMessageType, GetResponseMessageType_t, (), (const));

    ResponseMessageType GetResponseMessageType() const override {
        return GetResponseMessageType_t();
    };

    Error CheckForDuplicates()  override {
        return Error{CheckForDuplicates_t()};
    }

    MOCK_METHOD(ErrorInterface *, CheckForDuplicates_t, (), ());
};


class MockDataCache: public DataCache {
  public:
    MockDataCache(): DataCache(0, 0) {};
    void* GetFreeSlotAndLock(uint64_t size, CacheMeta** meta,std::string beamtime, std::string source, std::string stream, Error* err) override{
      ErrorInterface* error = nullptr;
      auto data = GetFreeSlotAndLock_t(size, meta,beamtime,source,stream, &error);
      err->reset(error);
      return data;
    }

    MOCK_METHOD(void*, GetFreeSlotAndLock_t, (uint64_t size, CacheMeta** meta,std::string beamtime, std::string source, std::string stream,ErrorInterface** err));
    MOCK_METHOD(bool, UnlockSlot, (CacheMeta* meta), (override));
    MOCK_CONST_METHOD0(AllMetaInfosAsVector, std::vector<std::shared_ptr<const CacheMeta>>());
    MOCK_CONST_METHOD0(GetCacheSize, uint64_t());
    MOCK_METHOD(void*, GetSlotToReadAndLock, (uint64_t id, uint64_t data_size, CacheMeta** meta), (override));
};

class MockStatisticsSender: public StatisticsSender {
  public:
    void SendStatistics(const StatisticsToSend& statistics) const noexcept override {
        SendStatistics_t(statistics);
    }
    MOCK_METHOD(void, SendStatistics_t, (const StatisticsToSend&), (const));
};


class MockFileProcessor: public FileProcessor {
  public:
    Error ProcessFile(const Request* request, bool overwrite) const override {
        return Error{ProcessFile_t(request, overwrite)};

    }
    MOCK_METHOD(ErrorInterface *, ProcessFile_t, (const Request*, bool), (const));
};


class MockAuthorizationClient: public AuthorizationClient  {
  public:
    Error Authorize(const Request* request, AuthorizationData* data) const override {
        return Error{Authorize_t(request, data)};
    }
    MOCK_METHOD(ErrorInterface *, Authorize_t, (const Request*, AuthorizationData*), (const));
};

inline void SetDefaultRequestCalls(MockRequest* mock_request,const std::string& bt) {
    ON_CALL(*mock_request, GetBeamtimeId()).WillByDefault(::testing::ReturnRefOfCopy(bt));
    ON_CALL(*mock_request, GetBeamline()).WillByDefault(::testing::ReturnRefOfCopy(std::string("")));
    ON_CALL(*mock_request, GetDataSource()).WillByDefault(::testing::ReturnRefOfCopy(std::string("")));
    ON_CALL(*mock_request, GetStream()).WillByDefault(::testing::Return(std::string("")));
    ON_CALL(*mock_request, GetOriginHost()).WillByDefault(::testing::ReturnRefOfCopy(std::string("")));
    ON_CALL(*mock_request, GetOpCode()).WillByDefault(::testing::Return(Opcode::kOpcodeTransferData));
}




}

#endif //ASAPO_RECEIVER_MOCKING_H
