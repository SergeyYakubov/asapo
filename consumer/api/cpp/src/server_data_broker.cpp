#include "server_data_broker.h"
#include "server_data_broker.h"

#include <chrono>

#include "asapo/json_parser/json_parser.h"
#include "asapo/io/io_factory.h"
#include "asapo/http_client/http_error.h"
#include "tcp_client.h"

#include "asapo/asapo_consumer.h"
#include "fabric_consumer_client.h"
#include "rds_response_error.h"

using std::chrono::system_clock;

namespace asapo {

const std::string ServerDataBroker::kBrokerServiceName = "asapo-broker";
const std::string ServerDataBroker::kFileTransferServiceName = "asapo-file-transfer";

Error GetNoDataResponseFromJson(const std::string &json_string, ConsumerErrorData* data) {
    JsonStringParser parser(json_string);
    Error err;
    if ((err = parser.GetUInt64("id", &data->id)) || (err = parser.GetUInt64("id_max", &data->id_max))
        || (err = parser.GetString("next_substream", &data->next_substream))) {
        return err;
    }
    return nullptr;
}

Error GetPartialDataResponseFromJson(const std::string &json_string, PartialErrorData* data) {
    Error err;
    auto parser = JsonStringParser(json_string);
    uint64_t  id,size;
    if ((err = parser.GetUInt64("size", &size)) ||
        (err = parser.GetUInt64("_id", &id))) {
        return err;
    }
    data->id = id;
    data->expected_size = size;
    return nullptr;
}

Error ConsumerErrorFromPartialDataResponse(const std::string &response) {
    PartialErrorData data;
    auto parse_error = GetPartialDataResponseFromJson(response, &data);
    if (parse_error) {
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
    }
    auto err = ConsumerErrorTemplates::kPartialData.Generate();
    PartialErrorData* error_data = new PartialErrorData{data};
    err->SetCustomData(std::unique_ptr<CustomErrorData>{error_data});
    return err;
}

Error ConsumerErrorFromNoDataResponse(const std::string &response) {
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
        err->SetCustomData(std::unique_ptr<CustomErrorData>{error_data});
        return err;
    }
    return ConsumerErrorTemplates::kNoData.Generate();
}

Error ConsumerErrorFromHttpCode(const RequestOutput* response, const HttpCode &code) {
    switch (code) {
        case HttpCode::OK:return nullptr;
        case HttpCode::PartialContent:return ConsumerErrorFromPartialDataResponse(response->to_string());
        case HttpCode::BadRequest:return ConsumerErrorTemplates::kWrongInput.Generate(response->to_string());
        case HttpCode::Unauthorized:return ConsumerErrorTemplates::kWrongInput.Generate(response->to_string());
        case HttpCode::InternalServerError:return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response->to_string());
        case HttpCode::NotFound:return ConsumerErrorTemplates::kUnavailableService.Generate(response->to_string());
        case HttpCode::Conflict:return ConsumerErrorFromNoDataResponse(response->to_string());
        default:return ConsumerErrorTemplates::kInterruptedTransaction.Generate(response->to_string());
    }
}
Error ConsumerErrorFromServerError(const Error &server_err) {
    if (server_err == HttpErrorTemplates::kTransferError) {
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(
            "error processing request: " + server_err->Explain());
    } else {
        return ConsumerErrorTemplates::kUnavailableService.Generate(
            "error processing request: " + server_err->Explain());
    }
}

Error ProcessRequestResponce(const Error &server_err, const RequestOutput* response, const HttpCode &code) {
    if (server_err != nullptr) {
        return ConsumerErrorFromServerError(server_err);
    }
    return ConsumerErrorFromHttpCode(response, code);
}

