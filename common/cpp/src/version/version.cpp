#include "asapo/common/internal/version.h"
#include "asapo/json_parser/json_parser.h"

namespace asapo {

Error ExtractVersionFromResponse(const std::string &response,
                                 const std::string &client,
                                 std::string* server_info,
                                 bool* supported) {
    JsonStringParser parser(response);
    std::string server_version, current_client_protocol, client_supported;
    Error err;
    if ((err = parser.GetString("softwareVersion", &server_version))
        || (err = parser.GetString("clientSupported", &client_supported))
        || (err = parser.Embedded("clientProtocol").GetString("versionInfo", &current_client_protocol))) {
        return err;
    }
    if (server_info) {
        *server_info =
            "Server version: " + server_version + ", " + client + " protocol on server: " + current_client_protocol;
    }
    if (supported) {
        *supported = client_supported == "yes";
    }
    return nullptr;
}

}