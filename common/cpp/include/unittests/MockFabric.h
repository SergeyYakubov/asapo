#ifndef ASAPO_MOCKFABRIC_H
#define ASAPO_MOCKFABRIC_H

#include <asapo_fabric/asapo_fabric.h>

namespace asapo {
namespace fabric {

class MockFabricMemoryRegion : public FabricMemoryRegion {
    MOCK_CONST_METHOD0(GetDetails, const MemoryRegionDetails * ());
};

class MockFabricContext : public FabricContext {
  public:
    MOCK_CONST_METHOD0(GetAddress, std::string());

    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        auto data = ShareMemoryRegion_t(src, size, &err);
        error->reset(err);
        return std::unique_ptr<FabricMemoryRegion> {data};
    }
    MOCK_METHOD3(ShareMemoryRegion_t, FabricMemoryRegion * (void* src, size_t size, ErrorInterface** err));

    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        Send_t(dstAddress, messageId, src, size, &err);
        error->reset(err);
    }
    MOCK_METHOD5(Send_t, void(FabricAddress dstAddress, FabricMessageId messageId,
                              const void* src, size_t size, ErrorInterface** err));

    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        Recv_t(srcAddress, messageId, dst, size, &err);
        error->reset(err);
    }
    MOCK_METHOD5(Recv_t, void(FabricAddress dstAddress, FabricMessageId messageId,
                              const void* src, size_t size, ErrorInterface** err));

    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override {
        ErrorInterface* err = nullptr;
        RdmaWrite_t(dstAddress, details, buffer, size, &err);
        error->reset(err);
    }
    MOCK_METHOD5(RdmaWrite_t, void(FabricAddress dstAddress, const MemoryRegionDetails* details, const void* buffer,
                                   size_t size, ErrorInterface** error));
};

class MockFabricClient : public MockFabricContext, public FabricClient {
    FabricAddress AddServerAddress(const std::string& serverAddress, Error* error) override {
        ErrorInterface* err = nullptr;
        auto data = AddServerAddress_t(serverAddress, &err);
        error->reset(err);
        return data;
    }
    MOCK_METHOD2(AddServerAddress_t, FabricAddress (const std::string& serverAddress, ErrorInterface** err));
};

class MockFabricServer : public MockFabricContext, public FabricServer {
  public:
    void RecvAny(FabricAddress* srcAddress, FabricMessageId* messageId, void* dst, size_t size, Error* error) override {
        ErrorInterface* err = nullptr;
        RecvAny_t(srcAddress, messageId, dst, size, &err);
        error->reset(err);
    }
    MOCK_METHOD5(RecvAny_t, void(FabricAddress* srcAddress, FabricMessageId* messageId,
                                 void* dst, size_t size, ErrorInterface** err));
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
    MOCK_CONST_METHOD4(CreateAndBindServer_t,
                       FabricServer * (const AbstractLogger* logger, const std::string& host,
                                       uint16_t port, ErrorInterface** err));

    std::unique_ptr<FabricClient> CreateClient(Error* error) const override {
        ErrorInterface* err = nullptr;
        auto data = CreateClient_t(&err);
        error->reset(err);
        return std::unique_ptr<FabricClient> {data};
    }
    MOCK_CONST_METHOD1(CreateClient_t,
                       FabricClient * (ErrorInterface** err));
};
}
}

#endif //ASAPO_MOCKFABRIC_H
