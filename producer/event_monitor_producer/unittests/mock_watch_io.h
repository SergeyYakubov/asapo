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
};

}


#endif //ASAPO_MOCKSYSTEMFOLDERWATCH_H
