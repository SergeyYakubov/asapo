#include "folder_event_detector.h"
#include "io/io_factory.h"
#include "eventmon_logger.h"
#include "event_monitor_error.h"

namespace asapo {

FolderEventDetector::FolderEventDetector(const EventMonConfig* config) : system_folder_watch__{new SystemFolderWatch()},
config_{config} {
}

inline bool ends_with(std::string const& value, std::string const& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}


bool FolderEventDetector::IgnoreEvent(const FileEvent& event) {
    for (auto& ext : config_->ignored_extentions) {
        if (ends_with(event.name, ext)) {
            return true;
        }
    }
    return false;
}


Error FolderEventDetector::UpdateEventsBuffer() {
    Error err;
    auto file_events = system_folder_watch__->GetFileEventList(&err);
    if (err) {
        return err;
    }

    if (file_events.size() == 0) {
        return EventMonitorErrorTemplates::kNoNewEvent.Generate();
    }

    for (auto& event : file_events) {
        if (!IgnoreEvent(event)) {
            events_buffer_.emplace_back(EventHeader{0, event.size, event.name});
        }
    }

    return nullptr;
}


Error FolderEventDetector::GetNextEvent(EventHeader* event_header) {
    if (!monitoring_started_) {
        auto err = TextError("monitoring is not started yet");
        return err;
    }

    if (BufferIsEmpty()) {
        if (auto err = UpdateEventsBuffer()) {
            return  err;
        }
    }

    return GetHeaderFromBuffer(event_header);
}

bool FolderEventDetector::BufferIsEmpty() const {
    return events_buffer_.size() == 0;
}

Error FolderEventDetector::StartMonitoring() {
    if (monitoring_started_) {
        return nullptr;
    }

    auto err = system_folder_watch__->StartFolderMonitor(config_->root_monitored_folder, config_->monitored_subfolders);
    if (err) {
        return err;
    }

    monitoring_started_ = true;
    return nullptr;
}

Error FolderEventDetector::GetHeaderFromBuffer(EventHeader* event_header) {
    if (events_buffer_.size() == 0) {
        return EventMonitorErrorTemplates::kNoNewEvent.Generate();
    }
    *event_header = std::move(events_buffer_.front());
    events_buffer_.pop_front();
    return nullptr;
}

}