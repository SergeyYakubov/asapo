#include "Inotify_event_detector.h"
#include <iostream>
#include <producer/producer.h>

namespace HIDRA2
{
    int inotifyeventdetector::InotifyEventDetector::main(int argc, char** argv)
    {

        Producer* producer = HIDRA2::Producer::create();

        if (!producer) {
            std::cerr << "Fail to create producer" << std::endl;
            return 1;
        }

        std::cout << "Running producer version: " << producer->get_version() << std::endl;


        InotifyError error;
        InotifyHelper* inotify_helper = inotify_helper->create("/tmp/data", INOTIFY_WATCH_MASK__IN_CLOSE_WRITE,
        [this](InotifyEvent* event) {
            event_handler_(event);
        }, error);

        inotify_helper->start();

        std::cout << "Successfully create producer " << std::hex << producer << std::endl;
        sleep(10);

        inotify_helper->stop();

        return 0;
    }

    void inotifyeventdetector::InotifyEventDetector::event_handler_(InotifyEvent* event)
    {
        std::cout << "File was edited: '" << event->filename << "'" << std::endl;
    }
}
