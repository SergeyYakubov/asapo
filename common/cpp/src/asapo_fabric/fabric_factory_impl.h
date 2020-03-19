#include <asapo_fabric/asapo_fabric.h>

#ifndef ASAPO_FABRIC_FACTORY_IMPL_H
#define ASAPO_FABRIC_FACTORY_IMPL_H

namespace asapo { namespace fabric {
    class FabricFactoryImpl : public FabricFactory {
        bool HasValidVersion(Error* error) const;

        std::unique_ptr<FabricServer> CreateAndBindServer(Error* error) const override;

        std::unique_ptr<FabricClient> CreateClient(Error* error) const override;
    };
}}

#endif //ASAPO_FABRIC_FACTORY_IMPL_H
