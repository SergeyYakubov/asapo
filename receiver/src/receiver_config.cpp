#include "receiver_config.h"
#include "receiver_config_factory.h"
#include "io/io_factory.h"
#include "json_parser/json_parser.h"

namespace asapo {

ReceiverConfig config;

ReceiverConfigFactory::ReceiverConfigFactory() : io__{GenerateDefaultIO()} {

}

Error ReceiverConfigFactory::SetConfigFromFile(std::string file_name) {
    JsonFileParser parser(file_name, &io__);
    std::string log_level;
    Error err;
    (err = parser.GetString("MonitorDbAddress", &config.monitor_db_uri)) ||
    (err = parser.GetUInt64("ListenPort", &config.listen_port)) ||
    (err = parser.Embedded("DataServer").GetUInt64("ListenPort", &config.dataserver_listen_port)) ||
    (err = parser.GetBool("WriteToDisk", &config.write_to_disk)) ||
    (err = parser.GetBool("WriteToDb", &config.write_to_db)) ||
    (err = parser.GetString("BrokerDbAddress", &config.broker_db_uri)) ||
    (err = parser.GetString("Tag", &config.tag)) ||
    (err = parser.GetString("AuthorizationServer", &config.authorization_server)) ||
    (err = parser.GetUInt64("AuthorizationInterval", &config.authorization_interval_ms)) ||
    (err = parser.GetString("RootFolder", &config.root_folder)) ||
    (err = parser.GetString("MonitorDbName", &config.monitor_db_name));
    (err = parser.GetString("LogLevel", &log_level));
    if (err) {
        return err;
    }

    config.log_level = StringToLogLevel(log_level, &err);
    return err;

}

const ReceiverConfig*  GetReceiverConfig() {
    return &config;
}


}