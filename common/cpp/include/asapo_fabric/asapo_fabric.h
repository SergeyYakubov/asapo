#ifndef ASAPO_FABRIC_H
#define ASAPO_FABRIC_H

#include <cstdint>
#include <string>
#include <memory>
#include <common/error.h>
#include <logger/logger.h>
#include "fabric_error.h"

namespace asapo {
namespace fabric {
typedef uint64_t FabricAddress;
typedef uint64_t FabricMessageId;

// TODO Use a serialization framework
struct MemoryRegionDetails {
    uint64_t addr;
    uint64_t length;
    uint64_t key;
};

class FabricMemoryRegion {
  public:
    virtual ~FabricMemoryRegion() = default;
    virtual const MemoryRegionDetails* GetDetails() const = 0;
};

class FabricContext {
  public:
    virtual std::string GetAddress() const = 0;

    virtual std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) = 0;

    virtual void Send(FabricAddress dstAddress, FabricMessageId messageId,
                      const void* src, size_t size, Error* error) = 0;

    virtual void Recv(FabricAddress srcAddress, FabricMessageId messageId,
                      void* dst, size_t size, Error* error) = 0;

    virtual void RdmaWrite(FabricAddress dstAddress,
                           const MemoryRegionDetails* details, const void* buffer, size_t size,
                           Error* error) = 0;

    // Since RdmaRead heavily impacts the performance we will not implement this
    // virtual void RdmaRead(...) = 0;


};

class FabricClient : public FabricContext {
  public:
    virtual ~FabricClient() = default;

    /// The serverAddress must be in this format: "hostname:port"
    virtual FabricAddress AddServerAddress(const std::string& serverAddress, Error* error) = 0;
};

class FabricServer : public FabricContext {
  public:
    virtual ~FabricServer() = default;

    virtual void RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, Error* error) = 0;
};

class FabricFactory {
  public:
    /**
     * Creates a new server and will immediately allocate and listen to the given host:port
     */
    virtual std::unique_ptr<FabricServer>
    CreateAndBindServer(const AbstractLogger* logger, const std::string& host, uint16_t port,
                        Error* error) const = 0;

    /**
     * Will allocate a proper domain as soon as the client gets his first server address added
     */
    virtual std::unique_ptr<FabricClient> CreateClient(Error* error) const = 0;
};

std::unique_ptr<FabricFactory> GenerateDefaultFabricFactory();
}
}

#endif //ASAPO_FABRIC_H
