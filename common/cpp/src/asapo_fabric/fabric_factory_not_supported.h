#ifndef ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H
#define ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H

#include "asapo/asapo_fabric/asapo_fabric.h"

namespace asapo {
namespace fabric {
class FabricFactoryNotSupported : public FabricFactory {
  private:
    FabricErrorTemplate reason_;
  public:
    explicit FabricFactoryNotSupported(FabricErrorTemplate reason);

    std::unique_ptr<FabricServer> CreateAndBindServer(
        const AbstractLogger* logger, const std::string& host, uint16_t port, Error* error) const override;

    std::unique_ptr<FabricClient> CreateClient(Error* error) const override;
};
}
}

#endif //ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H
