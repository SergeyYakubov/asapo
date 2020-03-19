#ifndef ASAPO_FABRIC_H
#define ASAPO_FABRIC_H

#include <cstdint>
#include <string>
#include <memory>
#include <common/error.h>

namespace asapo { namespace fabric {
        typedef uint64_t FabricAddress;
        typedef uint64_t FabricMessageId;

#pragma pack(push, 1)
        struct MemoryRegionDetails {
            uint64_t addr;
            uint64_t length;
            uint64_t key;
        };
#pragma pack(pop)

        class FabricMemoryRegion {
        public:
            virtual ~FabricMemoryRegion() = default;
            virtual MemoryRegionDetails* GetDetails() = 0;
        };

        class FabricContext {
        public:
            /// If this function is not called, the default timeout is 5000 ms
            virtual void SetRequestTimeout(uint64_t msTimeout) = 0;

            virtual std::string GetAddress() const = 0;

            virtual std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) = 0;

            virtual void Send(FabricAddress dstAddress, FabricMessageId messageId,
                              const void* src, size_t size, Error* error) = 0;

            virtual void Recv(FabricAddress srcAddress, FabricMessageId messageId,
                              void* dst, size_t size, Error* error) = 0;

            virtual void RdmaWrite(FabricAddress dstAddress,
                                   MemoryRegionDetails* details, const void* buffer, size_t size,
                                   Error* error) = 0;

            // Since RdmaRead heavily impacts the performance we will not implement this
            // virtual void RdmaRead(...) = 0;


        };

        class FabricClient : public FabricContext {
        public:
            virtual ~FabricClient() = default;

            virtual FabricAddress AddServerAddress(const std::string& serverAddress, Error* error) = 0;
        };

        class FabricServer : public FabricContext {
        public:
            virtual ~FabricServer() = default;

            virtual void RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* src, size_t size, Error* error) = 0;
        };

        class FabricFactory {
        public:
            virtual std::unique_ptr<FabricServer> CreateAndBindServer(Error* error) const = 0;

            virtual std::unique_ptr<FabricClient> CreateClient(Error* error) const = 0;
        };

        std::unique_ptr<FabricFactory> GenerateDefaultFabricFactory();
}}

#endif //ASAPO_FABRIC_H
