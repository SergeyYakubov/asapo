#include "receiver_config.h"
#include "receiver_config_factory.h"
#include "io/io_factory.h"

namespace hidra2 {

ReceiverConfig config;

ReceiverConfigFactory::ReceiverConfigFactory() : io__{GenerateDefaultIO()} {

}

Error ReceiverConfigFactory::SetConfigFromFile(std::string file_name) {
    config.influxdb_uri = "localhost";
    return {};
}

const ReceiverConfig*  GetReceiverConfig() {
    return &config;
}


}