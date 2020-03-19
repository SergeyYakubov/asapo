#ifndef ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H
#define ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H

#include <asapo_fabric/asapo_fabric.h>

namespace asapo { namespace fabric {
    class FabricFactoryNotSupported : public FabricFactory {
        std::unique_ptr<FabricServer> CreateAndBindServer(Error* error) const override;

        std::unique_ptr<FabricClient> CreateClient(Error* error) const override;
    };
}}

#endif //ASAPO_FABRIC_FACTORY_NOT_SUPPORTED_H
