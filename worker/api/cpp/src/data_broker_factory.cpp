#include "worker/data_source.h"
#include "folder_data_broker.h"

namespace hidra2 {

std::unique_ptr<DataBroker> DataBrokerFactory::Create(std::string source_name) {
    return std::unique_ptr<DataBroker> {new FolderDataBroker(source_name)};
};
}

