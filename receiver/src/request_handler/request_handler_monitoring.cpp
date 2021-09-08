#include "request_handler_monitoring.h"
#include "../request.h"

#include <utility>

namespace asapo {

RequestHandlerMonitoring::RequestHandlerMonitoring(asapo::SharedReceiverMonitoringClient monitoring) : monitoring_{monitoring} {
}

StatisticEntity RequestHandlerMonitoring::GetStatisticEntity() const {
    return kMonitoring;
}

Error RequestHandlerMonitoring::ProcessRequest(Request* request) const {
    auto stats = request->GetInstancedStatistics();

    monitoring_->SendProducerToReceiverTransferDataPoint(
            request->GetPipelineStepId(),
            request->GetProducerInstanceId(),
            request->GetBeamtimeId(),
            request->GetDataSource(),
            request->GetStream(),
            stats->GetIncomingBytes(),
            stats->GetElapsedMicrosecondsCount(StatisticEntity::kNetworkIncoming),
            stats->GetElapsedMicrosecondsCount(StatisticEntity::kDisk),
            stats->GetElapsedMicrosecondsCount(StatisticEntity::kDatabase)
    );

    return nullptr;
}

}
