#include "receiver_config.h"
#include "receiver_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace hidra2 {

ReceiverConfig config;

ReceiverConfigFactory::ReceiverConfigFactory() : io__{GenerateDefaultIO()} {

}

Error ReceiverConfigFactory::SetConfigFromFile(std::string file_name) {
    JsonParser parser(file_name, &io__);
    return parser.GetString("uri",&config.influxdb_uri);
}

const ReceiverConfig*  GetReceiverConfig() {
    return &config;
}


}