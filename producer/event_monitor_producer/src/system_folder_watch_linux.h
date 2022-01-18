#ifndef ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
#define ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H

#include <vector>
#include <string>
#include <map>
#include <unistd.h>

#include "asapo/common/error.h"
#include "asapo/preprocessor/definitions.h"
#include "asapo/asapo_producer.h"
#include "common.h"
#include "asapo/io/io.h"
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
    ASAPO_VIRTUAL Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders);
    ASAPO_VIRTUAL FilesToSend GetFileList(Error* err);
    SystemFolderWatch();
    std::unique_ptr<IO> io__;
    std::unique_ptr<Inotify> inotify__;
  private:
    Error AddFolderAndSubfoldersToWatch(std::string folder);
    Error AddFolderToWatch(std::string folder);
    Error ProcessInotifyEvent(const InotifyEvent& event, FilesToSend* file_events);
    Error ProcessFileEvent(const InotifyEvent& event, FilesToSend* files);
    Error ProcessDirectoryEvent(const InotifyEvent& event, FilesToSend* file_events);
    Error ProcessNewDirectoryInFolderEvent(const InotifyEvent& event, FilesToSend* file_events);
    Error AddExistingFilesToEvents(const std::string& full_path, const std::string& rel_path, FilesToSend* file_events);
    Error ProcessDeleteDirectoryInFolderEvent(const InotifyEvent& event);
    std::map<int, std::string>::iterator FindEventIterator(const InotifyEvent& event, Error* err);
    void RemoveFolderWithSubfoldersFromWatch(const std::string& path);
    std::map<int, std::string>::iterator RemoveFolderFromWatch(const std::map<int, std::string>::iterator& it);
    Error ReadInotifyEvents(int* bytes_read);
    Error ProcessInotifyEvents(int bytes_in_buffer, FilesToSend* events);
    Error FindEventPaths(const InotifyEvent& event, std::string* full_path, std::string* relative_path);
  private:
    std::unique_ptr<char[]> buffer_;
    std::map<int, std::string> watched_folders_paths_;
    int watch_fd_ = -1;
    std::string root_folder_;
    const uint64_t kProcessedFilenamesBufLen  = 1000;
    std::vector<std::string> processed_filenames_;
    uint64_t processed_filenames_counter_ = 0;
};

bool DirectoryEvent(const InotifyEvent& event);

}



#endif //ASAPO_SYSTEM_FOLDER_WATCH_LINUX_H
