#include <gtest/gtest.h>

#include "worker/data_broker.h"
#include "../src/folder_data_broker.h"

using hidra2::DataBrokerFactory;
using hidra2::FolderDataBroker;

namespace {

TEST(WorkerAPI, CanCreateFolderDataSource) {
    {
        auto data_broker = DataBrokerFactory::create("path/to/file");

    }
    SUCCEED();
//    EXPECT_NE(dynamic_cast<FolderDataBroker*>(data_broker.release()),nullptr);
}


}
