#ifndef ASAPO_SINGLE_FOLDER_MONITOR_H
#define ASAPO_SINGLE_FOLDER_MONITOR_H

#include <string>

namespace asapo {

class SingleFolderMonitor {
  public:
    explicit SingleFolderMonitor(std::string root_folder,std::string folder);
    void Monitor();
 private:
  std::string root_folder_;
  std::string folder_;
};

}

#endif //ASAPO_SINGLE_FOLDER_MONITOR_H
