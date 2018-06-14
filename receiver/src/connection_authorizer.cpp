#include "connection_authorizer.h"

#include "receiver_logger.h"


namespace asapo {

Error ConnectionAuthorizer::Authorize(std::string beamtime_id,std::string uri) const noexcept {
    Error err;
    if (err) {
        log__->Error("error authorizing beamtime "+ beamtime_id +" on "+ uri + " - " + err->Explain());
    } else {
        log__->Debug("authorized beamtime "+ beamtime_id +" on "+ uri);
    }

    return nullptr;
}
ConnectionAuthorizer::ConnectionAuthorizer() : log__{GetDefaultReceiverLogger()}{

}

}