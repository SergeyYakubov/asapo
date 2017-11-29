#include <inotify-event-detector-cpp/Inotify_event_detector.h>
#include <producer/producer.h>
#include <iostream>
#include <inotify-event-detector-cpp/inotify_helper.h>

namespace HIDRA2
{
    int inotifyeventdetector::InotifyEventDetector::main(int argc, char** argv)
    {

        std::cout << "Running producer version: " << Producer::kVersion << std::endl;

        Producer* producer = HIDRA2::Producer::CreateProducer("127.0.0.1");
        if (!producer) {
            std::cerr << "Fail to create producer" << std::endl;
            return 1;
        }

        inotifyeventdetector::InotifyHelper* inotify_helper;

        //inotifyeventdetector::InotifyHelper::on_event_function_t a = std::bind(inotifyeventdetector::InotifyEventDetector::event_handler_, this);
        typedef void (*FPtr)(void);
        FPtr p = inotifyeventdetector::InotifyEventDetector::event_handler_;


        if (!inotify_helper->create(inotify_helper, "/tmp/data", inotifyeventdetector::INOTIFY_WATCH_MASK__IN_CLOSE_WRITE,
                                    (InotifyHelper::on_event_function_t) p, this));

        inotify_helper->start();

        std::cout << "Successfully create producer " << std::hex << producer << std::endl;
        sleep(10);

        inotify_helper->stop();

        return 0;
    }

    void inotifyeventdetector::InotifyEventDetector::event_handler_(inotify_event* event)
    {
        std::cout << "File was edited: '" << event->name << "'" << std::endl;
    }
}
