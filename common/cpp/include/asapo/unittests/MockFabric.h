#ifndef ASAPO_MOCKFABRIC_H
#define ASAPO_MOCKFABRIC_H

#include <asapo/asapo_fabric/asapo_fabric.h>
#include <gmock/gmock.h>

namespace asapo {
namespace fabric {

class MockFabricMemoryRegion : public FabricMemoryRegion {
  public:
    MockFabricMemoryRegion() = default;
    ~MockFabricMemoryRegion() override {
        Destructor();
    }
    MOCK_METHOD(void, Destructor, (), ());
    MOCK_METHOD(const MemoryRegionDetails *, GetDetails, (), (const, override));
};

class MockFabricContext : public FabricContext {
  public:
    MOCK_METHOD(std::string, GetAddress, (), (const, override));

    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        auto data = ShareMemoryRegion_t(src, size, &err);
        error->reset(err);
        return std::unique_ptr<FabricMemoryRegion> {data};
    }
    MOCK_METHOD(FabricMemoryRegion *, ShareMemoryRegion_t, (void* src, size_t size, ErrorInterface** err), ());

    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        Send_t(dstAddress, messageId, src, size, &err);
        error->reset(err);
    }
    MOCK_METHOD(void, Send_t, (FabricAddress dstAddress, FabricMessageId messageId, const void* src, size_t size, ErrorInterface** err), ());

    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        Recv_t(srcAddress, messageId, dst, size, &err);
        error->reset(err);
    }
    MOCK_METHOD(void, Recv_t, (FabricAddress dstAddress, FabricMessageId messageId, const void* src, size_t size, ErrorInterface** err), ());

    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override {
        ErrorInterface* err = nullptr;
        RdmaWrite_t(dstAddress, details, buffer, size, &err);
        error->reset(err);
    }
    MOCK_METHOD(void, RdmaWrite_t, (FabricAddress dstAddress, const MemoryRegionDetails* details, const void* buffer, size_t size, ErrorInterface** error), ());
};

class MockFabricClient : public MockFabricContext, public FabricClient {
  public:
    FabricAddress AddServerAddress(const std::string& serverAddress, Error* error) override {
        ErrorInterface* err = nullptr;
        auto data = AddServerAddress_t(serverAddress, &err);
        error->reset(err);
        return data;
    }
    MOCK_METHOD(FabricAddress, AddServerAddress_t, (const std::string& serverAddress, ErrorInterface** err), ());
  public: // Link to FabricContext
    std::string GetAddress() const override {
        return MockFabricContext::GetAddress();
    }

    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override {
        return MockFabricContext::ShareMemoryRegion(src, size, error);
    }

    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override {
        MockFabricContext::Send(dstAddress, messageId, src, size, error);
    }

    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override {
        MockFabricContext::Recv(srcAddress, messageId, dst, size, error);
    }

    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override {
        MockFabricContext::RdmaWrite(dstAddress, details, buffer, size, error);
    }
};

class MockFabricServer : public MockFabricContext, public FabricServer {
  public:
    void RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        RecvAny_t(srcAddress, messageId, dst, size, &err);
        error->reset(err);
    }
    MOCK_METHOD(void, RecvAny_t, (FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, ErrorInterface** err), ());
  public: // Link to FabricContext
    std::string GetAddress() const override {
        return MockFabricContext::GetAddress();
    }

    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override {
        return MockFabricContext::ShareMemoryRegion(src, size, error);
    }

    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override {
        MockFabricContext::Send(dstAddress, messageId, src, size, error);
    }

    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override {
        MockFabricContext::Recv(srcAddress, messageId, dst, size, error);
    }

    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override {
        MockFabricContext::RdmaWrite(dstAddress, details, buffer, size, error);
    }
};

class MockFabricFactory : public FabricFactory {
  public:
    std::unique_ptr<FabricServer>
    CreateAndBindServer(const AbstractLogger* logger, const std::string& host, uint16_t port,
                        Error* error) const override {
        ErrorInterface* err = nullptr;
        auto data = CreateAndBindServer_t(logger, host, port, &err);
        error->reset(err);
        return std::unique_ptr<FabricServer> {data};
    }
    MOCK_METHOD(FabricServer *, CreateAndBindServer_t, (const AbstractLogger* logger, const std::string& host, uint16_t port, ErrorInterface** err), (const));

    std::unique_ptr<FabricClient> CreateClient(Error* error) const override {
        ErrorInterface* err = nullptr;
        auto data = CreateClient_t(&err);
        error->reset(err);
        return std::unique_ptr<FabricClient> {data};
    }
    MOCK_METHOD(FabricClient *, CreateClient_t, (ErrorInterface** err), (const));
};
}
}

#endif //ASAPO_MOCKFABRIC_H