ServerDataBroker::ServerDataBroker(std::string server_uri,
                                   std::string source_path,
                                   bool has_filesystem,
                                   SourceCredentials source) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    endpoint_{std::move(server_uri)}, source_path_{std::move(source_path)}, has_filesystem_{has_filesystem},
    source_credentials_(std::move(source)) {

    // net_client__ will be lazy initialized

    if (source_credentials_.stream.empty()) {
        source_credentials_.stream = SourceCredentials::kDefaultStream;
    }

}

void ServerDataBroker::SetTimeout(uint64_t timeout_ms) {
    timeout_ms_ = timeout_ms;
}

void ServerDataBroker::ForceNoRdma() {
    should_try_rdma_first_ = false;
}

NetworkConnectionType ServerDataBroker::CurrentConnectionType() const {
    return current_connection_type_;
}

std::string ServerDataBroker::RequestWithToken(std::string uri) {
    return std::move(uri) + "?token=" + source_credentials_.user_token;
}

Error ServerDataBroker::ProcessPostRequest(const RequestInfo &request, RequestOutput* response, HttpCode* code) {
    Error err;
    switch (request.output_mode) {
        case OutputDataMode::string:
            response->string_output =
                httpclient__->Post(RequestWithToken(request.host + request.api) + request.extra_params,
                                   request.cookie,
                                   request.body,
                                   code,
                                   &err);
            break;
        case OutputDataMode::array:
            err =
                httpclient__->Post(RequestWithToken(request.host + request.api) + request.extra_params, request.cookie,
                                   request.body, &response->data_output, response->data_output_size, code);
            break;
        default:break;
    }
    return err;
}

Error ServerDataBroker::ProcessGetRequest(const RequestInfo &request, RequestOutput* response, HttpCode* code) {
    Error err;
    response->string_output =
        httpclient__->Get(RequestWithToken(request.host + request.api) + request.extra_params, code, &err);
    return err;
}

Error ServerDataBroker::ProcessRequest(RequestOutput* response, const RequestInfo &request, std::string* service_uri) {
    Error err;
    HttpCode code;
    if (request.post) {
        err = ProcessPostRequest(request, response, &code);
    } else {
        err = ProcessGetRequest(request, response, &code);
    }
    if (err && service_uri) {
        service_uri->clear();
    }
    return ProcessRequestResponce(err, response, code);
}

