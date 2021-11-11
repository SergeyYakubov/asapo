#include "request_handler_secondary_authorization.h"

#include "request_handler_authorize.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

#include "asapo/json_parser/json_parser.h"
#include "asapo/common/internal/version.h"

using std::chrono::system_clock;

namespace asapo {

Error RequestHandlerSecondaryAuthorization::ProcessReAuthorization(const Request* request) const {
    std::string old_beamtimeId = authorization_cache_->beamtime_id;
    auto err = auth_client__->Authorize(request, authorization_cache_);
    if (err == asapo::ReceiverErrorTemplates::kAuthorizationFailure || (
                err == nullptr && old_beamtimeId != authorization_cache_->beamtime_id)) {
        InvalidateAuthCache();
        return asapo::ReceiverErrorTemplates::kReAuthorizationFailure.Generate();
    }
    return err;
}

bool RequestHandlerSecondaryAuthorization::NeedReauthorize() const {
    uint64_t elapsed_ms = (uint64_t) std::chrono::duration_cast<std::chrono::milliseconds>
                          (system_clock::now() - authorization_cache_->last_update).count();
    return elapsed_ms >= GetReceiverConfig()->authorization_interval_ms;
}

void RequestHandlerSecondaryAuthorization::SetRequestFields(Request* request) const {
    request->SetBeamtimeId(authorization_cache_->beamtime_id);
    request->SetBeamline(authorization_cache_->beamline);
    request->SetDataSource(authorization_cache_->data_source);
    request->SetOfflinePath(authorization_cache_->offline_path);
    request->SetOnlinePath(authorization_cache_->online_path);
    request->SetSourceType(authorization_cache_->source_type);
}

RequestHandlerSecondaryAuthorization::RequestHandlerSecondaryAuthorization(AuthorizationData* authorization_cache)
    : RequestHandlerAuthorize(authorization_cache) {

}

Error RequestHandlerSecondaryAuthorization::CheckRequest(const Request* request) const {
    auto err = CheckVersion(request);
    if (err != nullptr) {
        return err;
    }

    if (authorization_cache_->source_credentials.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate("not authorized");
    }
    return nullptr;
}

Error RequestHandlerSecondaryAuthorization::ProcessRequest(asapo::Request* request) const {
    auto err = CheckRequest(request);
    if (err != nullptr) {
        return err;
    }

    err = ReauthorizeIfNeeded(request);
    if (err != nullptr) {
        return err;
    }

    SetRequestFields(request);
    return nullptr;
}

Error RequestHandlerSecondaryAuthorization::ReauthorizeIfNeeded(const Request* request) const {
    if (!NeedReauthorize()) {
        return nullptr;
    }
    return ProcessReAuthorization(request);
}

void RequestHandlerSecondaryAuthorization::InvalidateAuthCache() const {
    authorization_cache_->source_credentials = "";
}

}
