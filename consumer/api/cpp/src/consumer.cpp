#include "asapo/common/networking.h"
#include "asapo/consumer/consumer.h"
#include "consumer_impl.h"
#include "asapo/consumer/consumer_error.h"

namespace asapo {

template <typename C, typename ...Args>
std::unique_ptr<Consumer> Create(const std::string& source_name,
                                 Error* error,
                                 Args&& ... args) noexcept {
    if (source_name.empty()) {
        *error = ConsumerErrorTemplates::kWrongInput.Generate("Empty Data Source");
        return nullptr;
    }

    std::unique_ptr<Consumer> p = nullptr;
    try {
        p.reset(new C(source_name, std::forward<Args>(args)...));
        error->reset(nullptr);
    } catch (...) {         // we do not test this part
        error->reset(new SimpleError("Memory error"));
    }

    return p;

}

std::unique_ptr<Consumer> ConsumerFactory::CreateConsumer(std::string server_name, std::string source_path,
                                                          bool has_filesystem, SourceCredentials source, Error* error) noexcept {
    return Create<ConsumerImpl>(std::move(server_name), error, std::move(source_path), has_filesystem,
                                std::move(source));
}


}

