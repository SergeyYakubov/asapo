#include "worker/data_broker.h"
#include "folder_data_broker.h"

namespace hidra2 {


std::unique_ptr<DataBroker> DataBrokerFactory::create(std::string source_name) noexcept {
    std::unique_ptr<DataBroker> p;
    try {
        p = (std::unique_ptr<DataBroker>)new FolderDataBroker(source_name);
    }
    catch (...){
        return nullptr;
    }

    return p;
};


}

