#include "request_handler_authorize.h"
#include "receiver_config.h"
#include "receiver_logger.h"
#include "request.h"


using std::chrono::high_resolution_clock;

namespace asapo {

Error RequestHandlerAuthorize::ProcessRequest(Request* request) const {
    Error auth_error = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    auto op_code = request->GetOpCode();
    if (op_code != kOpcodeAuthorize && beamtime_id_.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    }
    if ((op_code == kOpcodeAuthorize && beamtime_id_.empty()) ||
        (op_code != kOpcodeAuthorize && !beamtime_id_.empty())) {
            auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
            ( high_resolution_clock::now() - last_updated_).count();
            if (op_code == kOpcodeAuthorize || elapsed_ms>=GetReceiverConfig()->authorization_interval_ms) {
                HttpCode code;
                Error err;
                std::string request_string = std::string("{\"BeamtimeId\":\"") + (op_code == kOpcodeAuthorize?request->GetMessage():beamtime_id_)
                    + "\",\"OriginHost\":\""+
                    request->GetOriginUri()+"\"}";
                auto response = http_client__->Post(GetReceiverConfig()->authorization_server+"/authorize",request_string,&code,&err);
                if (err) {
                    auth_error->Append(err->Explain());
                    log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: "+request_string + " - " +
                        err->Explain());
                    return auth_error;
                }
                if (code != HttpCode::OK) {
                    log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: "+request_string + " - " +
                        "return code "+std::to_string(int(code)));
                    return auth_error;
                }
                last_updated_ = high_resolution_clock::now();
                beamtime_id_ = response;
            }
            request->SetBeamtimeId(beamtime_id_);
            return nullptr;
    }

    if (op_code == kOpcodeAuthorize && !beamtime_id_.empty()) {
        auth_error->Append("already authorized");
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " - " +
            "already authorized");
        return auth_error;

    }
    return nullptr;
}

RequestHandlerAuthorize::RequestHandlerAuthorize(): log__{GetDefaultReceiverLogger()},
                                                    http_client__{DefaultHttpClient()}{
}

StatisticEntity RequestHandlerAuthorize::GetStatisticEntity() const {
    return StatisticEntity::kAuthorizer;
}


}