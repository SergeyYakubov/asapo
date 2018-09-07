#ifndef ASAPO_SINGLE_FOLDER_MONITOR_H
#define ASAPO_SINGLE_FOLDER_MONITOR_H

#include <string>
#include <windows.h>
#include <winnt.h>


#include "watch_io.h"
#include "logger/logger.h"
#include "shared_event_list.h"
#include "win_event.h"

namespace asapo {

const uint64_t kBufLen  = 1000 * (sizeof(FILE_NOTIFY_INFORMATION) + FILENAME_MAX + 1);


class SingleFolderWatch {
  public:
    explicit SingleFolderWatch(std::string root_folder,std::string folder,SharedEventList* event_list);
    void Watch();
    std::unique_ptr<WatchIO> watch_io__;
    const AbstractLogger* log__;
 private:
  std::string root_folder_;
  std::string folder_;
  Error Init();
  HANDLE handle_;
  SharedEventList* event_list_;
  std::unique_ptr<char[]> buffer_;
  Error ProcessEvent(const WinEvent& event);
  void ProcessEvents(DWORD bytes_to_read);
};

}

#endif //ASAPO_SINGLE_FOLDER_MONITOR_H
