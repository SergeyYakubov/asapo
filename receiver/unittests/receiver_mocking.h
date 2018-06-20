#ifndef ASAPO_MOCK_STATISTICS_H
#define ASAPO_MOCK_STATISTICS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/statistics.h"
#include "../src/request.h"

namespace asapo {

class MockStatistics : public asapo::Statistics {
  public:
    void SendIfNeeded() noexcept override {
        SendIfNeeded_t();
    }

    void Send() noexcept override {
        Send_t();
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

    MOCK_METHOD0(SendIfNeeded_t, void());
    MOCK_METHOD0(Send_t, void());
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
        Request(request_header, socket_fd, std::move(origin_uri)) {};

    MOCK_CONST_METHOD0(GetFileName, std::string());
    MOCK_CONST_METHOD0(GetDataSize, uint64_t());
    MOCK_CONST_METHOD0(GetDataID, uint64_t());
    MOCK_CONST_METHOD0(GetData, const asapo::FileData & ());
    MOCK_CONST_METHOD0(GetBeamtimeId, const std::string & ());
    MOCK_CONST_METHOD0(GetOpCode, asapo::Opcode ());
    MOCK_CONST_METHOD0(GetMessage, const char* ());
    MOCK_METHOD1(SetBeamtimeId, void (std::string));
};


}

#endif //ASAPO_MOCK_STATISTICS_H
