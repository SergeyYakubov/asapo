#include "inotify_linux.h"

#include <sys/inotify.h>


namespace asapo {

int Inotify::Init() {
    return inotify_init();
}
int Inotify::AddWatch(int fd, const char* name, uint32_t mask) {
    return inotify_add_watch(fd, name, mask);
}
int Inotify::DeleteWatch(int fd, int wd) {
    return inotify_rm_watch(fd, wd);
}


ssize_t Inotify::Read(int fd, void* buf, size_t nbytes) {
    return read(fd, buf, nbytes);
}


}