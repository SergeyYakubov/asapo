#include "consumer_impl.h"
#include "consumer_impl.h"

#include <chrono>

#include "asapo/json_parser/json_parser.h"
#include "asapo/io/io_factory.h"
#include "asapo/http_client/http_error.h"
#include "tcp_consumer_client.h"

#include "asapo/asapo_consumer.h"
#include "fabric_consumer_client.h"
#include "rds_response_error.h"

#include "asapo/common/version.h"

using std::chrono::system_clock;

namespace asapo {

const std::string ConsumerImpl::kBrokerServiceName = "asapo-broker";
const std::string ConsumerImpl::kFileTransferServiceName = "asapo-file-transfer";

Error GetNoDataResponseFromJson(const std::string &json_string, ConsumerErrorData* data) {
    JsonStringParser parser(json_string);
    Error err;
    if ((err = parser.GetUInt64("id", &data->id)) || (err = parser.GetUInt64("id_max", &data->id_max))
        || (err = parser.GetString("next_stream", &data->next_stream))) {
        return err;
    }
    return nullptr;
}

Error GetPartialDataResponseFromJson(const std::string &json_string, PartialErrorData* data) {
    Error err;
    auto parser = JsonStringParser(json_string);
    uint64_t id, size;
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
            err = data.next_stream.empty() ? ConsumerErrorTemplates::kEndOfStream.Generate() :
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
        case HttpCode::UnsupportedMediaType:return ConsumerErrorTemplates::kUnsupportedClient.Generate(response->to_string());
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

ConsumerImpl::ConsumerImpl(std::string server_uri,
                           std::string source_path,
                           bool has_filesystem,
                           SourceCredentials source) :
    io__{GenerateDefaultIO()}, httpclient__{DefaultHttpClient()},
    endpoint_{std::move(server_uri)}, source_path_{std::move(source_path)}, has_filesystem_{has_filesystem},
    source_credentials_(std::move(source)) {

    // net_client__ will be lazy initialized

    if (source_credentials_.data_source.empty()) {
        source_credentials_.data_source = SourceCredentials::kDefaultStream;
    }

}

void ConsumerImpl::SetTimeout(uint64_t timeout_ms) {
    timeout_ms_ = timeout_ms;
}

void ConsumerImpl::ForceNoRdma() {
    should_try_rdma_first_ = false;
}

NetworkConnectionType ConsumerImpl::CurrentConnectionType() const {
    return current_connection_type_;
}

std::string ConsumerImpl::RequestWithToken(std::string uri) {
    return std::move(uri) + "?token=" + source_credentials_.user_token;
}

Error ConsumerImpl::ProcessPostRequest(const RequestInfo &request, RequestOutput* response, HttpCode* code) {
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

Error ConsumerImpl::ProcessGetRequest(const RequestInfo &request, RequestOutput* response, HttpCode* code) {
    Error err;
    response->string_output =
        httpclient__->Get(RequestWithToken(request.host + request.api) + request.extra_params, code, &err);
    return err;
}

Error ConsumerImpl::ProcessRequest(RequestOutput* response, const RequestInfo &request, std::string* service_uri) {
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

RequestInfo ConsumerImpl::GetDiscoveryRequest(const std::string &service_name) const {
    RequestInfo ri;
    ri.host = endpoint_;
    ri.api = "/asapo-discovery/" + kConsumerProtocol.GetDiscoveryVersion() + "/" + service_name;
    ri.extra_params = "&protocol=" + kConsumerProtocol.GetVersion();
    return ri;
}

Error ConsumerImpl::ProcessDiscoverServiceResult(Error err, std::string* uri_to_set) {
    if (err != nullptr || uri_to_set->empty()) {
        uri_to_set->clear();
        if (err == ConsumerErrorTemplates::kUnsupportedClient) {
            return err;
        }
        return ConsumerErrorTemplates::kUnavailableService.Generate(" on " + endpoint_
                                                                        + (err != nullptr ? ": " + err->Explain()
                                                                                          : ""));
    }
    return nullptr;
}

Error ConsumerImpl::DiscoverService(const std::string &service_name, std::string* uri_to_set) {
    if (!uri_to_set->empty()) {
        return nullptr;
    }
    auto ri = GetDiscoveryRequest(service_name);
    RequestOutput output;
    auto err = ProcessRequest(&output, ri, nullptr);
    *uri_to_set = std::move(output.string_output);
    return ProcessDiscoverServiceResult(std::move(err), uri_to_set);
}

bool ConsumerImpl::SwitchToGetByIdIfPartialData(Error* err,
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

bool ConsumerImpl::SwitchToGetByIdIfNoData(Error* err,
                                           const std::string &response,
                                           std::string* group_id,
                                           std::string* redirect_uri) {
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

RequestInfo ConsumerImpl::PrepareRequestInfo(std::string api_url, bool dataset, uint64_t min_size) {
    RequestInfo ri;
    ri.host = current_broker_uri_;
    ri.api = std::move(api_url);
    if (dataset) {
        ri.extra_params = "&dataset=true";
        ri.extra_params += "&minsize=" + std::to_string(min_size);
    }
    return ri;
}

Error ConsumerImpl::GetRecordFromServer(std::string* response, std::string group_id, std::string stream,
                                        GetMessageServerOperation op,
                                        bool dataset, uint64_t min_size) {

    if (stream.empty()) {
        return ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
    }

    interrupt_flag_ = false;
    std::string request_suffix = OpToUriCmd(op);
    std::string request_group = OpToUriCmd(op);
    std::string
        request_api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source
        + "/" + std::move(stream);
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
                ri.extra_params = ri.extra_params + "&resend_nacks=true" + "&delay_ms=" +
                    std::to_string(delay_ms_) + "&resend_attempts=" + std::to_string(resend_attempts_);
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

Error ConsumerImpl::GetNext(std::string group_id, MessageMeta* info, MessageData* data, std::string stream) {
    return GetMessageFromServer(GetMessageServerOperation::GetNext,
                                0,
                                std::move(group_id),
                                std::move(stream),
                                info,
                                data);
}

Error ConsumerImpl::GetLast(MessageMeta* info, MessageData* data, std::string stream) {
    return GetMessageFromServer(GetMessageServerOperation::GetLast,
                                0,
                                "0",
                                std::move(stream),
                                info,
                                data);
}

std::string ConsumerImpl::OpToUriCmd(GetMessageServerOperation op) {
    switch (op) {
        case GetMessageServerOperation::GetNext:return "next";
        case GetMessageServerOperation::GetLast:return "last";
        default:return "last";
    }
}

Error ConsumerImpl::GetMessageFromServer(GetMessageServerOperation op, uint64_t id, std::string group_id,
                                         std::string stream,
                                         MessageMeta* info,
                                         MessageData* data) {
    if (info == nullptr) {
        return ConsumerErrorTemplates::kWrongInput.Generate();
    }

    Error err;
    std::string response;
    if (op == GetMessageServerOperation::GetID) {
        err = GetRecordFromServerById(id, &response, std::move(group_id), std::move(stream));
    } else {
        err = GetRecordFromServer(&response, std::move(group_id), std::move(stream), op);
    }
    if (err != nullptr) {
        return err;
    }

    if (!info->SetFromJson(response)) {
        return ConsumerErrorTemplates::kInterruptedTransaction.Generate(std::string("malformed response:") + response);
    }
    return GetDataIfNeeded(info, data);
}

Error ConsumerImpl::GetDataFromFile(MessageMeta* info, MessageData* data) {
    Error error;
    *data = io__->GetDataFromFile(info->FullName(source_path_), &info->size, &error);
    if (error) {
        return ConsumerErrorTemplates::kLocalIOError.Generate(error->Explain());
    }
    return nullptr;
}

Error ConsumerImpl::RetrieveData(MessageMeta* info, MessageData* data) {
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

Error ConsumerImpl::GetDataIfNeeded(MessageMeta* info, MessageData* data) {
    if (data == nullptr) {
        return nullptr;
    }

    return RetrieveData(info, data);

}

bool ConsumerImpl::DataCanBeInBuffer(const MessageMeta* info) {
    return info->buf_id > 0;
}

Error ConsumerImpl::CreateNetClientAndTryToGetFile(const MessageMeta* info, MessageData* data) {
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
    net_client__.reset(new TcpConsumerClient());
    current_connection_type_ = NetworkConnectionType::kAsapoTcp;

    return net_client__->GetData(info, data);
}

Error ConsumerImpl::TryGetDataFromBuffer(const MessageMeta* info, MessageData* data) {
    if (!net_client__) {
        return CreateNetClientAndTryToGetFile(info, data);
    }

    return net_client__->GetData(info, data);
}

std::string ConsumerImpl::GenerateNewGroupId(Error* err) {
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/creategroup";
    ri.post = true;
    return BrokerRequestWithTimeout(ri, err);
}

Error ConsumerImpl::ServiceRequestWithTimeout(const std::string &service_name,
                                              std::string* service_uri,
                                              RequestInfo request,
                                              RequestOutput* response) {
    interrupt_flag_ = false;
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

Error ConsumerImpl::FtsSizeRequestWithTimeout(MessageMeta* info) {
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

Error ConsumerImpl::FtsRequestWithTimeout(MessageMeta* info, MessageData* data) {
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

RequestInfo ConsumerImpl::CreateFileTransferRequest(const MessageMeta* info) const {
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetFileTransferServiceVersion() + "/transfer";
    ri.post = true;
    ri.body = "{\"Folder\":\"" + source_path_ + "\",\"FileName\":\"" + info->name + "\"}";
    ri.cookie = "Authorization=Bearer " + folder_token_;
    ri.output_mode = OutputDataMode::array;
    return ri;
}

std::string ConsumerImpl::BrokerRequestWithTimeout(RequestInfo request, Error* err) {
    RequestOutput response;
    *err = ServiceRequestWithTimeout(kBrokerServiceName, &current_broker_uri_, request, &response);
    return std::move(response.string_output);
}

Error ConsumerImpl::ResetLastReadMarker(std::string group_id, std::string stream) {
    return SetLastReadMarker(group_id, 0, stream);
}

Error ConsumerImpl::SetLastReadMarker(std::string group_id, uint64_t value, std::string stream) {
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source + "/"
        + std::move(stream) + "/" + std::move(group_id) + "/resetcounter";
    ri.extra_params = "&value=" + std::to_string(value);
    ri.post = true;

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

uint64_t ConsumerImpl::GetCurrentSize(std::string stream, Error* err) {
    auto ri = GetSizeRequestForSingleMessagesStream(stream);
    return GetCurrentCount(stream, ri, err);
}

Error ConsumerImpl::GetById(uint64_t id, MessageMeta* info, MessageData* data, std::string stream) {
    if (id == 0) {
        return ConsumerErrorTemplates::kWrongInput.Generate("id should be positive");
    }
    return GetMessageFromServer(GetMessageServerOperation::GetID, id, "0", stream, info, data);
}

Error ConsumerImpl::GetRecordFromServerById(uint64_t id, std::string* response, std::string group_id,
                                            std::string stream,
                                            bool dataset, uint64_t min_size) {
    if (stream.empty()) {
        return ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
    }

    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) +
        "/" + std::move(
        group_id) + "/" + std::to_string(id);
    if (dataset) {
        ri.extra_params += "&dataset=true";
        ri.extra_params += "&minsize=" + std::to_string(min_size);
    }

    Error err;
    *response = BrokerRequestWithTimeout(ri, &err);
    return err;
}

std::string ConsumerImpl::GetBeamtimeMeta(Error* err) {
    RequestInfo ri;
    ri.api =
        "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
            + source_credentials_.data_source + "/default/0/meta/0";

    return BrokerRequestWithTimeout(ri, err);
}

DataSet DecodeDatasetFromResponse(std::string response, Error* err) {
    DataSet res;
    if (!res.SetFromJson(std::move(response))) {
        *err = ConsumerErrorTemplates::kInterruptedTransaction.Generate("malformed response:" + response);
        return {0, 0, MessageMetas{}};
    } else {
        return res;
    }
}

MessageMetas ConsumerImpl::QueryMessages(std::string query, std::string stream, Error* err) {
    if (stream.empty()) {
        *err = ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
        return {};
    }

    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        "/" + std::move(stream) + "/0/querymessages";
    ri.post = true;
    ri.body = std::move(query);

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return MessageMetas{};
    }

    auto dataset = DecodeDatasetFromResponse("{\"_id\":0,\"size\":0, \"messages\":" + response + "}", err);
    return dataset.content;
}

DataSet ConsumerImpl::GetNextDataset(std::string group_id, uint64_t min_size, std::string stream, Error* err) {
    return GetDatasetFromServer(GetMessageServerOperation::GetNext,
                                0,
                                std::move(group_id),
                                std::move(stream),
                                min_size,
                                err);
}

DataSet ConsumerImpl::GetLastDataset(uint64_t min_size, std::string stream, Error* err) {
    return GetDatasetFromServer(GetMessageServerOperation::GetLast, 0, "0", std::move(stream), min_size, err);
}

DataSet ConsumerImpl::GetDatasetFromServer(GetMessageServerOperation op,
                                           uint64_t id,
                                           std::string group_id, std::string stream,
                                           uint64_t min_size,
                                           Error* err) {
    MessageMetas infos;
    std::string response;
    if (op == GetMessageServerOperation::GetID) {
        *err = GetRecordFromServerById(id, &response, std::move(group_id), std::move(stream), true, min_size);
    } else {
        *err = GetRecordFromServer(&response, std::move(group_id), std::move(stream), op, true, min_size);
    }
    if (*err != nullptr && *err != ConsumerErrorTemplates::kPartialData) {
        return {0, 0, MessageMetas{}};
    }
    return DecodeDatasetFromResponse(response, err);
}

DataSet ConsumerImpl::GetDatasetById(uint64_t id, uint64_t min_size, std::string stream, Error* err) {
    if (id == 0) {
        *err = ConsumerErrorTemplates::kWrongInput.Generate("id should be positive");
        return {};
    }
    return GetDatasetFromServer(GetMessageServerOperation::GetID, id, "0", std::move(stream), min_size, err);
}

StreamInfos ParseStreamsFromResponse(std::string response, Error* err) {
    auto parser = JsonStringParser(std::move(response));
    std::vector<std::string> streams_endcoded;
    StreamInfos streams;
    Error parse_err;
    *err = parser.GetArrayRawStrings("streams", &streams_endcoded);
    if (*err) {
        return StreamInfos{};
    }
    for (auto stream_encoded : streams_endcoded) {
        StreamInfo si;
        auto ok = si.SetFromJson(stream_encoded);
        if (!ok) {
            *err = TextError("cannot parse " + stream_encoded);
            return StreamInfos{};
        }
        streams.emplace_back(si);
    }
    return streams;
}

std::string filterToString(StreamFilter filter) {
    switch (filter) {
        case StreamFilter::kAllStreams:return "all";
        case StreamFilter::kFinishedStreams:return "finished";
        case StreamFilter::kUnfinishedStreams:return "unfinished";
    }
}

StreamInfos ConsumerImpl::GetStreamList(std::string from, StreamFilter filter, Error* err) {
    RequestInfo ri = GetStreamListRequest(from, filter);

    auto response = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return StreamInfos{};
    }
    return ParseStreamsFromResponse(std::move(response), err);
}

RequestInfo ConsumerImpl::GetStreamListRequest(const std::string &from, const StreamFilter &filter) const {
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source + "/0/streams";
    ri.post = false;
    if (!from.empty()) {
        ri.extra_params = "&from=" + from;
    }
    ri.extra_params += "&filter=" + filterToString(filter);
    return ri;
}

Error ConsumerImpl::UpdateFolderTokenIfNeeded(bool ignore_existing) {
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

RequestInfo ConsumerImpl::CreateFolderTokenRequest() const {
    RequestInfo ri;
    ri.host = endpoint_;
    ri.api = "/asapo-authorizer/"+kConsumerProtocol.GetAuthorizerVersion()+"/folder";
    ri.post = true;
    ri.body =
        "{\"Folder\":\"" + source_path_ + "\",\"BeamtimeId\":\"" + source_credentials_.beamtime_id + "\",\"Token\":\""
            +
                source_credentials_.user_token + "\"}";
    return ri;
}

Error ConsumerImpl::GetDataFromFileTransferService(MessageMeta* info, MessageData* data,
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

Error ConsumerImpl::Acknowledge(std::string group_id, uint64_t id, std::string stream) {
    if (stream.empty()) {
        return ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
    }
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) +
        "/" + std::move(group_id) + "/" + std::to_string(id);
    ri.post = true;
    ri.body = "{\"Op\":\"ackmessage\"}";

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}

IdList ConsumerImpl::GetUnacknowledgedMessages(std::string group_id,
                                               uint64_t from_id,
                                               uint64_t to_id,
                                               std::string stream,
                                               Error* error) {
    if (stream.empty()) {
        *error = ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
        return {};
    }
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) +
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

uint64_t ConsumerImpl::GetLastAcknowledgedMessage(std::string group_id, std::string stream, Error* error) {
    if (stream.empty()) {
        *error = ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
        return 0;
    }
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) +
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

void ConsumerImpl::SetResendNacs(bool resend, uint64_t delay_ms, uint64_t resend_attempts) {
    resend_ = resend;
    delay_ms_ = delay_ms;
    resend_attempts_ = resend_attempts;
}

Error ConsumerImpl::NegativeAcknowledge(std::string group_id,
                                        uint64_t id,
                                        uint64_t delay_ms,
                                        std::string stream) {
    if (stream.empty()) {
        return ConsumerErrorTemplates::kWrongInput.Generate("empty stream");
    }
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) +
        "/" + std::move(group_id) + "/" + std::to_string(id);
    ri.post = true;
    ri.body = R"({"Op":"negackmessage","Params":{"DelayMs":)" + std::to_string(delay_ms) + "}}";

    Error err;
    BrokerRequestWithTimeout(ri, &err);
    return err;
}
void ConsumerImpl::InterruptCurrentOperation() {
    interrupt_flag_ = true;
}

uint64_t ConsumerImpl::GetCurrentDatasetCount(std::string stream, bool include_incomplete, Error* err) {
    RequestInfo ri = GetSizeRequestForDatasetStream(stream, include_incomplete);
    return GetCurrentCount(stream, ri, err);
}

RequestInfo ConsumerImpl::GetSizeRequestForDatasetStream(std::string &stream, bool include_incomplete) const {
    RequestInfo ri = GetSizeRequestForSingleMessagesStream(stream);
    ri.extra_params = std::string("&incomplete=") + (include_incomplete ? "true" : "false");
    return ri;
}

uint64_t ConsumerImpl::GetCurrentCount(std::string stream, const RequestInfo &ri, Error* err) {
    auto responce = BrokerRequestWithTimeout(ri, err);
    if (*err) {
        return 0;
    }
    return ParseGetCurrentCountResponce(err, responce);
}

uint64_t ConsumerImpl::ParseGetCurrentCountResponce(Error* err, const std::string &responce) const {
    JsonStringParser parser(responce);
    uint64_t size;
    if ((*err = parser.GetUInt64("size", &size)) != nullptr) {
        return 0;
    }
    return size;
}

RequestInfo ConsumerImpl::GetSizeRequestForSingleMessagesStream(std::string &stream) const {
    RequestInfo ri;
    ri.api = "/" + kConsumerProtocol.GetBrokerVersion() + "/beamtime/" + source_credentials_.beamtime_id + "/"
        + source_credentials_.data_source +
        +"/" + std::move(stream) + "/size";
    return ri;
}

}
