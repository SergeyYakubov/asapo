#include "request_handler_authorize.h"
#include "receiver_config.h"
#include "receiver_logger.h"
#include "request.h"

#include "json_parser/json_parser.h"

using std::chrono::system_clock;

namespace asapo {

std::string RequestHandlerAuthorize::GetRequestString(const Request* request, const char* source_credentials) const {
    std::string request_string = std::string("{\"SourceCredentials\":\"") +
                                 source_credentials + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    return request_string;
}

Error RequestHandlerAuthorize::ErrorFromAuthorizationServerResponse(const Error& err, HttpCode code) const {
    if (err) {
        return asapo::ReceiverErrorTemplates::kInternalServerError.Generate("cannot authorize request: " + err->Explain());
    } else {
        if (code != HttpCode::Unauthorized) {
            return asapo::ReceiverErrorTemplates::kInternalServerError.Generate("return code " + std::to_string(int(code)));
        }
        return asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate("return code " + std::to_string(int(code)));
    }
}

Error RequestHandlerAuthorize::Authorize(Request* request, const char* source_credentials) const {
    HttpCode code;
    Error err;
    std::string request_string = GetRequestString(request, source_credentials);

    auto response = http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", request_string, &code,
                                        &err);
    if (err || code != HttpCode::OK) {
        auto auth_error = ErrorFromAuthorizationServerResponse(err, code);
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: " + request_string +
                     " - " +
                     auth_error->Explain());
        return auth_error;
    }

    JsonStringParser parser{response};
    (err = parser.GetString("beamtimeId", &beamtime_id_)) ||
    (err = parser.GetString("stream", &stream_)) ||
    (err = parser.GetString("core-path", &offline_path_)) ||
    (err = parser.GetString("beamline-path", &online_path_)) ||
    (err = parser.GetString("beamline", &beamline_));
    if (err) {
        return ErrorFromAuthorizationServerResponse(err, code);
    } else {
        log__->Debug(std::string("authorized connection from ") + request->GetOriginUri() + " beamline: " +
                     beamline_ + ", beamtime id: " + beamtime_id_ + ", stream: " + stream_);
    }

    last_updated_ = system_clock::now();
    cached_source_credentials_ = source_credentials;

    return nullptr;
}

Error RequestHandlerAuthorize::ProcessAuthorizationRequest(Request* request) const {
    if (!cached_source_credentials_.empty()) {
        Error auth_error = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
        auth_error->Append("already authorized");
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " - " +
                     "already authorized");
        return auth_error;
    }

    return Authorize(request, request->GetMessage());
}

Error RequestHandlerAuthorize::ProcessOtherRequest(Request* request) const {
    if (cached_source_credentials_.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    }

    uint64_t elapsed_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
                          (system_clock::now() - last_updated_).count();
    if (elapsed_ms >= GetReceiverConfig()->authorization_interval_ms) {
        auto err = Authorize(request, cached_source_credentials_.c_str());
        if (err) {
            return err;
        }
    }
    request->SetBeamtimeId(beamtime_id_);
    request->SetBeamline(beamline_);
    request->SetStream(stream_);
    request->SetOfflinePath(offline_path_);
    request->SetOnlinePath(online_path_);
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
    return StatisticEntity::kNetwork;
}


}