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
    Error ResetCounter(std::string group_id) override;
    Error GetNext(FileInfo* info, std::string group_id, FileData* data) override;
    Error GetLast(FileInfo* info, std::string group_id, FileData* data) override;
    void SetTimeout(uint64_t timeout_ms) override {}; // to timeout in this case
    std::string GenerateNewGroupId(Error* err) override; // return "0" always and no error - no group ids for folder datra broker
    uint64_t GetNDataSets(Error* err) override;
    Error GetById(uint64_t id,FileInfo* info, FileData* data) override;
    std::unique_ptr<asapo::IO> io__; // modified in testings to mock system calls,otherwise do not touch
  private:
    std::string base_path_;
    bool is_connected_;
    int current_file_;
    FileInfos  filelist_;
    Error CanGetData(FileInfo* info, FileData* data, uint64_t nfile) const noexcept;
    Error GetFileByIndex(uint64_t nfile_to_get, FileInfo* info, FileData* data);
    std::mutex mutex_;
};

}

#endif //ASAPO_FOLDERDATABROKER_H

