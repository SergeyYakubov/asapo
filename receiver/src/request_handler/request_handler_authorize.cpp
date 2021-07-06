#include "request_handler_authorize.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

#include "asapo/json_parser/json_parser.h"
#include "asapo/common/internal/version.h"

using std::chrono::system_clock;

namespace asapo {

std::string RequestHandlerAuthorize::GetRequestString(const Request* request, const char* source_credentials) const {
    std::string request_string = std::string("{\"SourceCredentials\":\"") +
                                 source_credentials + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    return request_string;
}

Error RequestHandlerAuthorize::ErrorFromAuthorizationServerResponse(const Error& err, const std::string response,
        HttpCode code) const {
    if (err) {
        return asapo::ReceiverErrorTemplates::kInternalServerError.Generate("cannot authorize request: " + err->Explain());
    } else {
        if (code != HttpCode::Unauthorized) {
            return asapo::ReceiverErrorTemplates::kInternalServerError.Generate(response + " return code " + std::to_string(int(
                        code)));
        }
        return asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate(response);
    }
}

Error CheckAccessType(const std::vector<std::string>& access_types) {
    if(std::find(access_types.begin(), access_types.end(), "write") != access_types.end()) {
        return nullptr;
    } else {
        return asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate("wrong access types");
    }
}


Error RequestHandlerAuthorize::Authorize(Request* request, const char* source_credentials) const {
    HttpCode code;
    Error err;
    std::string request_string = GetRequestString(request, source_credentials);

    auto response = http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", "", request_string, &code,
                                        &err);
    if (err || code != HttpCode::OK) {
        auto auth_error = ErrorFromAuthorizationServerResponse(err, response, code);
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: " + request_string +
                     " - " +
                     auth_error->Explain());
        return auth_error;
    }

    std::string stype;
    std::vector<std::string> access_types;

    JsonStringParser parser{response};
    (err = parser.GetString("beamtimeId", &beamtime_id_)) ||
    (err = parser.GetString("dataSource", &data_source_)) ||
    (err = parser.GetString("corePath", &offline_path_)) ||
    (err = parser.GetString("beamline-path", &online_path_)) ||
    (err = parser.GetString("source-type", &stype)) ||
    (err = parser.GetArrayString("access-types", &access_types)) ||
    (err = GetSourceTypeFromString(stype, &source_type_)) ||
    (err = parser.GetString("beamline", &beamline_));
    if (err) {
        return ErrorFromAuthorizationServerResponse(err, "", code);
    }

    err = CheckAccessType(access_types);
    if (err) {
        log__->Error("failure authorizing at " + GetReceiverConfig()->authorization_server + " request: " + request_string +
                     " - " +
                     err->Explain());
        return err;
    }

    log__->Debug(std::string("authorized connection from ") + request->GetOriginUri() + "source type: " + stype +
                 " beamline: " +
                 beamline_ + ", beamtime id: " + beamtime_id_ + ", data soucre: " + data_source_);

    last_updated_ = system_clock::now();
    cached_source_credentials_ = source_credentials;

    return nullptr;
}

Error RequestHandlerAuthorize::CheckVersion(const std::string& version_from_client) const {
    int verClient = VersionToNumber(version_from_client);
    int verService = VersionToNumber(GetReceiverApiVersion());
    if (verClient > verService) {
        auto err_string = "client version: " + version_from_client + ", server version: " + GetReceiverApiVersion();
        return asapo::ReceiverErrorTemplates::kUnsupportedClient.Generate(err_string);
        log__->Error("failure serving client - unsupported version,  " + err_string);
    }
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

    auto err = CheckVersion(request->GetApiVersion());
    if (err) {
        log__->Error("failure authorizing at client: " + err->Explain());
        return err;
    }

    return Authorize(request, request->GetMetaData().c_str());
}

Error RequestHandlerAuthorize::ProcessReAuthorization(Request* request) const {
    std::string old_beamtimeId = beamtime_id_;
    auto err = Authorize(request, cached_source_credentials_.c_str());
    if (err == asapo::ReceiverErrorTemplates::kAuthorizationFailure || (
                err == nullptr && old_beamtimeId != beamtime_id_)) {
        return asapo::ReceiverErrorTemplates::kReAuthorizationFailure.Generate();
    }
    return err;
}

bool RequestHandlerAuthorize::NeedReauthorize() const {
    uint64_t elapsed_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
                          (system_clock::now() - last_updated_).count();
    return elapsed_ms >= GetReceiverConfig()->authorization_interval_ms;
}


Error RequestHandlerAuthorize::ProcessOtherRequest(Request* request) const {
    if (cached_source_credentials_.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    }

    if (NeedReauthorize()) {
        auto err = ProcessReAuthorization(request);
        if (err) {
            return err;
        }
    }
    request->SetBeamtimeId(beamtime_id_);
    request->SetBeamline(beamline_);
    request->SetDataSource(data_source_);
    request->SetOfflinePath(offline_path_);
    request->SetOnlinePath(online_path_);
    request->SetSourceType(source_type_);
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
