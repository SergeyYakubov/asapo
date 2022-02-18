#ifndef ASAPO_RECEIVER_DATA_SERVER_REQUEST_H
#define ASAPO_RECEIVER_DATA_SERVER_REQUEST_H

#include "asapo/common/networking.h"

#include "asapo/request/request.h"
#include "../statistics/instanced_statistics_provider.h"

namespace asapo {

class RdsNetServer;

class ReceiverDataServerRequest : public GenericRequest {
  private:
    RequestStatisticsPtr statistics_;
  public:
    explicit ReceiverDataServerRequest(const GenericRequestHeader& header, uint64_t source_id, RequestStatisticsPtr statistics);
    ~ReceiverDataServerRequest() override = default;

    const uint64_t source_id;

    RequestStatistics* GetStatistics();
};

using ReceiverDataServerRequestPtr = std::unique_ptr<ReceiverDataServerRequest>;

}

#endif //ASAPO_RECEIVER_DATA_SERVER_REQUEST_H
