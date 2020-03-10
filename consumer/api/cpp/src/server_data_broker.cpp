#include "server_data_broker.h"

#include <chrono>

#include <json_parser/json_parser.h>
#include "io/io_factory.h"
#include "http_client/http_error.h"
#include "tcp_client.h"

#include "asapo_consumer.h"

using std::chrono::system_clock;

namespace asapo {

const std::string ServerDataBroker::kBrokerDerviceName = "broker";
const std::string ServerDataBroker::kFileTransferService_name = "fts";

Error GetNoDataResponseFromJson(const std::string& json_string, ConsumerErrorData* data) {
    JsonStringParser parser(json_string);
    Error err;
    if ((err = parser.GetUInt64("id", &data->id)) || (err = parser.GetUInt64("id_max", &data->id_max))
            || (err = parser.GetString("next_substream", &data->next_substream))) {
        return err;
    }
    return nullptr;
}

Error ErrorFromNoDataResponse(const std::string& response) {
    if (response.find("get_record_by_id") != std::string::npos) {
        ConsumerErrorData data;
        auto parse_error = GetNoDataResponseFromJson(response, &data);
        if (parse_error) {
            return ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
        }
        Error err;
        if (data.id >= data.id_max) {
            err = data.next_substream.empty() ? ConsumerErrorTemplates::kEndOfStream.Generate() :
                  ConsumerErrorTemplates::kStreamFinished.Generate();
        } else {
            err = ConsumerErrorTemplates::kNoData.Generate();
        }
        ConsumerErrorData* error_data = new ConsumerErrorData{data};
        err->SetCustomData(std::unique_ptr<CustomErrorData> {error_data});
        return err;
    }
    return ConsumerErrorTemplates::kNoData.Generate();
}

Error ErrorFromServerResponce(const RequestOutput* response, const HttpCode& code) {
    switch (code) {
    case HttpCode::OK:
        return nullptr;
    case HttpCode::BadRequest:
        return ConsumerErrorTemplates::kWrongInput.Generate(response->to_string());
    case HttpCode::Unauthorized:
        return ConsumerErrorTemplates::kWrongInput.Generate(response->to_string());
    case HttpCode::InternalServerError:
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response->to_string());
    case HttpCode::NotFound:
        return ConsumerErrorTemplates::kUnavailableService.Generate(response->to_string());
    case HttpCode::Conflict:
        return ErrorFromNoDataResponse(response->to_string());
    default:
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response->to_string());
    }
}

ServerDataBroker::ServerDataBroker(std::string server_uri,
                                   std::string source_path,
                                   SourceCredentials source) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    net_client__{new TcpClient()},
