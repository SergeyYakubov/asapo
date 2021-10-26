#include "request_handler_secondary_authorization.h"

#include "request_handler_authorize.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

#include "asapo/json_parser/json_parser.h"
#include "asapo/common/internal/version.h"

using std::chrono::system_clock;

namespace asapo {
/*
Error RequestHandlerAuthorize::CheckVersion(const std::string& version_from_client) const {
    int verClient = VersionToNumber(version_from_client);
    int verService = VersionToNumber(GetReceiverApiVersion());
    if (verClient > verService) {
        auto err_string = "client version: " + version_from_client + ", server version: " + GetReceiverApiVersion();
        return asapo::ReceiverErrorTemplates::kUnsupportedClient.Generate(err_string);
    }
    return nullptr;
}

Error RequestHandlerAuthorize::ProcessAuthorizationRequest(Request* request) const {
    if (!cached_source_credentials_.empty()) {
        Error auth_error = asapo::ReceiverErrorTemplates::kAuthorizationFailure.Generate();
        auth_error->Append("already authorized");
        return auth_error;
    }

    auto err = CheckVersion(request->GetApiVersion());
    if (err) {
        return err;
    }
    err = auth_client__->Authorize(request, request->GetMetaData().c_str(),&cached_auth_);
    if (err) {
        return err;
    }

    cached_source_credentials_ = request->GetMetaData();
    last_updated_ = system_clock::now();
    return nullptr;
}

Error RequestHandlerAuthorize::ProcessReAuthorization(Request* request) const {
    std::string old_beamtimeId = cached_auth_.beamtime_id;
    auto err = auth_client__->Authorize(request, cached_source_credentials_.c_str(),&cached_auth_);
    if (err == asapo::ReceiverErrorTemplates::kAuthorizationFailure || (
        err == nullptr && old_beamtimeId != cached_auth_.beamtime_id)) {
        return asapo::ReceiverErrorTemplates::kReAuthorizationFailure.Generate();
    }
    if (err==nullptr) {
        last_updated_ = system_clock::now();
    }
    return err;
}

bool RequestHandlerAuthorize::NeedReauthorize() const {
    uint64_t elapsed_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
        (system_clock::now() - last_updated_).count();
    return elapsed_ms >= GetReceiverConfig()->authorization_interval_ms;
}

void RequestHandlerAuthorize::SetRequestFields(Request* request) const {
    request->SetBeamtimeId(cached_auth_.beamtime_id);
    request->SetBeamline(cached_auth_.beamline);
    request->SetDataSource(cached_auth_.data_source);
    request->SetOfflinePath(cached_auth_.offline_path);
    request->SetOnlinePath(cached_auth_.online_path);
    request->SetSourceType(cached_auth_.source_type);
}


Error RequestHandlerAuthorize::ProcessOtherRequest(Request* request) const {
    if (cached_source_credentials_.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate();
    }

    if (NeedReauthorize()) {
        auto err = ProcessReAuthorization(request);
        if (err) {
            return err;
        }
    }
    SetRequestFields(request);
    return nullptr;
}

Error RequestHandlerAuthorize::ProcessRequest(Request* request) const {
    if (request->GetOpCode() == kOpcodeAuthorize) {
        return ProcessAuthorizationRequest(request);
    } else {
        return ProcessOtherRequest(request);
    }
}

RequestHandlerAuthorize::RequestHandlerAuthorize() : log__{GetDefaultReceiverLogger()},
                                                     auth_client__{new AuthorizationClient()} {
}

StatisticEntity RequestHandlerAuthorize::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}
*/

RequestHandlerSecondaryAuthorization::RequestHandlerSecondaryAuthorization(AuthorizationData *authorization_cache)
    : RequestHandlerAuthorize(authorization_cache) {

}

Error RequestHandlerSecondaryAuthorization::ProcessRequest(asapo::Request *request) const {
    (void) request;
    return nullptr;
}

}

