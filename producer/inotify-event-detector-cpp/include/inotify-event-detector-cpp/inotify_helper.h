#ifndef HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H
#define HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H

#include <string>
#include <cstdint>
#include <unistd.h>
#include <sys/inotify.h>
#include <thread>

#define EVENT_SIZE        ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

namespace HIDRA2
{
    namespace inotifyeventdetector
    {
        enum INOTIFY_ERR {
            INOTIFY_ERR__OK,
            INOTIFY_ERR__FAIL_TO_CREATE_WATCH_DESCRIPTOR,
            INOTIFY_ERR__FAIL_TO_CREATE_INOTIFY_FILE_DESCRITPTOR,

        };

        class InotifyHelper
        {
        public:
            typedef void on_event_function(inotify_event*);

            InotifyHelper(const InotifyHelper&) = delete;
            InotifyHelper& operator=(const InotifyHelper&) = delete;
            ~InotifyHelper();

            static INOTIFY_ERR create(InotifyHelper* ref, std::string watch_folder_path, uint32_t watch_mask, on_event_function on_event);
            void start();
            void stop();
        private:
            InotifyHelper() = default;

            bool running_;

            on_event_function on_event_;
            int file_descriptor_;
            int watch_descriptor_;
            int error_ = 0;

            std::thread* internal_thread_handle_;
            void internal_thread_();
        };
    }
}


#endif //HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H
