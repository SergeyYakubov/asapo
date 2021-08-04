#ifndef ASAPO_RECEIVER_STATISTICS_ENTRY_TYPE_H
#define ASAPO_RECEIVER_STATISTICS_ENTRY_TYPE_H

namespace asapo {

static const size_t kNStatisticEntities = 5;
enum StatisticEntity : int {
    kDatabase = 0,
    kDisk,
    kNetworkIncoming,
    kNetworkOutgoing,
    kMonitoring,
};

}

#endif //ASAPO_RECEIVER_STATISTICS_ENTRY_TYPE_H
