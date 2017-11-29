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

        enum INOTIFY_WATCH_MASK : uint32_t {
            INOTIFY_WATCH_MASK__IN_CLOSE_WRITE = IN_CLOSE_WRITE,
            INOTIFY_WATCH_MASK__IN_CLOSE_NOWRITE = IN_CLOSE_NOWRITE,
            INOTIFY_WATCH_MASK__IN_CLOSE = IN_CLOSE,
        };

        class InotifyHelper
        {
        public:
            typedef void on_event_function_t (void* arg1, inotify_event*);

            InotifyHelper(const InotifyHelper&) = delete;
            InotifyHelper& operator=(const InotifyHelper&) = delete;
            ~InotifyHelper();

            static INOTIFY_ERR create(InotifyHelper*& ref, std::string watch_folder_path, INOTIFY_WATCH_MASK watch_mask, on_event_function_t on_event, void* on_event_arg1);
            void start();
            void stop();
            void free_event(inotify_event* event);

        private:
            InotifyHelper() = default;

            bool running_;

            on_event_function_t* on_event_;
            void* on_event_arg1_;
            int file_descriptor_;
            int watch_descriptor_;

            std::thread* internal_thread_handle_;
            void internal_thread_();
        };
    }
}


#endif //HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H
