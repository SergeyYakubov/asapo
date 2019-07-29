#include "server_data_broker.h"

#include <chrono>

#include <json_parser/json_parser.h>
#include "io/io_factory.h"
#include "http_client/http_error.h"
#include "tcp_client.h"


#include "asapo_worker.h"

using std::chrono::system_clock;

namespace asapo {

Error HttpCodeToWorkerError(const HttpCode& code) {
    switch (code) {
    case HttpCode::OK:
        return nullptr;
    case HttpCode::BadRequest:
        return WorkerErrorTemplates::kWrongInput.Generate();
    case HttpCode::Unauthorized:
        return WorkerErrorTemplates::kAuthorizationError.Generate();
    case HttpCode::InternalServerError:
        return WorkerErrorTemplates::kInternalError.Generate();
    case HttpCode::NotFound:
        return WorkerErrorTemplates::kErrorReadingSource.Generate();
    case HttpCode::Conflict:
        return asapo::ErrorTemplates::kEndOfFile.Generate("No Data");
    default:
        return WorkerErrorTemplates::kUnknownIOError.Generate();
    }
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

Error ServerDataBroker::ProcessRequest(std::string* response, const RequestInfo& request) {
    Error err;
    HttpCode code;
    if (request.post) {
        *response = httpclient__->Post(RequestWithToken(request.host + request.api) + request.extra_params, request.body, &code,
                                       &err);
    } else {
        *response = httpclient__->Get(RequestWithToken(request.host + request.api) + request.extra_params, &code, &err);
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

    RequestInfo ri;
    ri.host = server_uri_;
    ri.api = "/discovery/broker";

    Error err;
    err = ProcessRequest(&current_broker_uri_, ri);
    if (err != nullptr || current_broker_uri_.empty()) {
        current_broker_uri_ = "";
        return TextError("cannot get broker uri from " + server_uri_);
    }
    return nullptr;
}


Error ServerDataBroker::GetRecordFromServer(std::string* response, std::string group_id, GetImageServerOperation op,
                                            bool dataset) {
    std::string request_suffix = OpToUriCmd(op);
    std::string request_api = "/database/" + source_name_ + "/" + std::move(group_id) + "/";
    uint64_t elapsed_ms = 0;
    while (true) {
        auto err = GetBrokerUri();
        if (err == nullptr) {
            RequestInfo ri;
            ri.host = current_broker_uri_;
            ri.api = request_api + request_suffix;
            if (dataset) {
                ri.extra_params = "&dataset=true";
            }
            err = ProcessRequest(response, ri);
            if (err == nullptr) {
                break;
            }
        }

        ProcessServerError(&err, *response, &request_suffix);

        if (elapsed_ms >= timeout_ms_) {
            err = IOErrorTemplates::kTimeout.Generate( ", last error: " + err->Explain());
            return err;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
    }

    return nullptr;
}

Error ServerDataBroker::GetNext(FileInfo* info, std::string group_id, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetNext, 0, std::move(group_id), info, data);
}

Error ServerDataBroker::GetLast(FileInfo* info, std::string group_id, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetLast, 0, std::move(group_id), info, data);
}

std::string ServerDataBroker::OpToUriCmd(GetImageServerOperation op) {
    switch (op) {
    case GetImageServerOperation::GetNext:
        return "next";
    case GetImageServerOperation::GetLast:
        return "last";
    default:
        return "last";
    }
}

Error ServerDataBroker::GetImageFromServer(GetImageServerOperation op, uint64_t id, std::string group_id,
                                           FileInfo* info,
                                           FileData* data) {
    if (info == nullptr) {
        return WorkerErrorTemplates::kWrongInput.Generate();
    }

    Error err;
    std::string response;
    if (op == GetImageServerOperation::GetID) {
        err = GetRecordFromServerById(id, &response, std::move(group_id));
    } else {
        err = GetRecordFromServer(&response, std::move(group_id), op);
    }
    if (err != nullptr) {
        return err;
    }

    if (!info->SetFromJson(response)) {
        return WorkerErrorTemplates::kErrorReadingSource.Generate(std::string(":") + response);
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
    RequestInfo ri;
    ri.api = "/creategroup";
    ri.post = true;
    return BrokerRequestWithTimeout(ri, err);
}


std::string ServerDataBroker::AppendUri(std::string request_string) {
    return current_broker_uri_ + "/" + std::move(request_string);
}



std::string ServerDataBroker::BrokerRequestWithTimeout(RequestInfo request, Error* err) {
    uint64_t elapsed_ms = 0;
    std::string response;
    while (elapsed_ms <= timeout_ms_) {
        *err = GetBrokerUri();
        if (*err == nullptr) {
            request.host = current_broker_uri_;
            *err = ProcessRequest(&response, request);
            if (*err == nullptr || (*err)->GetErrorType() == ErrorType::kEndOfFile || (*err) == WorkerErrorTemplates::kWrongInput) {
                return response;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
    }
    *err = IOErrorTemplates::kTimeout.Generate( ", last error: " + (*err)->Explain());
    return "";
}

Error ServerDataBroker::ResetCounter(std::string group_id) {
    RequestInfo ri;
    ri.api = "/database/" + source_name_ + "/" + std::move(group_id) + "/resetcounter";
    ri.post = true;

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

uint64_t ServerDataBroker::GetNDataSets(Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_name_ + "/size";
    auto responce = BrokerRequestWithTimeout(ri, err);
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
Error ServerDataBroker::GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetID, id, group_id, info, data);
}


Error ServerDataBroker::GetRecordFromServerById(uint64_t id, std::string* response, std::string group_id,
                                                bool dataset) {
    RequestInfo ri;
    ri.api = "/database/" + source_name_ + "/" + std::move(group_id) + "/" + std::to_string(id);
    ri.extra_params = "&reset=true";
    if (dataset) {
        ri.extra_params += "&dataset=true";
    }

    Error err;
    *response = BrokerRequestWithTimeout(ri, &err);
    return err;
}

std::string ServerDataBroker::GetBeamtimeMeta(Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_name_ + "/0/meta/0";

    return BrokerRequestWithTimeout(ri, err);
}


DataSet ServerDataBroker::DecodeDatasetFromResponse(std::string response, Error* err) {
    auto parser = JsonStringParser(std::move(response));

    std::vector<std::string> vec_fi_endcoded;
    Error parse_err;
    uint64_t id;
    (parse_err = parser.GetArrayRawStrings("images", &vec_fi_endcoded)) ||
    (parse_err = parser.GetUInt64("_id", &id));
    if (parse_err) {
        *err = WorkerErrorTemplates::kInternalError.Generate("cannot parse response:" + parse_err->Explain());
        return {0, FileInfos{}};
    }

    auto res = FileInfos{};
    for (auto fi_encoded : vec_fi_endcoded) {
        FileInfo fi;
        if (!fi.SetFromJson(fi_encoded)) {
            *err = WorkerErrorTemplates::kInternalError.Generate("cannot parse response:" + fi_encoded);
            return {0, FileInfos{}};
        }
        res.emplace_back(fi);
    }
    return {id, std::move(res)};
}


FileInfos ServerDataBroker::QueryImages(std::string query, Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_name_ + "/0/queryimages";
    ri.post = true;
    ri.body = std::move(query);

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        (*err)->Append(response);
        return FileInfos{};
    }

    auto dataset = DecodeDatasetFromResponse("{\"_id\":0, \"images\":" + response + "}", err);
    return dataset.content;
}

DataSet ServerDataBroker::GetNextDataset(std::string group_id, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetNext, 0, std::move(group_id), err);
}

DataSet ServerDataBroker::GetDatasetFromServer(GetImageServerOperation op,
                                               uint64_t id,
                                               std::string group_id,
                                               Error* err) {
    FileInfos infos;
    std::string response;
    if (op == GetImageServerOperation::GetID) {
        *err = GetRecordFromServerById(id, &response, std::move(group_id), true);
    } else {
        *err = GetRecordFromServer(&response, std::move(group_id), op, true);
    }
    if (*err != nullptr) {
        return {0, FileInfos{}};
    }
    return DecodeDatasetFromResponse(response, err);
}
DataSet ServerDataBroker::GetLastDataset(std::string group_id, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetLast, 0, std::move(group_id), err);
}

DataSet ServerDataBroker::GetDatasetById(uint64_t id, std::string group_id, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetID, id, std::move(group_id), err);
}

}
