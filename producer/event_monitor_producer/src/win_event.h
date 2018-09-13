#ifndef ASAPO_WIN_EVENT_H
#define ASAPO_WIN_EVENT_H

#include <windows.h>
#include <winnt.h>
#include <string>

namespace asapo {

class WinEvent {
 public:
  WinEvent(const FILE_NOTIFY_INFORMATION* win_event);
  size_t Offset() const;
  void Print() const;
  std::string FileName() const ;
 private:
  const FILE_NOTIFY_INFORMATION* win_event_;
};

}

#endif //ASAPO_WIN_EVENT_H
