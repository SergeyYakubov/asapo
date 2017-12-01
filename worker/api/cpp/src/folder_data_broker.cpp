#include "folder_data_broker.h"

namespace hidra2 {

FolderDataBroker::FolderDataBroker(std::string source_name)
    : source_name_{source_name} {
}

int FolderDataBroker::Connect() {
    return 0;
}
}