endpoint_{std::move(server_uri)}, source_path_{std::move(source_path)}, source_credentials_(std::move(source)) {

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

Error ServerDataBroker::ProcessRequest(RequestOutput* response, const RequestInfo& request, std::string* service_uri) {
    Error err;
    HttpCode code;
    if (request.post) {
        switch (request.output_mode) {
        case OutputDataMode::string:
            response->string_output =
                httpclient__->Post(RequestWithToken(request.host + request.api) + request.extra_params,
                                   request.cookie,
                                   request.body,
                                   &code,
                                   &err);
            break;
        }
    } else {
        response->string_output =
            httpclient__->Get(RequestWithToken(request.host + request.api) + request.extra_params, &code, &err);
    }
    if (err != nullptr) {
        if (service_uri) {
            service_uri->clear();
        }
        if (err == HttpErrorTemplates::kTransferError) {
            return ConsumerErrorTemplates::kInterruptedTransaction.Generate(
                       "error processing request: " + err->Explain());
        } else {
            return ConsumerErrorTemplates::kUnavailableService.Generate("error processing request: " + err->Explain());
        }
    }
    return ErrorFromServerResponce(response, code);
}

Error ServerDataBroker::DiscoverService(const std::string& service_name, std::string* uri_to_set) {
    if (!uri_to_set->empty()) {
        return nullptr;
    }

    RequestInfo ri;
    ri.host = endpoint_;
    ri.api = "/discovery/" + service_name;
    RequestOutput output;
    Error err;
    err = ProcessRequest(&output, ri, nullptr);
    *uri_to_set = std::move(output.string_output);
    if (err != nullptr || uri_to_set->empty()) {
        uri_to_set->clear();
        return ConsumerErrorTemplates::kUnavailableService.Generate(" on " + endpoint_
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

Error ServerDataBroker::GetRecordFromServer(std::string* response, std::string group_id, std::string substream,
                                            GetImageServerOperation op,
                                            bool dataset) {
    std::string request_suffix = OpToUriCmd(op);
    std::string request_api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream
                              + "/" + std::move(substream) +
                              +"/" + std::move(group_id) + "/";
    uint64_t elapsed_ms = 0;
    Error no_data_error;
    while (true) {
        auto start = system_clock::now();
        auto err = DiscoverService(kBrokerDerviceName, &current_broker_uri_);
        if (err == nullptr) {
            auto ri = PrepareRequestInfo(request_api + request_suffix, dataset);
            RequestOutput output;
            err = ProcessRequest(&output, ri, &current_broker_uri_);
            *response = std::move(output.string_output);
            if (err == nullptr) {
                break;
            }
        }

        if (err == ConsumerErrorTemplates::kStreamFinished) {
            return err;
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
        elapsed_ms += std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now() - start).count();
    }
    return nullptr;
}

Error ServerDataBroker::GetNext(FileInfo* info, std::string group_id, FileData* data) {
    return GetNext(info, std::move(group_id), kDefaultSubstream, data);
}

Error ServerDataBroker::GetNext(FileInfo* info, std::string group_id, std::string substream, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetNext,
                              0,
                              std::move(group_id),
                              std::move(substream),
                              info,
                              data);
}

Error ServerDataBroker::GetLast(FileInfo* info, std::string group_id, FileData* data) {
    return GetLast(info, std::move(group_id), kDefaultSubstream, data);
}

Error ServerDataBroker::GetLast(FileInfo* info, std::string group_id, std::string substream, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetLast,
                              0,
                              std::move(group_id),
                              std::move(substream),
                              info,
                              data);
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
                                           std::string substream,
                                           FileInfo* info,
                                           FileData* data) {
    if (info == nullptr) {
        return ConsumerErrorTemplates::kWrongInput.Generate();
    }

    Error err;
    std::string response;
    if (op == GetImageServerOperation::GetID) {
        err = GetRecordFromServerById(id, &response, std::move(group_id), std::move(substream));
    } else {
        err = GetRecordFromServer(&response, std::move(group_id), std::move(substream), op);
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

Error ServerDataBroker::ServiceRequestWithTimeout(const std::string& service_name,
                                                  std::string* service_uri,
                                                  RequestInfo request,
                                                  RequestOutput* response) {
    uint64_t elapsed_ms = 0;
    Error err;
    while (elapsed_ms <= timeout_ms_) {
        auto start = system_clock::now();
        err = DiscoverService(service_name, service_uri);
        if (err == nullptr) {
            request.host = *service_uri;
            err = ProcessRequest(response, request, service_uri);
            if (err == nullptr || err == ConsumerErrorTemplates::kWrongInput) {
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now() - start).count();
    }
    return err;
}

std::string ServerDataBroker::BrokerRequestWithTimeout(RequestInfo request, Error* err) {
    RequestOutput response;
    *err = ServiceRequestWithTimeout(kBrokerDerviceName, &current_broker_uri_, request, &response);
    if (*err) {
        return "";
    }
    return std::move(response.string_output);
}

Error ServerDataBroker::SetLastReadMarker(uint64_t value, std::string group_id) {
    return SetLastReadMarker(value, std::move(group_id), kDefaultSubstream);
}

Error ServerDataBroker::ResetLastReadMarker(std::string group_id) {
    return ResetLastReadMarker(std::move(group_id), kDefaultSubstream);
}

Error ServerDataBroker::ResetLastReadMarker(std::string group_id, std::string substream) {
    return SetLastReadMarker(0, group_id, substream);
}

Error ServerDataBroker::SetLastReadMarker(uint64_t value, std::string group_id, std::string substream) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/"
             + std::move(substream) + "/" + std::move(group_id) + "/resetcounter";
    ri.extra_params = "&value=" + std::to_string(value);
    ri.post = true;

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

uint64_t ServerDataBroker::GetCurrentSize(std::string substream, Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
             +"/" + std::move(substream) + "/size";
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

uint64_t ServerDataBroker::GetCurrentSize(Error* err) {
    return GetCurrentSize(kDefaultSubstream, err);
}
Error ServerDataBroker::GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) {
    return GetById(id, info, std::move(group_id), kDefaultSubstream, data);
}

Error ServerDataBroker::GetById(uint64_t id,
                                FileInfo* info,
                                std::string group_id,
                                std::string substream,
                                FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetID, id, group_id, substream, info, data);
}

Error ServerDataBroker::GetRecordFromServerById(uint64_t id, std::string* response, std::string group_id,
                                                std::string substream,
                                                bool dataset) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
             +"/" + std::move(substream) +
             "/" + std::move(
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
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/default/0/meta/0";

    return BrokerRequestWithTimeout(ri, err);
}

DataSet ServerDataBroker::DecodeDatasetFromResponse(std::string response, Error* err) {
    DataSet res;
    if (!res.SetFromJson(std::move(response))) {
        *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response:" + response);
        return {0, FileInfos{}};
    } else {
        *err = nullptr;
        return res;
    }
}

FileInfos ServerDataBroker::QueryImages(std::string query, std::string substream, Error* err) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
             "/" + std::move(substream) + "/0/queryimages";
    ri.post = true;
    ri.body = std::move(query);

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return FileInfos{};
    }

    auto dataset = DecodeDatasetFromResponse("{\"_id\":0, \"images\":" + response + "}", err);
    return dataset.content;
}

FileInfos ServerDataBroker::QueryImages(std::string query, Error* err) {
    return QueryImages(std::move(query), kDefaultSubstream, err);
}

DataSet ServerDataBroker::GetNextDataset(std::string group_id, Error* err) {
    return GetNextDataset(std::move(group_id), kDefaultSubstream, err);
}

DataSet ServerDataBroker::GetNextDataset(std::string group_id, std::string substream, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetNext, 0, std::move(group_id), std::move(substream), err);
}

DataSet ServerDataBroker::GetLastDataset(std::string group_id, std::string substream, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetLast, 0, std::move(group_id), std::move(substream), err);
}

