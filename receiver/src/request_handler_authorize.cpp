#include "request_handler_authorize.h"
#include "receiver_config.h"
#include "receiver_logger.h"
#include "request.h"

#include "json_parser/json_parser.h"

using std::chrono::high_resolution_clock;

namespace asapo {

std::string RequestHandlerAuthorize::GetRequestString(const Request* request, const char* beamtime_id) const {
    std::string request_string = std::string("{\"BeamtimeId\":\"") +
                                 beamtime_id + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    return request_string;
}

Error RequestHandlerAuthorize::ErrorFromServerResponse(const Error& err, HttpCode code) const {
    Error auth_error = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    if (err) {
        auth_error->Append(err->Explain());
        return auth_error;
    } else {
        auth_error->Append("return code " + std::to_string(int(code)));
        return auth_error;
    }
}

Error RequestHandlerAuthorize::Authorize(Request* request, const char* beamtime_id) const {
    HttpCode code;
    Error err;
    std::string request_string = GetRequestString(request, beamtime_id);

    auto response = http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", request_string, &code,
                                        &err);
    if (err || code != HttpCode::OK) {
        auto auth_error = ErrorFromServerResponse(err, code);
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: " + request_string +
                     " - " +
                     auth_error->Explain());
        return auth_error;
    }

    last_updated_ = high_resolution_clock::now();

    JsonStringParser parser{response};
    (err = parser.GetString("BeamtimeId", &beamtime_id_)) ||
    (err = parser.GetString("Beamline", &beamline_));
    if (err) {
        return ErrorFromServerResponse(err, code);
    } else {
        log__->Debug(std::string("authorized connection from ") + request->GetOriginUri() + " beamline: " +
                     beamline_ + ", beamtime id: " + beamtime_id_);
    }

    return nullptr;
}

Error RequestHandlerAuthorize::ProcessAuthorizationRequest(Request* request) const {
    if (!beamtime_id_.empty()) {
        Error auth_error = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
        auth_error->Append("already authorized");
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " - " +
                     "already authorized");
        return auth_error;
    }

    return Authorize(request, request->GetMessage());
}

Error RequestHandlerAuthorize::ProcessOtherRequest(Request* request) const {
    if (beamtime_id_.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    }

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>
                      (high_resolution_clock::now() - last_updated_).count();
    if (elapsed_ms >= GetReceiverConfig()->authorization_interval_ms) {
        auto err = Authorize(request, beamtime_id_.c_str());
        if (err) {
            return err;
        }
    }
    request->SetBeamtimeId(beamtime_id_);
    request->SetBeamline(beamline_);
    return nullptr;
}


Error RequestHandlerAuthorize::ProcessRequest(Request* request) const {
    if (request->GetOpCode() == kOpcodeAuthorize) {
        return ProcessAuthorizationRequest(request);
    } else {
        return ProcessOtherRequest(request);
    }
}

RequestHandlerAuthorize::RequestHandlerAuthorize(): log__{GetDefaultReceiverLogger()},
    http_client__{DefaultHttpClient()} {
}

StatisticEntity RequestHandlerAuthorize::GetStatisticEntity() const {
    return StatisticEntity::kAuthorizer;
}


}