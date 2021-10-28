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
    void StartTimer(const asapo::StatisticEntity& entity) noexcept override {
        StartTimer_t(entity);
    }
    void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept override {
        IncreaseRequestDataVolume_t(transferred_data_volume);
    }
    void StopTimer() noexcept override {
        StopTimer_t();
    }

    MOCK_METHOD1(SendIfNeeded_t, void(bool send_always));
    MOCK_METHOD0(IncreaseRequestCounter_t, void());
    MOCK_METHOD0(StopTimer_t, void());
    MOCK_METHOD1(IncreaseRequestDataVolume_t, void (uint64_t
                                                    transferred_data_volume));
    MOCK_METHOD1(StartTimer_t, void(
                     const asapo::StatisticEntity& entity));

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

    MOCK_CONST_METHOD1(ProcessRequest_t, ErrorInterface * (const Request& request));

};


class MockRequest: public Request {
  public:
    MockRequest(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri,
                const RequestHandlerDbCheckRequest* db_check_handler ):
        Request(request_header, socket_fd, std::move(origin_uri), nullptr, db_check_handler) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetStream, std::string());
    MOCK_CONST_METHOD0(GetApiVersion, std::string());
    MOCK_CONST_METHOD0(GetOriginUri, const std::string & ());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetDataID, uint64_t());
    MOCK_CONST_METHOD0(GetSlotId, uint64_t());
    MOCK_CONST_METHOD0(GetData, void* ());
    MOCK_CONST_METHOD0(GetBeamtimeId, const std::string & ());
    MOCK_CONST_METHOD0(GetDataSource, const std::string & ());
    MOCK_CONST_METHOD0(GetMetaData, const std::string & ());
    MOCK_CONST_METHOD0(GetBeamline, const std::string & ());
    MOCK_CONST_METHOD0(GetOpCode, asapo::Opcode ());
    MOCK_CONST_METHOD0(GetSocket, asapo::SocketDescriptor ());

    MOCK_CONST_METHOD0(GetOnlinePath, const std::string & ());
    MOCK_CONST_METHOD0(GetOfflinePath, const std::string & ());

    // not nice casting, but mocking GetCustomData directly does not compile on Windows.
    const CustomRequestData& GetCustomData() const override {
        return (CustomRequestData&) * GetCustomData_t();
    };

    MOCK_CONST_METHOD0(GetCustomData_t, const uint64_t* ());
    MOCK_CONST_METHOD0(GetMessage, const char* ());
    MOCK_METHOD1(SetBeamtimeId, void (std::string));
    MOCK_METHOD1(SetDataSource, void (std::string));
    MOCK_METHOD1(SetBeamline, void (std::string));
    MOCK_METHOD1(SetOnlinePath, void (std::string));
    MOCK_METHOD1(SetOfflinePath, void (std::string));

    MOCK_METHOD1(SetSourceType, void (SourceType));
    MOCK_CONST_METHOD0(GetSourceType, SourceType ());

    MOCK_CONST_METHOD0(WasAlreadyProcessed, bool());
    MOCK_METHOD0(SetAlreadyProcessedFlag, void());
    MOCK_METHOD2(SetResponseMessage, void(std::string, ResponseMessageType));
    MOCK_CONST_METHOD0(GetResponseMessage, const std::string & ());
    MOCK_CONST_METHOD0(GetResponseMessageType_t, ResponseMessageType ());

    ResponseMessageType GetResponseMessageType() const override {
        return GetResponseMessageType_t();
    };

    Error CheckForDuplicates()  override {
        return Error{CheckForDuplicates_t()};
    }

    MOCK_METHOD0(CheckForDuplicates_t, ErrorInterface * ());
};


class MockDataCache: public DataCache {
  public:
    MockDataCache(): DataCache(0, 0) {};
    MOCK_METHOD2(GetFreeSlotAndLock, void* (uint64_t
                                            size, CacheMeta** meta));
    MOCK_METHOD1(UnlockSlot, bool(CacheMeta* meta));
    MOCK_METHOD3(GetSlotToReadAndLock, void* (uint64_t
                                              id, uint64_t data_size, CacheMeta** meta));

};


class MockStatisticsSender: public StatisticsSender {
  public:
    void SendStatistics(const StatisticsToSend& statistics) const noexcept override {
        SendStatistics_t(statistics);
    }
    MOCK_CONST_METHOD1(SendStatistics_t, void (const StatisticsToSend&));
};


class MockFileProcessor: public FileProcessor {
  public:
    Error ProcessFile(const Request* request, bool overwrite) const override {
        return Error{ProcessFile_t(request, overwrite)};

    }
    MOCK_CONST_METHOD2(ProcessFile_t, ErrorInterface * (const Request*, bool));
};


class MockAuthorizationClient: public AuthorizationClient  {
  public:
    Error Authorize(const Request* request, AuthorizationData* data) const override {
        return Error{Authorize_t(request, data)};
    }
    MOCK_CONST_METHOD2(Authorize_t, ErrorInterface * (const Request*, AuthorizationData* ));
};


}

#endif //ASAPO_RECEIVER_MOCKING_H
