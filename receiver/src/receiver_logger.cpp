#include "receiver_logger.h"

#include "request.h"

namespace asapo {

AbstractLogger* GetDefaultReceiverLogger() {
    static Logger logger = asapo::CreateDefaultLoggerBin("receiver");
    return logger.get();
}

LogMessageWithFields RequestLog(std::string message, const Request* request) {
    LogMessageWithFields msg{std::move(message)};
    msg.Append("beamtime", request->GetBeamtimeId())
    .Append("dataSource", request->GetDataSource())
    .Append("stream", request->GetStream())
    .Append("origin", request->GetOriginHost())
    .Append("operation", OpcodeToString(request->GetOpCode()));

    switch (request->GetOpCode()) {
    case Opcode::kOpcodeTransferData:
        msg.Append("id", request->GetDataID());
        break;
    case Opcode::kOpcodeTransferDatasetData:
        msg.Append("id", request->GetDataID());
        msg.Append("substream", request->GetCustomData()[1]);
        break;
    default:
        break;
    }

    return msg;
}

}
