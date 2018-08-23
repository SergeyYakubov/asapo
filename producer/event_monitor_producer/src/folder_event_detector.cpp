#include "folder_event_detector.h"
#include "io/io_factory.h"
#include "eventmon_logger.h"

namespace asapo {

Error FolderEventDetector::GetNextEvent(EventHeader* event_header) {
    return nullptr;
}
FolderEventDetector::FolderEventDetector(const EventMonConfig* config) : system_folder_watch__{new SystemFolderWatch()},
log__{GetDefaultEventMonLogger()}, config_{config}{

}

}