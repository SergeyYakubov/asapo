#ifndef HIDRA2_FOLDERDATABROKER_H
#define HIDRA2_FOLDERDATABROKER_H

#include "worker/data_broker.h"

#include <string>

#include "system_wrappers/io.h"


namespace hidra2 {

class FolderDataBroker final : public hidra2::DataBroker {
  public:
    explicit FolderDataBroker(const std::string& source_name);
    WorkerErrorCode Connect() override;
    std::unique_ptr<hidra2::IO> io__; // modified in testings to mock system calls,otherwise do not touch

  private:
    std::string base_path_;
    std::vector<FileInfo>  filelist_;
};

}

#endif //HIDRA2_FOLDERDATABROKER_H

