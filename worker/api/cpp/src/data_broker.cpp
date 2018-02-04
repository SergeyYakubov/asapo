#include "worker/data_broker.h"
#include "folder_data_broker.h"
#include "server_data_broker.h"


namespace hidra2 {

template <typename Broker, typename ...Args>
std::unique_ptr<DataBroker> Create(const std::string& source_name,
                                   WorkerErrorCode* return_code,
                                   Args&& ... args) noexcept {
    if (source_name.empty()) {
        *return_code = WorkerErrorCode::kEmptyDatasource;
        return nullptr;
    }

    std::unique_ptr<DataBroker> p = nullptr;
    try {
        p.reset(new Broker(source_name, std::forward<Args>(args)...));
        *return_code = WorkerErrorCode::kOK;
    } catch (...) {         // we do not test this part
        *return_code = WorkerErrorCode::kMemoryError;
    }

    return p;

}

std::unique_ptr<DataBroker> DataBrokerFactory::CreateFolderBroker(const std::string& source_name,
        WorkerErrorCode* return_code) noexcept {
    return Create<FolderDataBroker>(source_name, return_code);
};

std::unique_ptr<DataBroker> DataBrokerFactory::CreateServerBroker(const std::string& server_name,
        const std::string& source_name,
        WorkerErrorCode* return_code) noexcept {
    return Create<ServerDataBroker>(server_name, return_code, source_name);
}


}

