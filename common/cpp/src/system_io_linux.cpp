#include "system_wrappers/system_io.h"

#include <cstring>

#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#include <fcntl.h>

#include <cerrno>
#include <unistd.h>

using std::string;
using std::vector;
using std::chrono::system_clock;

namespace hidra2 {

bool IsDirectory(const struct dirent* entity) {
    return entity->d_type == DT_DIR &&
           strstr(entity->d_name, "..") == nullptr &&
           strstr(entity->d_name, ".") == nullptr;
}

void SetModifyDate(const struct stat& t_stat, FileInfo* file_info) {
#ifdef __APPLE__
#define st_mtim st_mtimespec
#endif
    std::chrono::nanoseconds d = std::chrono::nanoseconds {t_stat.st_mtim.tv_nsec} +
                                 std::chrono::seconds{t_stat.st_mtim.tv_sec};
#ifdef __APPLE__
#undef st_mtim
#endif

    file_info->modify_date = system_clock::time_point
    {std::chrono::duration_cast<system_clock::duration>(d)};
}

void SetFileSize(const struct stat& t_stat, FileInfo* file_info) {
    file_info->size = t_stat.st_size;
}

void SetFileName(const string& path, const string& name, FileInfo* file_info) {
    file_info->relative_path = path;
    file_info->base_name = name;
}

struct stat FileStat(const string& fname, IOError* err) {
    struct stat t_stat {};
    int res = stat(fname.c_str(), &t_stat);
    if (res < 0) {
        *err = IOErrorFromErrno();
    }
    return t_stat;
}

FileInfo GetFileInfo(const string& path, const string& name, IOError* err) {
    FileInfo file_info;

    SetFileName(path, name, &file_info);

    auto t_stat = FileStat(path + "/" + name, err);
    if (*err != IOError::kNoError) {
        return FileInfo{};
    }

    SetFileSize(t_stat, &file_info);

    SetModifyDate(t_stat, &file_info);

    return file_info;
}

void ProcessFileEntity(const struct dirent* entity, const std::string& path,
                       std::vector<FileInfo>* files, IOError* err) {

    *err = IOError::kNoError;
    if (entity->d_type != DT_REG) {
        return;
    }

    FileInfo file_info = GetFileInfo(path, entity->d_name, err);
    if (*err != IOError::kNoError) {
        return;
    }

    files->push_back(file_info);
}

void SystemIO::CollectFileInformationRecursivly(const std::string& path,
                                                std::vector<FileInfo>* files, IOError* err)  const {
    auto dir = opendir((path).c_str());
    if (dir == nullptr) {
        *err = IOErrorFromErrno();
        return;
    }

    while (struct dirent* current_entity = readdir(dir)) {
        if (IsDirectory(current_entity)) {
            CollectFileInformationRecursivly(path + "/" + current_entity->d_name,
                                             files, err);
        } else {
            ProcessFileEntity(current_entity, path, files, err);
        }
        if (*err != IOError::kNoError) {
            closedir(dir);
            return;
        }
    }
    *err = IOErrorFromErrno();
    closedir(dir);
}


int64_t SystemIO::read(int __fd, void* buf, size_t count) const noexcept {
    return (int64_t) ::read(__fd, buf, count);
}

int64_t SystemIO::write(int __fd, const void* __buf, size_t __n) const noexcept {
    return (int64_t) ::write(__fd, __buf, __n);
}

int SystemIO::open(const char* __file, int __oflag) const noexcept {
    return ::open(__file, __oflag);
}

int SystemIO::close(int __fd) const noexcept {
    return ::close(__fd);
}


}
