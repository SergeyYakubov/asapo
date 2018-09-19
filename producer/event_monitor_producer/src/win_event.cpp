#include "win_event.h"

#include <vector>


namespace asapo {

WinEvent::WinEvent(const FILE_NOTIFY_INFORMATION* win_event): win_event_{win_event} {

}
std::string WinEvent::FileName() const {
    std::size_t len = win_event_->FileNameLength / sizeof(WCHAR);
    std::vector<char> buffer(len + 1);
    buffer[len] = 0;
//    std::locale loc("");
//    std::use_facet<std::ctype<wchar_t> >(loc).narrow(win_event_->FileName, win_event_->FileName + len, '_', &buffer[0]);
    for (size_t i = 0; i < len; i++) {
        buffer[i] = (char)win_event_->FileName[i];
    }
    return std::string(&buffer[0], &buffer[len]);
}

size_t WinEvent::Offset()const {
    return win_event_->NextEntryOffset;
}

void WinEvent::Print() const {
    printf("\nNew Event: ");
    if (win_event_->Action == FILE_ACTION_ADDED) printf("FILE_ACTION_ADDED ");
    if (win_event_->Action == FILE_ACTION_REMOVED) printf("FILE_ACTION_REMOVED ");
    if (win_event_->Action == FILE_ACTION_MODIFIED) printf("FILE_ACTION_MODIFIED ");
    if (win_event_->Action == FILE_ACTION_RENAMED_OLD_NAME) printf("FILE_ACTION_RENAMED_OLD_NAME ");
    if (win_event_->Action == FILE_ACTION_RENAMED_NEW_NAME) printf("FILE_ACTION_RENAMED_NEW_NAME ");
    printf("\n");
    if (win_event_->FileNameLength > 0)
        printf("Filename: %s\n", FileName().c_str());

}
bool WinEvent::IsFileModifiedEvent() const {
    return win_event_->Action == FILE_ACTION_MODIFIED;
}
bool WinEvent::IsFileMovedEvent() const {
    return win_event_->Action == FILE_ACTION_RENAMED_NEW_NAME;
}
bool WinEvent::ShouldInitiateTransfer() const {
    return IsFileModifiedEvent() || IsFileMovedEvent();
}
bool WinEvent::ShouldBeProcessedAfterDelay() const {
    return !IsFileMovedEvent();
}
}
