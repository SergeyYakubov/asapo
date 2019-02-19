#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <chrono>

#include "unittests/MockLogger.h"
#include "common/error.h"

#include "../../include/request/request_pool.h"
#include "../../include/request/request_handler_factory.h"
#include "mocking.h"

#include "io/io_factory.h"

namespace {

using ::testing::Return;
using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Gt;
using ::testing::Eq;
using ::testing::Ne;
using ::testing::Mock;
using ::testing::AllOf;
using testing::DoAll;
using testing::NiceMock;
using ::testing::InSequence;
using ::testing::HasSubstr;
using testing::AtLeast;
using testing::Ref;

using asapo::RequestHandler;
using asapo::RequestPool;
using asapo::Error;
using asapo::ErrorInterface;
using asapo::GenericRequest;
using asapo::GenericRequestHeader;


TEST(Request, Constructor) {
    GenericRequestHeader header{asapo::kOpcodeTransferData, 1, 2, "hello"};
    GenericRequest r{header};

    ASSERT_THAT(r.header.data_id,Eq(1));
    ASSERT_THAT(r.header.op_code,Eq(asapo::kOpcodeTransferData));
    ASSERT_THAT(r.header.data_size,Eq(2));
    ASSERT_THAT(r.header.message,testing::StrEq("hello"));
}

}
