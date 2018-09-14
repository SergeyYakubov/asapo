#ifndef ASAPO_MOCKSYSTEMFOLDERWATCH_H
#define ASAPO_MOCKSYSTEMFOLDERWATCH_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/watch_io.h"

namespace asapo {

class MockWatchIO : public WatchIO {
  public:
   HANDLE Init(const char* folder, Error* err) override {
       ErrorInterface* error = nullptr;
       auto handle = Init_t(folder,&error);
       err->reset(error);
       return handle;
   }

  MOCK_METHOD2(Init_t, HANDLE (const char* folder, ErrorInterface** err));

  Error ReadDirectoryChanges(HANDLE handle,LPVOID buffer, DWORD buffer_length,LPDWORD bytes_returned) override {
      return Error{ReadDirectoryChanges_t(handle,buffer,buffer_length,bytes_returned)};
  }

  MOCK_METHOD4(ReadDirectoryChanges_t, ErrorInterface* (HANDLE handle,LPVOID buffer, DWORD buffer_length,LPDWORD bytes_returned));

  MOCK_METHOD1(IsDirectory, bool (const std::string&));


  };

}


#endif //ASAPO_MOCKSYSTEMFOLDERWATCH_H
