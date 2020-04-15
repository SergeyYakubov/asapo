#include <asapo_fabric/asapo_fabric.h>

#ifndef ASAPO_FABRIC_FACTORY_IMPL_H
#define ASAPO_FABRIC_FACTORY_IMPL_H

namespace asapo {
namespace fabric {
class FabricFactoryImpl : public FabricFactory {
  public:
    bool HasValidVersion(Error* error) const;

    std::unique_ptr<FabricServer>
    CreateAndBindServer(const AbstractLogger* logger,
                        const std::string& host, uint16_t port, Error* error) const override;

    std::unique_ptr<FabricClient> CreateClient(Error* error) const override;
};
}
}

#endif //ASAPO_FABRIC_FACTORY_IMPL_H
