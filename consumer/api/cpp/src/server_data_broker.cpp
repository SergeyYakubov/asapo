#include "server_data_broker.h"

#include <chrono>

#include <json_parser/json_parser.h>
#include "io/io_factory.h"
#include "http_client/http_error.h"
#include "tcp_client.h"

#include "asapo_consumer.h"

using std::chrono::system_clock;

namespace asapo {

Error GetIDsFromJson(const std::string& json_string, uint64_t* id, uint64_t* id_max) {
    JsonStringParser parser(json_string);
    Error err;
    if ((err = parser.GetUInt64("id", id)) || (err = parser.GetUInt64("id_max", id_max))) {
        return err;
    }
    return nullptr;
}

Error ErrorFromNoDataResponse(const std::string& response) {
    if (response.find("get_record_by_id") != std::string::npos) {
        uint64_t id, id_max;
        auto parse_error = GetIDsFromJson(response, &id, &id_max);
        if (parse_error) {
            return ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
        }
        Error err;
        if (id >= id_max ) {
            err = ConsumerErrorTemplates::kEndOfStream.Generate();
        } else {
            err = ConsumerErrorTemplates::kNoData.Generate();
        }
        ConsumerErrorData* error_data = new ConsumerErrorData;
        error_data->id = id;
        error_data->id_max = id_max;
        err->SetCustomData(std::unique_ptr<CustomErrorData> {error_data});
        return err;
    }
    return ConsumerErrorTemplates::kNoData.Generate();
}

Error ErrorFromServerResponce(const std::string& response, const HttpCode& code) {
    switch (code) {
    case HttpCode::OK:
        return nullptr;
    case HttpCode::BadRequest:
        return ConsumerErrorTemplates::kWrongInput.Generate(response);
    case HttpCode::Unauthorized:
        return ConsumerErrorTemplates::kWrongInput.Generate(response);
    case HttpCode::InternalServerError:
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response);
    case HttpCode::NotFound:
        return ConsumerErrorTemplates::kUnavailableService.Generate(response);
    case HttpCode::Conflict:
        return ErrorFromNoDataResponse(response);
    default:
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response);
    }
}

ServerDataBroker::ServerDataBroker(std::string server_uri,
                                   std::string source_path,
                                   SourceCredentials source) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    net_client__{new TcpClient()},
server_uri_{std::move(server_uri)}, source_path_{std::move(source_path)}, source_credentials_(std::move(source)) {

    if (source_credentials_.stream.empty()) {
        source_credentials_.stream = SourceCredentials::kDefaultStream;
    }

}

void ServerDataBroker::SetTimeout(uint64_t timeout_ms) {
    timeout_ms_ = timeout_ms;
}

std::string ServerDataBroker::RequestWithToken(std::string uri) {
    return std::move(uri) + "?token=" + source_credentials_.user_token;
}

Error ServerDataBroker::ProcessRequest(std::string* response, const RequestInfo& request) {
    Error err;
    HttpCode code;
    if (request.post) {
        *response =
            httpclient__->Post(RequestWithToken(request.host + request.api) + request.extra_params, request.body, &code,
                               &err);
    } else {
        *response = httpclient__->Get(RequestWithToken(request.host + request.api) + request.extra_params, &code, &err);
    }
    if (err != nullptr) {
        current_broker_uri_ = "";
        if (err == HttpErrorTemplates::kTransferError) {
            return ConsumerErrorTemplates::kInterruptedTransaction.Generate("error processing request: " + err->Explain());
        } else {
            return ConsumerErrorTemplates::kUnavailableService.Generate("error processing request: " + err->Explain());
        }
    }
    return ErrorFromServerResponce(*response, code);
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
        return ConsumerErrorTemplates::kUnavailableService.Generate(" on " + server_uri_
                + (err != nullptr ? ": " + err->Explain()
                   : ""));
    }
    return nullptr;
}

bool ServerDataBroker::SwitchToGetByIdIfNoData(Error* err, const std::string& response, std::string* redirect_uri) {
    if (*err == ConsumerErrorTemplates::kNoData) {
        auto error_data = static_cast<const ConsumerErrorData*>((*err)->GetCustomData());
        if (error_data == nullptr) {
            *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
            return false;
        }
        *redirect_uri = std::to_string(error_data->id);
        return true;
    }
    return false;
}

