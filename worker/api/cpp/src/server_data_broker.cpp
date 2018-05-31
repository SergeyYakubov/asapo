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

void ServerDataBroker::ProcessServerError(Error* err,const std::string& response,std::string* redirect_uri) {
    if ((*err)->GetErrorType() != asapo::ErrorType::kEndOfFile) {
        (*err)->Append(response);
        return;
    } else {
        if (response.find("id") != std::string::npos) {
            auto id = GetIDFromJson(response, err);
            if (*err) {
                return;
            }
            *redirect_uri = server_uri_ + "/database/" + source_name_ + "/" + id;
        }
    }
    *err=nullptr;
    return;
}

Error ServerDataBroker::ProcessRequest(std::string* response,std::string request_uri) {
    Error err;
    HttpCode code;
    *response = httpclient__->Get(request_uri, &code, &err);
    if (err != nullptr) {
        return err;
    }
    return HttpCodeToWorkerError(code);
}

Error ServerDataBroker::GetFileInfoFromServer(FileInfo* info, const std::string& operation) {
    std::string request_uri = server_uri_ + "/database/" + source_name_ + "/" + operation;
    uint64_t elapsed_ms = 0;
    std::string response;
    while (true) {
        auto err = ProcessRequest(&response,request_uri);
        if (err == nullptr) {
            break;
        }

        ProcessServerError(&err,response,&request_uri);
        if (err != nullptr) {
            return err;
        }

        if (elapsed_ms >= timeout_ms_) {
            err = TextErrorWithType("no more data found, exit on timeout", asapo::ErrorType::kTimeOut);
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
