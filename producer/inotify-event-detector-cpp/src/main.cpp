#include <producer/producer.h>
#include <inotify-event-detector-cpp/inotify_helper.h>
#include <inotify-event-detector-cpp/Inotify_event_detector.h>
#include <iostream>

int main (int argc, char* argv[])
{
    HIDRA2::inotifyeventdetector::InotifyEventDetector* inotify_event_detector;
    return inotify_event_detector->main(argc, argv);
}
