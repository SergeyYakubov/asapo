#include <inotify-event-detector-cpp/inotify_helper.h>
#include <iostream>
#include <cstring>

namespace HIDRA2
{
    namespace inotifyeventdetector
    {
        InotifyHelper::~InotifyHelper()
        {
            inotify_rm_watch(file_descriptor_, watch_descriptor_);
            close(file_descriptor_);
            stop();
        }

        INOTIFY_ERR
        InotifyHelper::create(InotifyHelper*& ref,
                              std::string watch_folder_path, INOTIFY_WATCH_MASK watch_mask,
                              InotifyHelper::on_event_function_t on_event, void* on_event_arg1)
        {
            auto* self = new InotifyHelper();
            self->file_descriptor_ = inotify_init();
            if (self->file_descriptor_ < 0) {
                return INOTIFY_ERR__FAIL_TO_CREATE_INOTIFY_FILE_DESCRITPTOR;
            }

            self->watch_descriptor_ = inotify_add_watch(self->file_descriptor_, watch_folder_path.c_str(), watch_mask);
            if (self->watch_descriptor_ < 0) {
                return INOTIFY_ERR__FAIL_TO_CREATE_WATCH_DESCRIPTOR;
            }

            self->running_ = false;
            self->on_event_ = on_event;
            self->on_event_arg1_ = on_event_arg1;

            ref = self;
            return INOTIFY_ERR__OK;
        }

        void InotifyHelper::start()
        {
            if(running_) {
                return;
            }
            running_ = true;
            internal_thread_handle_ = new std::thread(&InotifyHelper::internal_thread_, this);
        }

        void InotifyHelper::stop()
        {
            running_ = false;
            if(internal_thread_handle_) {
                internal_thread_handle_->join();
            }
        }

        void InotifyHelper::internal_thread_()
        {
            std::cout << "Hello world" << std::endl;
            char buffer[EVENT_BUF_LEN];
            ssize_t length;
            while(running_) {
                fd_set read_fds, write_fds, except_fds;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                FD_ZERO(&except_fds);
                FD_SET(file_descriptor_, &read_fds);

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                if (select(file_descriptor_ + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1) {
                    length = read(file_descriptor_, buffer, EVENT_BUF_LEN);
                } else {
                    continue;// A timeout occurred
                }

                int event_pointer = 0;

                /*checking for error*/
                if (length < 0) {
                    perror("read");
                }

                while (event_pointer < length && running_) {
                    inotify_event* event = (inotify_event*) &(buffer[event_pointer]);
                    inotify_event* copy = (inotify_event *) malloc(sizeof(inotify_event) + event->len);
                    memmove(copy, event, sizeof(inotify_event) + event->len);
                    on_event_(on_event_arg1_, copy);
                    event_pointer += EVENT_SIZE + event->len;
                }
            }
        }

        void InotifyHelper::free_event(inotify_event* event)
        {
            free(event);
        }
    }
}
