#include "worker/data_broker.h"
#include "folder_data_broker.h"
#include "server_data_broker.h"


namespace asapo {

template <typename Broker, typename ...Args>
std::unique_ptr<DataBroker> Create(const std::string& source_name,
                                   Error* error,
                                   Args&& ... args) noexcept {
    if (source_name.empty()) {
        error->reset(new SimpleError("Empty Data Source"));
        //*return_code = WorkerErrorMessage::kEmptyDatasource;
        return nullptr;
    }

    std::unique_ptr<DataBroker> p = nullptr;
    try {
        p.reset(new Broker(source_name, std::forward<Args>(args)...));
        error->reset(nullptr);
    } catch (...) {         // we do not test this part
        error->reset(new SimpleError("Memory error"));
//       *return_code = WorkerErrorMessage::kMemoryError;
    }

    return p;

}

std::unique_ptr<DataBroker> DataBrokerFactory::CreateFolderBroker(const std::string& source_name,
        Error* error) noexcept {
    return Create<FolderDataBroker>(source_name, error);
};

std::unique_ptr<DataBroker> DataBrokerFactory::CreateServerBroker(const std::string& server_name,
        const std::string& source_name,
        Error* error) noexcept {
    return Create<ServerDataBroker>(server_name, error, source_name);
}


}

