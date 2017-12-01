#include <gtest/gtest.h>

#include "worker/data_source.h"
#include "../src/folder_data_broker.h"

using hidra2::DataBrokerFactory;
using hidra2::DataBroker;
using hidra2::FolderDataBroker;

namespace {

TEST(WorkerAPI, CanCreateFolderDataSource) {

    auto data_broker=DataBrokerFactory::Create("folder");

    EXPECT_NE(dynamic_cast<FolderDataBroker*>(data_broker.release()),nullptr);
}


}