DataSet ServerDataBroker::GetLastDataset(std::string group_id, Error* err) {
    return GetLastDataset(std::move(group_id), kDefaultSubstream, err);
}

DataSet ServerDataBroker::GetDatasetFromServer(GetImageServerOperation op,
                                               uint64_t id,
                                               std::string group_id, std::string substream,
                                               Error* err) {
    FileInfos infos;
    std::string response;
    if (op == GetImageServerOperation::GetID) {
        *err = GetRecordFromServerById(id, &response, std::move(group_id), std::move(substream), true);
    } else {
        *err = GetRecordFromServer(&response, std::move(group_id), std::move(substream), op, true);
    }
    if (*err != nullptr) {
        return {0, FileInfos{}};
    }
    return DecodeDatasetFromResponse(response, err);
}

DataSet ServerDataBroker::GetDatasetById(uint64_t id, std::string group_id, Error* err) {
    return GetDatasetById(id, std::move(group_id), kDefaultSubstream, err);
}

DataSet ServerDataBroker::GetDatasetById(uint64_t id, std::string group_id, std::string substream, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetID, id, std::move(group_id), std::move(substream), err);
}

std::vector<std::string> ParseSubstreamsFromResponse(std::string response, Error* err) {
    auto parser = JsonStringParser(std::move(response));
    std::vector<std::string> substreams;
    *err = parser.GetArrayString("substreams", &substreams);
    if (*err) {
        return std::vector<std::string> {};
    }
    return substreams;
}

std::vector<std::string> ServerDataBroker::GetSubstreamList(Error* err) {

    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/0/substreams";
    ri.post = false;

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return std::vector<std::string> {};
    }

    return ParseSubstreamsFromResponse(std::move(response), err);
}

}
