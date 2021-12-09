#include "receiver_logger.h"

#include "request.h"

#include "request_handler/structs.h"

namespace asapo {

AbstractLogger* GetDefaultReceiverLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("receiver");
    return logger.get();
}

AbstractLogger* GetDefaultReceiverMonitoringLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("receiver_monitoring");
    return logger.get();
}


void AppendIdToLogMessageIfNeeded(const Request* request, LogMessageWithFields* msg) {
    switch (request->GetOpCode()) {
    case Opcode::kOpcodeTransferData:
        msg->Append("id", request->GetDataID());
        break;
    case Opcode::kOpcodeTransferDatasetData:
        msg->Append("id", request->GetDataID());
        msg->Append("substream", request->GetCustomData()[1]);
        break;
    default:
        break;
    }
}

LogMessageWithFields AuthorizationLog(std::string message, const Request* request, const AuthorizationData* data) {
    LogMessageWithFields msg{std::move(message)};
    msg.Append("beamtime", data->beamtime_id)
    .Append("dataSource", data->data_source)
    .Append("beamline", data->beamline)
    .Append("origin", request->GetOriginHost());

    if (request->GetOpCode() != kOpcodeAuthorize) {
        msg.Append("stream", request->GetStream());
        AppendIdToLogMessageIfNeeded(request, &msg);
    }

    return msg;
}


LogMessageWithFields RequestLog(std::string message, const Request* request) {
    LogMessageWithFields msg{std::move(message)};
    msg.Append("beamtime", request->GetBeamtimeId())
    .Append("beamline", request->GetBeamline())
    .Append("dataSource", request->GetDataSource())
    .Append("stream", request->GetStream())
    .Append("origin", request->GetOriginHost())
    .Append("operation", OpcodeToString(request->GetOpCode()));

    AppendIdToLogMessageIfNeeded(request, &msg);

    return msg;
}

}