Error ServerDataBroker::DiscoverService(const std::string &service_name, std::string* uri_to_set) {
    if (!uri_to_set->empty()) {
        return nullptr;
    }
    RequestInfo ri;
    ri.host = endpoint_;
    ri.api = "/asapo-discovery/" + service_name;
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

bool ServerDataBroker::SwitchToGetByIdIfPartialData(Error* err,
                                                    const std::string &response,
                                                    std::string* group_id,
                                                    std::string* redirect_uri) {
    if (*err == ConsumerErrorTemplates::kPartialData) {
        auto error_data = static_cast<const PartialErrorData*>((*err)->GetCustomData());
        if (error_data == nullptr) {
            *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
            return false;
        }
        *redirect_uri = std::to_string(error_data->id);
        *group_id = "0";
        return true;
    }
    return false;
}

bool ServerDataBroker::SwitchToGetByIdIfNoData(Error* err, const std::string &response,std::string* group_id, std::string* redirect_uri) {
    if (*err == ConsumerErrorTemplates::kNoData) {
        auto error_data = static_cast<const ConsumerErrorData*>((*err)->GetCustomData());
        if (error_data == nullptr) {
            *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response - " + response);
            return false;
        }
        *redirect_uri = std::to_string(error_data->id);
        *group_id = "0";
        return true;
    }
    return false;
}

RequestInfo ServerDataBroker::PrepareRequestInfo(std::string api_url, bool dataset, uint64_t min_size) {
    RequestInfo ri;
    ri.host = current_broker_uri_;
    ri.api = std::move(api_url);
    if (dataset) {
        ri.extra_params = "&dataset=true";
        ri.extra_params += "&minsize="+std::to_string(min_size);
    }
    return ri;
}

Error ServerDataBroker::GetRecordFromServer(std::string* response, std::string group_id, std::string substream,
                                            GetImageServerOperation op,
                                            bool dataset, uint64_t min_size) {
    interrupt_flag_= false;
    std::string request_suffix = OpToUriCmd(op);
    std::string request_group = OpToUriCmd(op);
    std::string request_api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream
        + "/" + std::move(substream);
    uint64_t elapsed_ms = 0;
    Error no_data_error;
    while (true) {
        if (interrupt_flag_) {
            return ConsumerErrorTemplates::kInterruptedTransaction.Generate("interrupted by user request");
        }

        auto start = system_clock::now();
        auto err = DiscoverService(kBrokerServiceName, &current_broker_uri_);
        if (err == nullptr) {
            auto ri = PrepareRequestInfo(request_api + "/" + group_id + "/" + request_suffix, dataset, min_size);
            if (request_suffix == "next" && resend_) {
                ri.extra_params = ri.extra_params + "&resend_nacks=true" + "&delay_sec=" +
                    std::to_string(delay_sec_) + "&resend_attempts=" + std::to_string(resend_attempts_);
            }
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
            auto save_error = SwitchToGetByIdIfNoData(&err, *response, &group_id, &request_suffix)
                || SwitchToGetByIdIfPartialData(&err, *response, &group_id, &request_suffix);
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

Error ServerDataBroker::GetLast(FileInfo* info, FileData* data) {
    return GetLast(info, kDefaultSubstream, data);
}

Error ServerDataBroker::GetLast(FileInfo* info, std::string substream, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetLast,
                              0,
                              "0",
                              std::move(substream),
                              info,
                              data);
}

std::string ServerDataBroker::OpToUriCmd(GetImageServerOperation op) {
    switch (op) {
        case GetImageServerOperation::GetNext:return "next";
        case GetImageServerOperation::GetLast:return "last";
        default:return "last";
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

Error ServerDataBroker::GetDataFromFile(FileInfo* info, FileData* data) {
    Error error;
    *data = io__->GetDataFromFile(info->FullName(source_path_), &info->size, &error);
    if (error) {
        return ConsumerErrorTemplates::kLocalIOError.Generate(error->Explain());
    }
    return nullptr;
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

    if (has_filesystem_) {
        return GetDataFromFile(info, data);
    }

    return GetDataFromFileTransferService(info, data, false);
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

Error ServerDataBroker::CreateNetClientAndTryToGetFile(const FileInfo* info, FileData* data) {
    const std::lock_guard<std::mutex> lock(net_client_mutex__);
    if (net_client__) {
        return nullptr;
    }

    if (should_try_rdma_first_) { // This will check if a rdma connection can be made and will return early if so
        auto fabricClient = std::unique_ptr<NetClient>(new FabricConsumerClient());

        Error error = fabricClient->GetData(info, data);

        // Check if the error comes from the receiver data server (so a connection was made)
        if (!error || error == RdsResponseErrorTemplates::kNetErrorNoData) {
            net_client__.swap(fabricClient);
            current_connection_type_ = NetworkConnectionType::kFabric;
            return error; // Successfully received data and is now using a fabric client
        }

        // An error occurred!

        if (std::getenv("ASAPO_PRINT_FALLBACK_REASON")) {
            std::cout << "Fallback to TCP because error: " << error << std::endl;
        }

        // Retry with TCP
        should_try_rdma_first_ = false;
    }

    // Create regular tcp client
    net_client__.reset(new TcpClient());
    current_connection_type_ = NetworkConnectionType::kAsapoTcp;

    return net_client__->GetData(info, data);
}

Error ServerDataBroker::TryGetDataFromBuffer(const FileInfo* info, FileData* data) {
    if (!net_client__) {
        return CreateNetClientAndTryToGetFile(info, data);
    }

    return net_client__->GetData(info, data);
}

std::string ServerDataBroker::GenerateNewGroupId(Error* err) {
    RequestInfo ri;
    ri.api = "/creategroup";
    ri.post = true;
    return BrokerRequestWithTimeout(ri, err);
}

Error ServerDataBroker::ServiceRequestWithTimeout(const std::string &service_name,
                                                  std::string* service_uri,
                                                  RequestInfo request,
                                                  RequestOutput* response) {
    interrupt_flag_= false;
    uint64_t elapsed_ms = 0;
    Error err;
    while (elapsed_ms <= timeout_ms_) {
        if (interrupt_flag_) {
            err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("interrupted by user request");
            break;
        }
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

Error ServerDataBroker::FtsSizeRequestWithTimeout(FileInfo* info) {
    RequestInfo ri = CreateFileTransferRequest(info);
    ri.extra_params = "&sizeonly=true";
    ri.output_mode = OutputDataMode::string;
    RequestOutput response;
    auto err = ServiceRequestWithTimeout(kFileTransferServiceName, &current_fts_uri_, ri, &response);
    if (err) {
        return err;
    }

    auto parser = JsonStringParser(std::move(response.string_output));
    err = parser.GetUInt64("file_size", &(info->size));
    return err;
}

Error ServerDataBroker::FtsRequestWithTimeout(FileInfo* info, FileData* data) {
    RequestInfo ri = CreateFileTransferRequest(info);
    RequestOutput response;
    response.data_output_size = info->size;
    auto err = ServiceRequestWithTimeout(kFileTransferServiceName, &current_fts_uri_, ri, &response);
    if (err) {
        return err;
    }
    *data = std::move(response.data_output);
    return nullptr;
}

RequestInfo ServerDataBroker::CreateFileTransferRequest(const FileInfo* info) const {
    RequestInfo ri;
    ri.api = "/transfer";
    ri.post = true;
    ri.body = "{\"Folder\":\"" + source_path_ + "\",\"FileName\":\"" + info->name + "\"}";
    ri.cookie = "Authorization=Bearer " + folder_token_;
    ri.output_mode = OutputDataMode::array;
    return ri;
}

std::string ServerDataBroker::BrokerRequestWithTimeout(RequestInfo request, Error* err) {
    RequestOutput response;
    *err = ServiceRequestWithTimeout(kBrokerServiceName, &current_broker_uri_, request, &response);
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
Error ServerDataBroker::GetById(uint64_t id, FileInfo* info, FileData* data) {
    if (id == 0) {
        return ConsumerErrorTemplates::kWrongInput.Generate("id should be positive");
    }

    return GetById(id, info, kDefaultSubstream, data);
}

Error ServerDataBroker::GetById(uint64_t id, FileInfo* info, std::string substream, FileData* data) {
    return GetImageFromServer(GetImageServerOperation::GetID, id, "0", substream, info, data);
}

Error ServerDataBroker::GetRecordFromServerById(uint64_t id, std::string* response, std::string group_id,
                                                std::string substream,
                                                bool dataset, uint64_t min_size) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
        +"/" + std::move(substream) +
        "/" + std::move(
        group_id) + "/" + std::to_string(id);
    if (dataset) {
        ri.extra_params += "&dataset=true";
        ri.extra_params += "&minsize="+std::to_string(min_size);
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

DataSet DecodeDatasetFromResponse(std::string response, Error* err) {
    DataSet res;
    if (!res.SetFromJson(std::move(response))) {
        *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response:" + response);
        return {0,0,FileInfos{}};
    } else {
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

    auto dataset = DecodeDatasetFromResponse("{\"_id\":0,\"size\":0, \"images\":" + response + "}", err);
    return dataset.content;
}

FileInfos ServerDataBroker::QueryImages(std::string query, Error* err) {
    return QueryImages(std::move(query), kDefaultSubstream, err);
}

DataSet ServerDataBroker::GetNextDataset(std::string group_id, uint64_t min_size, Error* err) {
    return GetNextDataset(std::move(group_id), kDefaultSubstream, min_size, err);
}

DataSet ServerDataBroker::GetNextDataset(std::string group_id, std::string substream, uint64_t min_size, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetNext, 0, std::move(group_id), std::move(substream),min_size, err);
}

DataSet ServerDataBroker::GetLastDataset(std::string substream, uint64_t min_size, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetLast, 0, "0", std::move(substream),min_size, err);
}

DataSet ServerDataBroker::GetLastDataset(uint64_t min_size, Error* err) {
    return GetLastDataset(kDefaultSubstream, min_size, err);
}

DataSet ServerDataBroker::GetDatasetFromServer(GetImageServerOperation op,
                                               uint64_t id,
                                               std::string group_id, std::string substream,
                                               uint64_t min_size,
                                               Error* err) {
    FileInfos infos;
    std::string response;
    if (op == GetImageServerOperation::GetID) {
        *err = GetRecordFromServerById(id, &response, std::move(group_id), std::move(substream), true, min_size);
    } else {
        *err = GetRecordFromServer(&response, std::move(group_id), std::move(substream), op, true, min_size);
    }
    if (*err != nullptr && *err!=ConsumerErrorTemplates::kPartialData) {
        return {0, 0,FileInfos{}};
    }
    return DecodeDatasetFromResponse(response, err);
}

DataSet ServerDataBroker::GetDatasetById(uint64_t id, uint64_t min_size, Error* err) {
    return GetDatasetById(id, kDefaultSubstream, min_size, err);
}

DataSet ServerDataBroker::GetDatasetById(uint64_t id, std::string substream, uint64_t min_size, Error* err) {
    return GetDatasetFromServer(GetImageServerOperation::GetID, id, "0", std::move(substream), min_size, err);
}

StreamInfos ParseSubstreamsFromResponse(std::string response, Error* err) {
    auto parser = JsonStringParser(std::move(response));
    std::vector<std::string> substreams_endcoded;
    StreamInfos substreams;
    Error parse_err;
    *err = parser.GetArrayRawStrings("substreams", &substreams_endcoded);
    if (*err) {
        return StreamInfos{};
    }
    for (auto substream_encoded : substreams_endcoded) {
        StreamInfo si;
        auto ok = si.SetFromJson(substream_encoded, false);
        if (!ok) {
            *err = TextError("cannot parse " + substream_encoded);
            return StreamInfos{};
        }
        substreams.emplace_back(si);
    }
    return substreams;
}

StreamInfos ServerDataBroker::GetSubstreamList(std::string from, Error* err) {

    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream + "/0/substreams";
    ri.post = false;
    if (!from.empty()) {
        ri.extra_params = "&from=" + from;
    }

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return StreamInfos{};
    }

    return ParseSubstreamsFromResponse(std::move(response), err);
}

Error ServerDataBroker::UpdateFolderTokenIfNeeded(bool ignore_existing) {
    if (!folder_token_.empty() && !ignore_existing) {
        return nullptr;
    }
    folder_token_.clear();

    RequestOutput output;
    RequestInfo ri = CreateFolderTokenRequest();
    auto err = ProcessRequest(&output, ri, nullptr);
    if (err) {
        return err;
    }
    folder_token_ = std::move(output.string_output);
    return nullptr;
}

RequestInfo ServerDataBroker::CreateFolderTokenRequest() const {
    RequestInfo ri;
    ri.host = endpoint_;
    ri.api = "/asapo-authorizer/folder";
    ri.post = true;
    ri.body =
        "{\"Folder\":\"" + source_path_ + "\",\"BeamtimeId\":\"" + source_credentials_.beamtime_id + "\",\"Token\":\""
            +
                source_credentials_.user_token + "\"}";
    return ri;
}

Error ServerDataBroker::GetDataFromFileTransferService(FileInfo* info, FileData* data,
                                                       bool retry_with_new_token) {
    auto err = UpdateFolderTokenIfNeeded(retry_with_new_token);
    if (err) {
        return err;
    }

    if (info->size == 0) {
        err = FtsSizeRequestWithTimeout(info);
        if (err == ConsumerErrorTemplates::kWrongInput
            && !retry_with_new_token) { // token expired? Refresh token and try again.
            return GetDataFromFileTransferService(info, data, true);
        }
        if (err) {
            return err;
        }
    }

    err = FtsRequestWithTimeout(info, data);
    if (err == ConsumerErrorTemplates::kWrongInput
        && !retry_with_new_token) { // token expired? Refresh token and try again.
        return GetDataFromFileTransferService(info, data, true);
    }
    return err;
}

Error ServerDataBroker::Acknowledge(std::string group_id, uint64_t id, std::string substream) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
        +"/" + std::move(substream) +
        "/" + std::move(group_id) + "/" + std::to_string(id);
    ri.post = true;
    ri.body = "{\"Op\":\"ackimage\"}";

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

IdList ServerDataBroker::GetUnacknowledgedTupleIds(std::string group_id,
                                                   std::string substream,
                                                   uint64_t from_id,
                                                   uint64_t to_id,
                                                   Error* error) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
        +"/" + std::move(substream) +
        "/" + std::move(group_id) + "/nacks";
    ri.extra_params = "&from=" + std::to_string(from_id) + "&to=" + std::to_string(to_id);

    auto json_string = BrokerRequestWithTimeout(ri, error);
    if (*error) {
        return IdList{};
    }

    IdList list;
    JsonStringParser parser(json_string);
    if ((*error = parser.GetArrayUInt64("unacknowledged", &list))) {
        return IdList{};
    }

    return list;
}

IdList ServerDataBroker::GetUnacknowledgedTupleIds(std::string group_id,
                                                   uint64_t from_id,
                                                   uint64_t to_id,
                                                   Error* error) {
    return GetUnacknowledgedTupleIds(std::move(group_id), kDefaultSubstream, from_id, to_id, error);
}

uint64_t ServerDataBroker::GetLastAcknowledgedTulpeId(std::string group_id, std::string substream, Error* error) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
        +"/" + std::move(substream) +
        "/" + std::move(group_id) + "/lastack";

    auto json_string = BrokerRequestWithTimeout(ri, error);
    if (*error) {
        return 0;
    }

    uint64_t id;
    JsonStringParser parser(json_string);
    if ((*error = parser.GetUInt64("lastAckId", &id))) {
        return 0;
    }

    if (id == 0) {
        *error = ConsumerErrorTemplates::kNoData.Generate();
    }
    return id;
}

uint64_t ServerDataBroker::GetLastAcknowledgedTulpeId(std::string group_id, Error* error) {
    return GetLastAcknowledgedTulpeId(std::move(group_id), kDefaultSubstream, error);
}

void ServerDataBroker::SetResendNacs(bool resend, uint64_t delay_sec, uint64_t resend_attempts) {
    resend_ = resend;
    delay_sec_ = delay_sec;
    resend_attempts_ = resend_attempts;
}

Error ServerDataBroker::NegativeAcknowledge(std::string group_id,
                                            uint64_t id,
                                            uint64_t delay_sec,
                                            std::string substream) {
    RequestInfo ri;
    ri.api = "/database/" + source_credentials_.beamtime_id + "/" + source_credentials_.stream +
        +"/" + std::move(substream) +
        "/" + std::move(group_id) + "/" + std::to_string(id);
    ri.post = true;
    ri.body = R"({"Op":"negackimage","Params":{"DelaySec":)" + std::to_string(delay_sec) + "}}";

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}
void ServerDataBroker::InterruptCurrentOperation() {
    interrupt_flag_= true;
}

}
