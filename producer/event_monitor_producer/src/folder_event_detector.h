#ifndef ASAPO_INOTOFY_EVENT_DETECTOR_H
#define ASAPO_INOTOFY_EVENT_DETECTOR_H

#include <deque>


#include "event_detector.h"
#include "eventmon_config.h"
#include "io/io.h"

#include "system_folder_watch.h"

namespace asapo {

class FolderEventDetector : public AbstractEventDetector {
  public:
    Error GetNextEvent(EventHeader* event_header) override;
    Error StartMonitoring() override;
    FolderEventDetector(const EventMonConfig* config);
    std::unique_ptr<SystemFolderWatch> system_folder_watch__;
  private:
    const EventMonConfig* config_;
    bool monitoring_started_= false;
    std::deque<EventHeader> events_buffer_;
    Error UpdateEventsBuffer();
    Error GetHeaderFromBuffer(EventHeader* event_header);
    bool IgnoreEvent(const FileEvent& event);
  bool BufferIsEmpty() const;
};

}

#endif //ASAPO_INOTOFY_EVENT_DETECTOR_H
