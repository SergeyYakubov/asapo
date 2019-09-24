#include "consumer/data_broker.h"
#include "server_data_broker.h"


namespace asapo {

template <typename Broker, typename ...Args>
std::unique_ptr<DataBroker> Create(const std::string& source_name,
                                   Error* error,
                                   Args&& ... args) noexcept {
    if (source_name.empty()) {
        error->reset(new SimpleError("Empty Data Source"));
        return nullptr;
    }

    std::unique_ptr<DataBroker> p = nullptr;
    try {
        p.reset(new Broker(source_name, std::forward<Args>(args)...));
        error->reset(nullptr);
    } catch (...) {         // we do not test this part
        error->reset(new SimpleError("Memory error"));
    }

    return p;

}

std::unique_ptr<DataBroker> DataBrokerFactory::CreateServerBroker(std::string server_name, std::string source_path,
        SourceCredentials source,
        Error* error) noexcept {
    return Create<ServerDataBroker>(std::move(server_name), error, std::move(source_path), std::move(source));
}


}

