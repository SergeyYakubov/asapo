#include "../../api/src/producer_impl.h"
#include "inotify_helper.h"
#include "Inotify_event_detector.h"
#include <iostream>

int main (int argc, char* argv[])
{
    HIDRA2::inotifyeventdetector::InotifyEventDetector* inotify_event_detector;
    return inotify_event_detector->main(argc, argv);
}
