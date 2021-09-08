#ifndef ASAPO_REQUEST_HANDLER_SEND_MONITORING_H
#define ASAPO_REQUEST_HANDLER_SEND_MONITORING_H

#include "request_handler.h"
#include "asapo/logger/logger.h"

#include "asapo/io/io.h"
#include "../monitoring/receiver_monitoring_client.h"

namespace asapo {

    class RequestHandlerMonitoring final: public ReceiverRequestHandler {
    private:
        SharedReceiverMonitoringClient monitoring_;
    public:
        RequestHandlerMonitoring(SharedReceiverMonitoringClient monitoring);
        RequestHandlerMonitoring(const RequestHandlerMonitoring&) = delete;
        RequestHandlerMonitoring& operator=(const RequestHandlerMonitoring&) = delete;

        StatisticEntity GetStatisticEntity() const override;
        Error ProcessRequest(Request* request) const override;
        std::unique_ptr<IO> io__;
        const AbstractLogger* log__{};
    };

}

#endif //ASAPO_REQUEST_HANDLER_SEND_MONITORING_H