RequestInfo ServerDataBroker::PrepareRequestInfo(std::string api_url, bool dataset) {
    RequestInfo ri;
    ri.host = current_broker_uri_;
    ri.api = std::move(api_url);
    if (dataset) {
        ri.extra_params = "&dataset=true";
    }
    return ri;
}


Error ServerDataBroker::GetRecordFromServer(std::string* response, std::string group_id, GetImageServerOperation op,
                                            bool dataset) {
    std::string request_suffix = OpToUriCmd(op);
    std::string request_api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/" +
                              std::move(group_id) + "/";
    uint64_t elapsed_ms = 0;
    Error no_data_error;
    while (true) {
        auto start = system_clock::now();
        auto err = GetBrokerUri();
        if (err == nullptr) {
            auto  ri = PrepareRequestInfo(request_api + request_suffix, dataset);
            err = ProcessRequest(response, ri);
            if (err == nullptr) {
                break;
            }
        }

        if (request_suffix == "next") {
            auto save_error = SwitchToGetByIdIfNoData(&err, *response, &request_suffix);
            if (err == ConsumerErrorTemplates::kInterruptedTransaction) {
                return err;
            }
            if (save_error) {
                no_data_error = std::move(err);
            }
        }
        if (elapsed_ms >= timeout_ms_) {
            return no_data_error ? std::move(no_data_error) : std::move(err);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += std::chrono::duration_cast<std::chrono::milliseconds>( system_clock::now() - start).count();
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
        return ConsumerErrorTemplates::kWrongInput.Generate();
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
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(std::string("malformed response:") + response);
    }

    return GetDataIfNeeded(info, data);
}

Error ServerDataBroker::RetrieveData(FileInfo* info, FileData* data) {
    if (data == nullptr || info == nullptr) {
        return ConsumerErrorTemplates::kWrongInput.Generate("pointers are empty");
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
    if (error) {
        return ConsumerErrorTemplates::kLocalIOError.Generate(error->Explain());
    }

    return nullptr;
}

Error ServerDataBroker::GetDataIfNeeded(FileInfo* info, FileData* data) {
    if (data == nullptr) {
        return nullptr;
    }

    return RetrieveData(info, data);

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
        auto start = system_clock::now();
        *err = GetBrokerUri();
        if (*err == nullptr) {
            request.host = current_broker_uri_;
            *err = ProcessRequest(&response, request);
            if (*err == nullptr || (*err) == ConsumerErrorTemplates::kWrongInput) {
                return response;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += std::chrono::duration_cast<std::chrono::milliseconds>( system_clock::now() - start).count();
    }
    return "";
}

Error ServerDataBroker::SetLastReadMarker(uint64_t value, std::string group_id) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/" + std::move(
                 group_id) + "/resetcounter";
    ri.extra_params = "&value=" + std::to_string(value);
    ri.post = true;

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

Error ServerDataBroker::ResetLastReadMarker(std::string group_id) {
    return SetLastReadMarker(0, group_id);
}

uint64_t ServerDataBroker::GetCurrentSize(Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/size";
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
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/" + std::move(
                 group_id) + "/" + std::to_string(id);
    if (dataset) {
        ri.extra_params += "&dataset=true";
    }

    Error err;
    *response = BrokerRequestWithTimeout(ri, &err);
    return err;
}

std::string ServerDataBroker::GetBeamtimeMeta(Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/0/meta/0";

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
        *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response:" + parse_err->Explain());
        return {0, FileInfos{}};
    }

    auto res = FileInfos{};
    for (auto fi_encoded : vec_fi_endcoded) {
        FileInfo fi;
        if (!fi.SetFromJson(fi_encoded)) {
            *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response:" + fi_encoded);
            return {0, FileInfos{}};
        }
        res.emplace_back(fi);
    }
    return {id, std::move(res)};
}

FileInfos ServerDataBroker::QueryImages(std::string query, Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/0/queryimages";
    ri.post = true;
    ri.body = std::move(query);

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
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
