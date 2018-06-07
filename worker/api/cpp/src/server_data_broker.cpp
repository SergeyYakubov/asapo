#include "server_data_broker.h"

#include <chrono>

#include <json_parser/json_parser.h>

#include "io/io_factory.h"

#include "http_client/http_error.h"

using std::chrono::high_resolution_clock;

namespace asapo {

Error HttpCodeToWorkerError(const HttpCode& code) {
    const char* message;
    switch (code) {
    case HttpCode::OK:
        return nullptr;
    case HttpCode::BadRequest:
        message = WorkerErrorMessage::kWrongInput;
        break;
    case HttpCode::InternalServerError:
        message = WorkerErrorMessage::kErrorReadingSource;
        break;
    case HttpCode::NotFound:
        message = WorkerErrorMessage::kErrorReadingSource;
        break;
    case HttpCode::Conflict:
        message = WorkerErrorMessage::kNoData;
        return TextErrorWithType(message, ErrorType::kEndOfFile);
    default:
        message = WorkerErrorMessage::kErrorReadingSource;
        break;
    }
    return Error{new HttpError(message, code)};
}

ServerDataBroker::ServerDataBroker(const std::string& server_uri,
                                   const std::string& source_name) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    server_uri_{server_uri}, source_name_{source_name} {
}

Error ServerDataBroker::Connect() {
    return nullptr;
}

void ServerDataBroker::SetTimeout(uint64_t timeout_ms) {
    timeout_ms_ = timeout_ms;
}

std::string GetIDFromJson(const std::string& json_string, Error* err) {
    JsonStringParser parser(json_string);
    uint64_t id;
    if ((*err = parser.GetUInt64("id", &id)) != nullptr) {
        return "";
    }
    return std::to_string(id);
}

void ServerDataBroker::ProcessServerError(Error* err, const std::string& response, std::string* op) {
    (*err)->Append(response);
    if ((*err)->GetErrorType() == asapo::ErrorType::kEndOfFile) {
        if (response.find("id") != std::string::npos) {
            Error parse_error;
            auto id = GetIDFromJson(response, &parse_error);
            if (parse_error) {
                (*err)->Append(parse_error->Explain());
                return;
            }
            *op = id;
        }
    }
    return;
}

Error ServerDataBroker::ProcessRequest(std::string* response, std::string request_uri) {
    Error err;
    HttpCode code;
    *response = httpclient__->Get(request_uri, &code, &err);
    if (err != nullptr) {
        return err;
    }
    return HttpCodeToWorkerError(code);
}

Error ServerDataBroker::GetBrokerUri() {
    if (!current_broker_uri_.empty()) {
        return nullptr;
    }

    std::string request_uri = server_uri_ + "/discovery/broker";
    Error err;
    err = ProcessRequest(&current_broker_uri_, request_uri);
    if (err != nullptr || current_broker_uri_.empty()) {
        current_broker_uri_ = "";
        return TextError("cannot get broker uri from " + server_uri_);
    }
    return nullptr;
}


Error ServerDataBroker::GetFileInfoFromServer(FileInfo* info, const std::string& operation) {
    std::string request_suffix = operation;
    uint64_t elapsed_ms = 0;
    std::string response;
    while (true) {
        auto err = GetBrokerUri();
        if (err == nullptr) {
            std::string request_api = current_broker_uri_ + "/database/" + source_name_ + "/";
            err = ProcessRequest(&response, request_api + request_suffix);
            if (err == nullptr) {
                break;
            }
        }

        ProcessServerError(&err, response, &request_suffix);

        if (elapsed_ms >= timeout_ms_) {
            err = TextErrorWithType("exit on timeout, last error: " + err->Explain(), asapo::ErrorType::kTimeOut);
            return err;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
    }

    if (!info->SetFromJson(response)) {
        return TextError(WorkerErrorMessage::kErrorReadingSource);
    }
    return nullptr;
}

Error ServerDataBroker::GetNext(FileInfo* info, FileData* data) {
    if (info == nullptr) {
        return TextError(WorkerErrorMessage::kWrongInput);
    }

    auto err = GetFileInfoFromServer(info, "next");
    if (err != nullptr) {
        return err;
    }

    if (data == nullptr) {
        return nullptr;
    }

    Error error;
    *data = io__->GetDataFromFile(info->FullName(""), info->size, &error);
    return error;
}

}
