#include <inotify-event-detector-cpp/inotify_helper.h>

HIDRA2::inotifyeventdetector::InotifyHelper::~InotifyHelper() {
    inotify_rm_watch(file_descriptor_, watch_descriptor_);
    close(file_descriptor_);
    stop();
}


HIDRA2::inotifyeventdetector::INOTIFY_ERR
HIDRA2::inotifyeventdetector::InotifyHelper::create(HIDRA2::inotifyeventdetector::InotifyHelper *ref,
                                                    std::string watch_folder_path, uint32_t watch_mask,
                                                    HIDRA2::inotifyeventdetector::InotifyHelper::on_event_function on_event) {
    auto* self = new InotifyHelper();
    self->file_descriptor_ = inotify_init();
    if (self->file_descriptor_ < 0) {
        return INOTIFY_ERR__FAIL_TO_CREATE_INOTIFY_FILE_DESCRITPTOR;
    }

    self->watch_descriptor_ = inotify_add_watch(self->file_descriptor_, watch_folder_path.c_str(), watch_mask);
    if (self->watch_descriptor_ < 0) {
        return INOTIFY_ERR__FAIL_TO_CREATE_WATCH_DESCRIPTOR;
    }

    self->on_event_ = on_event;
    return INOTIFY_ERR__OK;
}

void HIDRA2::inotifyeventdetector::InotifyHelper::start() {
    if(running_) {
        return;
    }
    running_ = true;
    internal_thread_handle_ = new std::thread(internal_thread_);
}

void HIDRA2::inotifyeventdetector::InotifyHelper::stop() {
    running_ = false;
    if(internal_thread_handle_){
        internal_thread_handle_->join();
    }
}

void HIDRA2::inotifyeventdetector::InotifyHelper::internal_thread_() {
    char buffer[EVENT_BUF_LEN];
    while(running_) {
        ssize_t length = read(file_descriptor_, buffer, EVENT_BUF_LEN);//TODO if !running_ close read stream
        int event_pointer = 0;

        /*checking for error*/
        if (length < 0) {
            perror("read");
        }

        while (event_pointer < length && running_) {
            inotify_event* event = (inotify_event*) &(buffer[event_pointer]);
            on_event_(event);
            event_pointer += EVENT_SIZE + event->len;
        }
    }
}
