#include "authorization_client.h"

#include "asapo/json_parser/json_parser.h"


#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"


namespace asapo {

std::string AuthorizationClient::GetRequestString(const Request* request, const std::string& source_credentials) const {
    std::string request_string = std::string("{\"SourceCredentials\":\"") +
        source_credentials + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    return request_string;
}

Error AuthorizationClient::ErrorFromAuthorizationServerResponse(const Error& err, const std::string response,
                                                                    HttpCode code) const {
    if (err) {
        return asapo::ReceiverErrorTemplates::kInternalServerError.Generate(
            "cannot authorize request: " + err->Explain());
    } else {
        if (code != HttpCode::Unauthorized) {
            return asapo::ReceiverErrorTemplates::kInternalServerError.Generate(
                response + " return code " + std::to_string(int(
                    code)));
        }
        return asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate(response);
    }
}

Error CheckAccessType(SourceType source_type, const std::vector<std::string>& access_types) {
    if (std::find(access_types.begin(), access_types.end(),
                  source_type == SourceType::kProcessed ? "write" : "writeraw") != access_types.end()) {
        return nullptr;
    } else {
        return asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate("wrong access types");
    }
}

LogMessageWithFields AuthErrorLogMsg(const Request* request, const Error& err, const std::string& request_string) {
    return RequestLog("failure authorizing: " + err->Explain(), request)
        .Append("authServer", GetReceiverConfig()->authorization_server)
        .Append("request", request_string);
}

Error AuthorizationClient::Authorize(const Request* request, std::string source_credentials, AuthorizationData* data) const {
    HttpCode code;
    Error err;
    std::string request_string = GetRequestString(request, std::move(source_credentials));
    auto response =
        http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", "", request_string, &code,
                            &err);
    if (err || code != HttpCode::OK) {
        auto auth_error = ErrorFromAuthorizationServerResponse(err, response, code);
        log__->Error(AuthErrorLogMsg(request, auth_error, request_string));
        return auth_error;
    }

    std::string stype;
    std::vector<std::string> access_types;

    AuthorizationData creds;
    JsonStringParser parser{response};
    (err = parser.GetString("beamtimeId", &creds.beamtime_id)) ||
        (err = parser.GetString("dataSource", &creds.data_source)) ||
        (err = parser.GetString("corePath", &creds.offline_path)) ||
        (err = parser.GetString("beamline-path", &creds.online_path)) ||
        (err = parser.GetString("source-type", &stype)) ||
        (err = parser.GetArrayString("access-types", &access_types)) ||
        (err = GetSourceTypeFromString(stype, &creds.source_type)) ||
        (err = parser.GetString("beamline", &creds.beamline));
    if (err) {
        return ErrorFromAuthorizationServerResponse(err, "", code);
    }

    err = CheckAccessType(creds.source_type, access_types);
    if (err) {
        log__->Error(AuthErrorLogMsg(request, err, request_string));
        return err;
    }
    log__->Debug(RequestLog("authorized connection",request));
    *data = creds;
    return nullptr;
}

AuthorizationClient::AuthorizationClient() : log__{GetDefaultReceiverLogger()},
                                                     http_client__{DefaultHttpClient()} {
}

}
