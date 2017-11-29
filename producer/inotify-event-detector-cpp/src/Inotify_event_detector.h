#ifndef HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H
#define HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H

#include <sys/inotify.h>
#include "inotify_helper.h"

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
            void event_handler_(InotifyEvent* event);
        };
    }
}


#endif //HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYEVENTDETECTOR_H
