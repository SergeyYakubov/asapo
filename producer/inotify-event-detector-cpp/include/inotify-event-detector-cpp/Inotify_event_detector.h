#ifndef HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H
#define HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H

#include <sys/inotify.h>

namespace HIDRA2
{
    namespace inotifyeventdetector
    {
        class InotifyEventDetector
        {
        public:
            InotifyEventDetector(const InotifyEventDetector&) = delete;
            InotifyEventDetector& operator=(const InotifyEventDetector&) = delete;
            InotifyEventDetector() = default;

            int main(int argc, char* argv[]);
        private:
            void event_handler_(inotify_event* event);
        };
    }
}


#endif //HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H
