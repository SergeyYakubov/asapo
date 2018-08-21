#ifndef ASAPO_FolderMon_CONFIG_FACTORY__H
#define ASAPO_FolderMon_CONFIG_FACTORY__H

#include "io/io.h"
#include "common/error.h"

namespace asapo {

class FolderMonConfigFactory {
  public:
    FolderMonConfigFactory();
    Error SetConfigFromFile(std::string file_name);
  public:
    std::unique_ptr<IO> io__;
  private:
    Error ParseConfigFile(std::string file_name);
    Error CheckMode();
    Error CheckLogLevel();
    Error CheckNThreads();
    Error CheckConfig();

};

}


#endif //ASAPO_FolderMon_CONFIG_FACTORY__H
