#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "mock_receiver_config.h"
#include "../src/receiver_config_factory.h"

#include <unittests/MockIO.h>

using testing::_;

namespace hidra2 {

Error SetReceiverConfig (const ReceiverConfig& config) {
    MockIO mock_io;
    ReceiverConfigFactory config_factory;
    config_factory.io__ = std::unique_ptr<IO> {&mock_io};

    auto config_string = std::string("{\"MonitorDbAddress\":") + "\"" + config.monitor_db_uri + "\"";
    config_string += "," + std::string("\"MonitorDbName\":") + "\"" + config.monitor_db_name + "\"";
    config_string += "," + std::string("\"ListenPort\":") + std::to_string(config.listen_port);
    config_string += "," + std::string("\"WriteToDisk\":") + (config.write_to_disk ? "true" : "false");
    config_string += "}";

    EXPECT_CALL(mock_io, ReadFileToString_t("fname", _)).WillOnce(
        testing::Return(config_string)
    );

    auto err = config_factory.SetConfigFromFile("fname");

    config_factory.io__.release();

    return err;
}

}


