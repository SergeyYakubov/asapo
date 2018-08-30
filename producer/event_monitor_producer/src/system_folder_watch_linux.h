#ifndef ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
#define ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H

#include <vector>
#include <string>
#include <map>
#include <unistd.h>

#include "common/error.h"
#include "preprocessor/definitions.h"
#include "asapo_producer.h"
#include "common.h"
#include "io/io.h"
#include "inotify_event.h"
#include "inotify_linux.h"


namespace asapo {


const uint64_t kBufLen  = 1000 * (sizeof(struct inotify_event) + FILENAME_MAX + 1);
const uint32_t kInotifyWatchFlags  = IN_CLOSE_WRITE |
                                     IN_MOVED_TO    |
                                     IN_MOVED_FROM  |
                                     IN_CREATE      |
                                     IN_DELETE_SELF |
                                     IN_EXCL_UNLINK |
                                     IN_DONT_FOLLOW |
                                     IN_ONLYDIR;



class SystemFolderWatch {
  public:
    VIRTUAL Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders);
    VIRTUAL FilesToSend GetFileList(Error* err);
    SystemFolderWatch();
    std::unique_ptr<IO> io__;
    std::unique_ptr<Inotify> inotify__;
  private:
    Error AddFolderAndSubfoldersToWatch(std::string folder);
    Error AddFolderToWatch(std::string folder);
    Error ProcessInotifyEvent(const InotifyEvent& event, FilesToSend* file_events);
    Error ProcessFileEvent(const InotifyEvent& event, FilesToSend* files);
    Error ProcessDirectoryEvent(const InotifyEvent& event);
    Error ProcessNewDirectoryInFolderEvent(const InotifyEvent& event);
    Error ProcessDeleteDirectoryInFolderEvent(const InotifyEvent& event);
    std::map<int, std::string>::iterator FindEventIterator(const InotifyEvent& event, Error* err);
    void RemoveFolderWithSubfoldersFromWatch(const std::string& path);
    std::map<int, std::string>::iterator RemoveFolderFromWatch(const std::map<int, std::string>::iterator& it);

  private:
    std::unique_ptr<char[]> buffer_;
    std::map<int, std::string> watched_folders_paths_;
    int watch_fd_ = -1;
    Error ReadInotifyEvents(int* bytes_read);
    Error ProcessInotifyEvents(int bytes_in_buffer, FilesToSend* events);
    Error FindEventPath(const InotifyEvent& event, std::string* folder, bool add_root);
    std::string root_folder_;
};

bool DirectoryEvent(const InotifyEvent& event);

}



#endif //ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
