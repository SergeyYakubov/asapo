#ifndef ASAPO_FOLDERDATABROKER_H
#define ASAPO_FOLDERDATABROKER_H

#include "worker/data_broker.h"

#include <string>
#include <mutex>

#include "io/io.h"

namespace asapo {

class FolderDataBroker final : public asapo::DataBroker {
  public:
    explicit FolderDataBroker(const std::string& source_name);
    Error Connect() override;
    Error GetNext(FileInfo* info, FileData* data) override;
    void SetTimeout(uint64_t timeout_ms) override {}; // to timeout in this case

    std::unique_ptr<asapo::IO> io__; // modified in testings to mock system calls,otherwise do not touch
  private:
    std::string base_path_;
    bool is_connected_;
    int current_file_;
    FileInfos  filelist_;
    Error CanGetData(FileInfo* info, FileData* data, int nfile) const noexcept;
    std::mutex mutex_;
};

}

#endif //ASAPO_FOLDERDATABROKER_H

