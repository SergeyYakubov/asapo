#include "folder_event_detector.h"
#include "asapo/io/io_factory.h"
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


bool FileInList(const std::vector<std::string>& list, const std::string& file) {
    for (auto& ext : list) {
        if (ends_with(file, ext)) {
            return true;
        }
    }
    return false;
}

bool FolderEventDetector::IgnoreFile(const std::string& file) {
    return FileInList(config_->ignored_extensions, file);
}

bool FolderEventDetector::FileInWhiteList(const std::string& file) {
    if (config_->whitelisted_extensions.empty()) {
        return true;
    }
    return FileInList(config_->whitelisted_extensions, file);
}


Error FolderEventDetector::UpdateEventsBuffer() {
    Error err;
    auto files = system_folder_watch__->GetFileList(&err);
    if (err) {
        return err;
    }

    if (files.size() == 0) {
        return EventMonitorErrorTemplates::kNoNewEvent.Generate();
    }

    for (auto& file : files) {
        if (!IgnoreFile(file) && FileInWhiteList(file) ) {
            events_buffer_.emplace_back(MessageHeader{0, 0, file});
        }
    }

    return nullptr;
}


Error FolderEventDetector::GetNextEvent(MessageHeader* message_header) {
    if (!monitoring_started_) {
        auto err = TextError("monitoring is not started yet");
        return err;
    }

    if (BufferIsEmpty()) {
        if (auto err = UpdateEventsBuffer()) {
            return  err;
        }
    }

    return GetHeaderFromBuffer(message_header);
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

Error FolderEventDetector::GetHeaderFromBuffer(MessageHeader* message_header) {
    if (events_buffer_.size() == 0) {
        return EventMonitorErrorTemplates::kNoNewEvent.Generate();
    }
    *message_header = std::move(events_buffer_.front());
    events_buffer_.pop_front();
    return nullptr;
}

}