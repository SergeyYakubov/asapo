#include "worker/data_broker.h"
#include "folder_data_broker.h"

namespace hidra2 {

std::unique_ptr<DataBroker> DataBrokerFactory::Create(const std::string& source_name,
        WorkerErrorCode* return_code) noexcept {

    if (source_name.empty()) {
        *return_code = WorkerErrorCode::ERR__EMPTY_DATASOURCE;
        return nullptr;
    }

    std::unique_ptr<DataBroker> p = nullptr;
    try {
        p.reset(new FolderDataBroker(source_name));
        *return_code = WorkerErrorCode::ERR__NO_ERROR;
    } catch (...) {         // we do not test this part
        *return_code = WorkerErrorCode::ERR__MEMORY_ERROR;
    }

    return p;
};

}

