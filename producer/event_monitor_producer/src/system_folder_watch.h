#ifndef ASAPO_SYSTEM_FODLER_WATCH_H
#define ASAPO_SYSTEM_FODLER_WATCH_H

#ifdef _WIN32
#include "system_folder_watch_windows.h"
#endif

#if defined(__linux__)
#include "system_folder_watch_linux.h"
#endif


#if defined(__APPLE__)
#include "system_folder_watch_macos_dummy.h"
#endif


#endif //ASAPO_SYSTEM_FODLER_WATCH_H
