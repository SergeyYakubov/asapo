#ifndef HIDRA2_MOCK_STATISTICS_H
#define HIDRA2_MOCK_STATISTICS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/statistics.h"

namespace hidra2 {

class MockStatistics : public hidra2::Statistics {
 public:
  void SendIfNeeded() noexcept override {
      SendIfNeeded_t();
  }
  void IncreaseRequestCounter() noexcept override {
      IncreaseRequestCounter_t();
  }
  void StartTimer(const hidra2::StatisticEntity &entity) noexcept override {
      StartTimer_t(entity);
  }
  void IncreaseRequestDataVolume(uint64_t transferred_data_volume) noexcept override {
      IncreaseRequestDataVolume_t(transferred_data_volume);
  }
  void StopTimer() noexcept override {
      StopTimer_t();
  }

  MOCK_METHOD0(SendIfNeeded_t, void());
  MOCK_METHOD0(IncreaseRequestCounter_t, void());
  MOCK_METHOD0(StopTimer_t, void());
  MOCK_METHOD1(IncreaseRequestDataVolume_t, void (uint64_t
      transferred_data_volume));
  MOCK_METHOD1(StartTimer_t, void(
      const hidra2::StatisticEntity &entity));

};

}

#endif //HIDRA2_MOCK_STATISTICS_H
