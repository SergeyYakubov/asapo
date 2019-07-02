#ifndef ASAPO_RECEIVER_MOCKING_H
#define ASAPO_RECEIVER_MOCKING_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/receiver_statistics.h"
#include "../src/request.h"
#include "../src/data_cache.h"

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

class MockRequest: public Request {
  public:
    MockRequest(const GenericRequestHeader& request_header, SocketDescriptor socket_fd, std::string origin_uri):
        Request(request_header, socket_fd, std::move(origin_uri), nullptr) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetDataID, uint64_t());
    MOCK_CONST_METHOD0(GetSlotId, uint64_t());
    MOCK_CONST_METHOD0(GetData, void* ());
    MOCK_CONST_METHOD0(GetBeamtimeId, const std::string & ());
    MOCK_CONST_METHOD0(GetMetaData, const std::string & ());
    MOCK_CONST_METHOD0(GetBeamline, const std::string & ());
    MOCK_CONST_METHOD0(GetOpCode,
                       asapo::Opcode ());
    const asapo::CustomRequestData& GetCustomData() const override {
        return (asapo::CustomRequestData&) * GetCustomData_t();
    };

    MOCK_CONST_METHOD0(GetCustomData_t, const uint64_t* ());
    MOCK_CONST_METHOD0(GetMessage, const char* ());
    MOCK_METHOD1(SetBeamtimeId, void (std::string));
    MOCK_METHOD1(SetBeamline, void (std::string));
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


}

#endif //ASAPO_RECEIVER_MOCKING_H
