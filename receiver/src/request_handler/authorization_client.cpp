#include "authorization_client.h"

#include <chrono>

#include "asapo/json_parser/json_parser.h"

#include "asapo/common/internal/version.h"

#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

namespace asapo {

std::string GetRequestString(const Request *request, const std::string &source_credentials) {
    auto api_version = VersionToNumber(request->GetApiVersion());
    std::string request_string;
    if (api_version < 6) {         // old approach, need to add instanceId and step, deprecates 01.03.2023
        std::string updated_source_credentials_prefix = source_credentials;
        std::string uri = request->GetOriginUri();
        updated_source_credentials_prefix.insert(source_credentials.find("%")+1,uri+"%DefaultStep%");
        request_string = std::string("{\"SourceCredentials\":\"") +
            updated_source_credentials_prefix + "\",\"OriginHost\":\"" + uri + "\"}";
    } else {
        request_string = std::string("{\"SourceCredentials\":\"") +
            source_credentials + "\",\"OriginHost\":\"" + request->GetOriginUri() + "\"}";
    }
    return request_string;
}

Error ErrorFromAuthorizationServerResponse(Error err, const std::string response,
                                           HttpCode code) {
    Error return_err;
    if (err) {
        return_err = asapo::ReceiverErrorTemplates::kInternalServerError.Generate(
            "cannot authorize request");
        return_err->AddDetails("response", response);
        return_err->SetCause(std::move(err));
    } else {
        if (code != HttpCode::Unauthorized) {
            return_err = asapo::ReceiverErrorTemplates::kInternalServerError.Generate();
        } else {
            return_err = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
        }
        return_err->AddDetails("response", response)->AddDetails("errorCode", std::to_string(int(
            code)));
    }
    return return_err;
}

Error CheckAccessType(SourceType source_type, const std::vector<std::string> &access_types) {
    if (std::find(access_types.begin(), access_types.end(),
                  source_type == SourceType::kProcessed ? "write" : "writeraw") != access_types.end()) {
        return nullptr;
    } else {
        auto err = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate("wrong access types");
        std::string types;
        for (size_t i = 0; i < access_types.size(); i++) {
            types += (i > 0 ? "," : "") + access_types[i];
        }
        err->AddDetails("expected", source_type == SourceType::kProcessed ? "write" : "writeraw")->AddDetails("have",
                                                                                                              types);
        return err;
    }
}

Error ParseServerResponse(const std::string &response,
                          HttpCode code,
                          std::vector<std::string> *access_types,
                          AuthorizationData *data) {
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
        (err = parser.GetString("instanceId", &data->producer_instance_id)) ||
        (err = parser.GetString("pipelineStep", &data->pipeline_step_id)) ||
        (err = parser.GetString("beamline", &data->beamline));
    if (err) {
        return ErrorFromAuthorizationServerResponse(std::move(err), response, code);
    }
    return nullptr;
}

Error UpdateDataFromServerResponse(const std::string &response, HttpCode code, AuthorizationData *data) {
    Error err;
    std::string stype;
    std::vector<std::string> access_types;
    AuthorizationData old_data = *data;
    err = ParseServerResponse(response, code, &access_types, data);
    if (err) {
        *data = old_data;
        return err;
    }

    err = CheckAccessType(data->source_type, access_types);
    if (err) {
        *data = old_data;
        return err;
    }
    data->last_update = std::chrono::system_clock::now();
    return nullptr;
}

Error AuthorizationClient::DoServerRequest(const std::string &request_string,
                                           std::string *response,
                                           HttpCode *code) const {
    Error err;
    *response =
        http_client__->Post(GetReceiverConfig()->authorization_server + "/authorize", "", request_string, code,
                            &err);
    if (err || *code != HttpCode::OK) {
        auto auth_error = ErrorFromAuthorizationServerResponse(std::move(err), *response, *code);
        return auth_error;
    }
    return nullptr;
}

Error AuthorizationClient::Authorize(const Request *request, AuthorizationData *data) const {
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
