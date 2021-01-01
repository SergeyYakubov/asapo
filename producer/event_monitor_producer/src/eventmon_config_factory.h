#ifndef ASAPO_EventMon_CONFIG_FACTORY__H
#define ASAPO_EventMon_CONFIG_FACTORY__H

#include "asapo/io/io.h"
#include "asapo/common/error.h"

namespace asapo {

class EventMonConfigFactory {
  public:
    EventMonConfigFactory();
    Error SetConfigFromFile(std::string file_name);
  public:
    std::unique_ptr<IO> io__;
  private:
    Error ParseConfigFile(std::string file_name);
    Error CheckMode();
    Error CheckLogLevel();
    Error CheckDatasets();
    Error CheckNThreads();
    Error CheckConfig();
};

}


#endif //ASAPO_EventMon_CONFIG_FACTORY__H
