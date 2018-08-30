#ifndef ASAPO_MOCKSYSTEMFOLDERWATCH_H
#define ASAPO_MOCKSYSTEMFOLDERWATCH_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/inotify_linux.h"

namespace asapo {

class MockInotify : public Inotify {
  public:
    MOCK_METHOD0(Init, int ());
    MOCK_METHOD3(AddWatch, int (int, const char*, uint32_t));
    MOCK_METHOD2(DeleteWatch, int (int, int));
    MOCK_METHOD3(Read, ssize_t (int, void*, size_t));
};

}


#endif //ASAPO_MOCKSYSTEMFOLDERWATCH_H
