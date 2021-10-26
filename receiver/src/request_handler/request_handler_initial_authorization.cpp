#include "request_handler_initial_authorization.h"

#include "../request.h"

namespace asapo {

RequestHandlerInitialAuthorization::RequestHandlerInitialAuthorization(AuthorizationData *authorization_cache)
    : RequestHandlerAuthorize(authorization_cache) {

}

Error RequestHandlerInitialAuthorization::ProcessRequest(asapo::Request *request) const {
    auto err = CheckVersion(request);
    if (err!=nullptr) {
        return err;
    }

    if (!authorization_cache_->source_credentials.empty()) {
        return ReceiverErrorTemplates::kAuthorizationFailure.Generate("already authorized");
    }

    authorization_cache_->source_credentials = request->GetMetaData();
    return auth_client__->Authorize(request,authorization_cache_);
}


}