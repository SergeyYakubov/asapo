#include "receiver_config.h"
#include "receiver_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace hidra2 {

ReceiverConfig config;

ReceiverConfigFactory::ReceiverConfigFactory() : io__{GenerateDefaultIO()} {

}

Error ReceiverConfigFactory::SetConfigFromFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    Error err;
    (err = parser.GetString("MonitorDbAddress", &config.monitor_db_uri)) ||
    (err = parser.GetUInt64("ListenPort", &config.listen_port)) ||
    (err = parser.GetBool("WriteToDisk", &config.write_to_disk)) ||
    (err = parser.GetString("MonitorDbName", &config.monitor_db_name));
    return err;
}

const ReceiverConfig*  GetReceiverConfig() {
    return &config;
}


}