#ifndef ASAPO_MOCKSYSTEMFOLDERWATCH_H
#define ASAPO_MOCKSYSTEMFOLDERWATCH_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/system_folder_watch.h"
#include "asapo/io/io.h"

namespace asapo {

class MockSystemFolderWatch : public SystemFolderWatch {
  public:
    MOCK_METHOD2(StartFolderMonitor_t, ErrorInterface * (const std::string& root_folder,
                 const std::vector<std::string>& monitored_folders));

    Error StartFolderMonitor(const std::string& root_folder, const std::vector<std::string>& monitored_folders) override {
        return Error{StartFolderMonitor_t(root_folder, monitored_folders)};

    }

    MOCK_METHOD1(GetFileEventList_t, FilesToSend (ErrorInterface** error));

    FilesToSend GetFileList(Error* err) override {
        ErrorInterface* error = nullptr;
        auto data = GetFileEventList_t(&error);
        err->reset(error);
        return data;
    };


};

}


#endif //ASAPO_MOCKSYSTEMFOLDERWATCH_H
