#include "authorization_client.h"

#include <chrono>

#include "asapo/json_parser/json_parser.h"

#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

namespace asapo {

std::string GetRequestString(const Request* request, const std::string& source_credentials) {
    std::string request_string = std::string("{\"SourceCredentials\":\"") +
                                 source_credentials + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    return request_string;
}

Error ErrorFromAuthorizationServerResponse(const Error& err, const std::string response,
                                           HttpCode code) {
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

Error ParseServerResponse(const std::string& response,
                          HttpCode code,
                          std::vector<std::string>* access_types,
                          AuthorizationData* data) {
    Error err;
    AuthorizationData creds;
    JsonStringParser parser{response};
    std::string stype;

    (err = parser.GetString("beamtimeId", &data->beamtime_id)) ||
    (err = parser.GetString("dataSource", &data->data_source)) ||
    (err = parser.GetString("corePath", &data->offline_path)) ||
    (err = parser.GetString("beamline-path", &data->online_path)) ||
    (err = parser.GetString("source-type", &stype)) ||
    (err = parser.GetArrayString("access-types", access_types)) ||
    (err = GetSourceTypeFromString(stype, &data->source_type)) ||
    (err = parser.GetString("beamline", &data->beamline));
    if (err) {
        return ErrorFromAuthorizationServerResponse(err, "", code);
    }
    return nullptr;
}

Error UpdateDataFromServerResponse(const std::string& response, HttpCode code, AuthorizationData* data) {
    Error err;
    std::string stype;
    std::vector<std::string> access_types;
    AuthorizationData old_data = *data;
    err = ParseServerResponse(response, code, &access_types, data);
    if (err) {
        *data = old_data;
        return ErrorFromAuthorizationServerResponse(err, response, code);
    }

    err = CheckAccessType(data->source_type, access_types);
    if (err) {
        *data = old_data;
        return err;
    }
    data->last_update = std::chrono::system_clock::now();
    return nullptr;
}

Error AuthorizationClient::DoServerRequest(const std::string& request_string,
                                           std::string* response,
                                           HttpCode* code) const {
    Error err;
    *response =
        http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", "", request_string, code,
                            &err);
    if (err || *code != HttpCode::OK) {
        auto auth_error = ErrorFromAuthorizationServerResponse(err, *response, *code);
        return auth_error;
    }
    return nullptr;
}

Error AuthorizationClient::Authorize(const Request* request, AuthorizationData* data) const {
    HttpCode code;
    std::string response;
    std::string request_string = GetRequestString(request, data->source_credentials);
    auto err = DoServerRequest(request_string, &response, &code);
    if (err != nullptr) {
        return err;
    }

    err = UpdateDataFromServerResponse(response, code, data);
    if (err != nullptr) {
        return err;
    }
    log__->Debug(AuthorizationLog(
                     request->GetOpCode() == kOpcodeAuthorize ? "authorized connection" : "reauthorized connection", request, data));
    return nullptr;
}

AuthorizationClient::AuthorizationClient() : log__{GetDefaultReceiverLogger()},
    http_client__{DefaultHttpClient()} {
}

}
