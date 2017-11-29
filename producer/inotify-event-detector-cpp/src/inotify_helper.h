#ifndef HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H
#define HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H

#include <string>
#include <cstdint>
#include <unistd.h>
#include <sys/inotify.h>
#include <thread>

namespace HIDRA2
{
    enum InotifyError {
        INOTIFY_ERR__OK,
        INOTIFY_ERR__FAIL_TO_CREATE_WATCH_DESCRIPTOR,
        INOTIFY_ERR__FAIL_TO_CREATE_INOTIFY_FILE_DESCRITPTOR,

    };

    enum InotifyWatchMask : uint32_t {
        INOTIFY_WATCH_MASK__IN_CLOSE_WRITE = IN_CLOSE_WRITE,
        INOTIFY_WATCH_MASK__IN_CLOSE_NOWRITE = IN_CLOSE_NOWRITE,
        INOTIFY_WATCH_MASK__IN_CLOSE = IN_CLOSE,
    };

    struct InotifyEvent {
        int watch_descriptor;
        uint32_t mask;
        uint32_t cookie;
        uint32_t filename_length;
        char filename[];
    };

    typedef std::function<void(InotifyEvent* event)> InotifyEventFunction;//void InotifyEventFunction(InotifyEvent* event, void* opaque);

    class InotifyHelper
    {
    private:
        InotifyHelper() = default;

        bool running_;

        InotifyEventFunction on_event_;
        int file_descriptor_;
        int watch_descriptor_;

        std::thread* internal_thread_handle_;
        void internal_thread_();

    public:
        InotifyHelper(const InotifyHelper&) = delete;
        InotifyHelper& operator=(const InotifyHelper&) = delete;
        ~InotifyHelper();

        static InotifyHelper* create(std::string watch_folder_path, InotifyWatchMask watch_mask, InotifyEventFunction on_event, InotifyError& error);
        void start();
        void stop();
        void free_event(InotifyEvent* event);

    };
}


#endif //HIDRA2__INOTIFYEVENTDETECTOR_INOTIFYHELPER_H
