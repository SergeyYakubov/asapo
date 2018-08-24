#include "folder_event_detector.h"
#include "io/io_factory.h"
#include "eventmon_logger.h"
#include "event_monitor_error.h"

namespace asapo {

FolderEventDetector::FolderEventDetector(const EventMonConfig* config) : system_folder_watch__{new SystemFolderWatch()},
config_{config}{
}

Error FolderEventDetector::GetNextEvent(EventHeader* event_header) {
    if (!monitoring_started_) {
            auto err = TextError("monitoring is not started yet");
            return err;
    }

    Error err;
    auto file_events = system_folder_watch__->GetFileEventList(&err);
    if (err) {
        return err;
    }
    if (file_events.size() == 0) {
        return EventMonitorErrorTemplates::kNoNewEvent.Generate();
    }

    FileEvent file_event = file_events[0];
    event_header->file_size = file_event.size;
    event_header->file_name = std::move(file_event.name);
    return nullptr;
}

Error FolderEventDetector::StartMonitoring() {
    if (monitoring_started_) {
        return nullptr;
    }

    auto err = system_folder_watch__->StartFolderMonitor(config_->monitored_folders);
    if (err) {
        return err;
    }

    monitoring_started_ = true;
    return nullptr;
}

}