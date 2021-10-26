#include "request_handler_authorize.h"
#include "../receiver_config.h"
#include "../receiver_logger.h"
#include "../request.h"

#include "asapo/json_parser/json_parser.h"
#include "asapo/common/internal/version.h"

using std::chrono::system_clock;

namespace asapo {

Error RequestHandlerAuthorize::CheckVersion(const Request* request) const {
    auto version_from_client = request->GetApiVersion();
    int verClient = VersionToNumber(version_from_client);
    int verService = VersionToNumber(GetReceiverApiVersion());
    if (verClient > verService) {
        auto err_string = "client version: " + version_from_client + ", server version: " + GetReceiverApiVersion();
        return asapo::ReceiverErrorTemplates::kUnsupportedClient.Generate(err_string);
    }
    return nullptr;
}

RequestHandlerAuthorize::RequestHandlerAuthorize(AuthorizationData* authorization_cache) : log__{GetDefaultReceiverLogger()},
    auth_client__{new AuthorizationClient()},authorization_cache_{authorization_cache} {
}

StatisticEntity RequestHandlerAuthorize::GetStatisticEntity() const {
    return StatisticEntity::kNetwork;
}

void RequestHandlerAuthorize::SetRequestFields(Request* request) const {
    request->SetBeamtimeId(authorization_cache_->beamtime_id);
    request->SetBeamline(authorization_cache_->beamline);
    request->SetDataSource(authorization_cache_->data_source);
    request->SetOfflinePath(authorization_cache_->offline_path);
    request->SetOnlinePath(authorization_cache_->online_path);
    request->SetSourceType(authorization_cache_->source_type);
}


}
