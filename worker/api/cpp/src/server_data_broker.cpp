#include "server_data_broker.h"

#include <chrono>

#include <json_parser/json_parser.h>

#include "io/io_factory.h"

#include "http_client/http_error.h"
#include "tcp_client.h"

#include <iostream>

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
    case HttpCode::Unauthorized:
        message = WorkerErrorMessage::kAuthorizationError;
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
        message = WorkerErrorMessage::kUnknownIOError;
        break;
    }
    return Error{new HttpError(message, code)};
}

ServerDataBroker::ServerDataBroker(std::string server_uri,
                                   std::string source_path,
                                   std::string source_name,
                                   std::string token) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    net_client__{new TcpClient()},
server_uri_{std::move(server_uri)}, source_path_{std::move(source_path)}, source_name_{std::move(source_name)}, token_{std::move(token)} {
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

std::string ServerDataBroker::RequestWithToken(std::string uri) {
    return std::move(uri) + "?token=" + token_;
}

Error ServerDataBroker::ProcessRequest(std::string* response, std::string request_uri, bool post) {
    Error err;
    HttpCode code;
    if (post) {
        *response = httpclient__->Post(RequestWithToken(request_uri), "", &code, &err);
    } else {
        *response = httpclient__->Get(RequestWithToken(request_uri), &code, &err);
    }
    if (err != nullptr) {
        current_broker_uri_ = "";
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
    err = ProcessRequest(&current_broker_uri_, request_uri, false);
    if (err != nullptr || current_broker_uri_.empty()) {
        current_broker_uri_ = "";
        return TextError("cannot get broker uri from " + server_uri_);
    }
    return nullptr;
}


Error ServerDataBroker::GetFileInfoFromServer(FileInfo* info, std::string group_id, GetImageServerOperation op) {
    std::string request_suffix = std::move(group_id) + "/" + OpToUriCmd(op);
    uint64_t elapsed_ms = 0;
    std::string response;
    while (true) {
        auto err = GetBrokerUri();
        if (err == nullptr) {
            std::string request_api = current_broker_uri_ + "/database/" + source_name_ + "/";
            err = ProcessRequest(&response, request_api + request_suffix, false);
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

Error ServerDataBroker::GetNext(FileInfo* info, std::string group_id, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetNext, std::move(group_id), info, data);
}

Error ServerDataBroker::GetLast(FileInfo* info, std::string group_id, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetLast, std::move(group_id), info, data);
}

std::string ServerDataBroker::OpToUriCmd(GetImageServerOperation op) {
    switch (op) {
    case GetImageServerOperation::GetNext:
        return "next";
    case GetImageServerOperation::GetLast:
        return "last";
    }
    return "";
}

Error ServerDataBroker::GetImageFromServer(GetImageServerOperation op, std::string group_id, FileInfo* info,
                                           FileData* data) {
    if (info == nullptr) {
        return TextError(WorkerErrorMessage::kWrongInput);
    }

    auto err = GetFileInfoFromServer(info, std::move(group_id), op);
    if (err != nullptr) {
        return err;
    }

    return GetDataIfNeeded(info, data);
}

Error ServerDataBroker::GetDataIfNeeded(FileInfo* info, FileData* data) {
    if (data == nullptr) {
        return nullptr;
    }

    if (DataCanBeInBuffer(info)) {
        if (TryGetDataFromBuffer(info, data) == nullptr) {
            return nullptr;
        } else {
            info->buf_id = 0;
        }
    }

    Error error;
    *data = io__->GetDataFromFile(info->FullName(source_path_), &info->size, &error);
    return error;
}

bool ServerDataBroker::DataCanBeInBuffer(const FileInfo* info) {
    return info->buf_id > 0;
}

Error ServerDataBroker::TryGetDataFromBuffer(const FileInfo* info, FileData* data) {
    return net_client__->GetData(info, data);
}


std::string ServerDataBroker::GenerateNewGroupId(Error* err) {
    return BrokerRequestWithTimeout("creategroup",true,err);
}

std::string ServerDataBroker::BrokerRequestWithTimeout(std::string request_string, bool post_request, Error* err) {
    uint64_t elapsed_ms = 0;
    std::string response;
    while (elapsed_ms <= timeout_ms_) {
        *err = GetBrokerUri();
        if (*err == nullptr) {
            *err = ProcessRequest(&response, current_broker_uri_ + "/" + request_string, post_request);
            if (*err == nullptr || (*err)->GetErrorType() == ErrorType::kEndOfFile) {
                return response;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
    }
    *err = TextErrorWithType("exit on timeout, last error: " + (*err)->Explain(), asapo::ErrorType::kTimeOut);
    return "";
}

Error ServerDataBroker::ResetCounter(std::string group_id) {
    std::string request_string =  "database/" + source_name_+"/"+std::move(group_id) + "/resetcounter";
    Error err;
    BrokerRequestWithTimeout(request_string,true,&err);
    return err;
}

uint64_t ServerDataBroker::GetNDataSets(Error* err) {
    std::string request_string =  "database/" + source_name_+"/size";
    auto responce = BrokerRequestWithTimeout(request_string,false,err);
    if (*err) {
        return 0;
    }
    JsonStringParser parser(responce);
    uint64_t size;
    if ((*err = parser.GetUInt64("size", &size)) != nullptr) {
        return 0;
    }
    return size;

}
Error ServerDataBroker::GetById(uint64_t id, FileInfo* info, FileData* data) {
    std::string request_string =  "database/" + source_name_+"/"+std::to_string(id);
    Error err;
    auto responce = BrokerRequestWithTimeout(request_string,false,&err);
    if (err) {
        return err;
    }

    if (!info->SetFromJson(responce)) {
        return TextError(WorkerErrorMessage::kErrorReadingSource);
    }

    return nullptr;
}

}
