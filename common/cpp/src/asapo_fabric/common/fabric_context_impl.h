#ifndef ASAPO_FABRIC_CONTEXT_IMPL_H
#define ASAPO_FABRIC_CONTEXT_IMPL_H

#include <io/io.h>
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <memory>
#include <asapo_fabric/asapo_fabric.h>
#include <thread>
#include "fabric_waitable_task.h"
#include "../fabric_internal_error.h"

namespace asapo {
namespace fabric {
// TODO Use a serialization framework
struct FabricHandshakePayload {
    // Hostnames can be up to 256 Bytes long. We also need to store the port number.
    char hostnameAndPort[512];
};

class FabricContextImpl : public FabricContext {
  public:
    std::unique_ptr<IO> io__;
  protected:
    fi_info* fabric_info_{};
    fid_fabric* fabric_{};
    fid_domain* domain_{};
    fid_cq* completion_queue_{};
    fid_av* address_vector_{};
    fid_ep* endpoint_{};

    uint64_t requestTimeoutMs_ = 10000; // 10 sec should be enough. TODO: maybe make a public variable setter

    std::unique_ptr<std::thread> completion_thread_;
    bool background_threads_running_ = false;
  public:
    explicit FabricContextImpl();
    virtual ~FabricContextImpl();

    static const uint32_t kMinExpectedLibFabricVersion;

    std::string GetAddress() const override;

    /// The memory will be shared until the result is freed
    std::unique_ptr<FabricMemoryRegion> ShareMemoryRegion(void* src, size_t size, Error* error) override;

    /// With message id
    void Send(FabricAddress dstAddress, FabricMessageId messageId,
              const void* src, size_t size, Error* error) override;
    void Recv(FabricAddress srcAddress, FabricMessageId messageId,
              void* dst, size_t size, Error* error) override;

    /// Without message id
    void RawSend(FabricAddress dstAddress,
                 const void* src, size_t size, Error* error);
    void RawRecv(FabricAddress srcAddress,
                 void* dst, size_t size, Error* error);

    /// Rdma
    void RdmaWrite(FabricAddress dstAddress,
                   const MemoryRegionDetails* details, const void* buffer, size_t size,
                   Error* error) override;

  protected:
    /// If client serverListenPort == 0
    void InitCommon(const std::string& networkIpHint, uint16_t serverListenPort, Error* error);

    void StartBackgroundThreads();
    void StopBackgroundThreads();

    template<class FuncType, class... ArgTypes>
    inline void HandleFiCommandWithBasicTaskAndWait(FuncType func, Error* error, ArgTypes... args) {
        FabricWaitableTask task;
        HandleFiCommandAndWait(func, &task, error, args...);
    }

    template<class FuncType, class... ArgTypes>
    inline void HandleFiCommandAndWait(FuncType func, FabricWaitableTask* task, Error* error, ArgTypes... args) {
        HandleFiCommand(func, task, error, args...);
        if (!(*error)) {
            task->Wait(requestTimeoutMs_, error);

            // If a timeout occurs we want to cancel the action,
            // which invokes an 'Operation canceled' error in the completion queue.
            if (*error == FabricErrorTemplates::kTimeout) {
                fi_cancel(&endpoint_->fid, task);
                task->Wait(0, error);
                // We expect the task to fail with 'Operation canceled'
                if (*error == FabricErrorTemplates::kInternalOperationCanceledError) {
                    // Switch it to a timeout so its more clearly what happened
                    *error = FabricErrorTemplates::kTimeout.Generate();
                }
            }
        }
    }

    template<class FuncType, class... ArgTypes>
    inline void HandleFiCommand(FuncType func, void* context, Error* error, ArgTypes... args) {
        ssize_t ret;
        // Since handling timeouts is an overhead, we first try to send the data regularly
        ret = func(args..., context);
        if (ret == -FI_EAGAIN) {
            using namespace std::chrono;
            using clock = std::chrono::high_resolution_clock;
            auto maxTime = clock::now() + milliseconds(requestTimeoutMs_);

            do {
                std::this_thread::sleep_for(milliseconds(3));
                ret = func(args..., context);
            } while (ret == -FI_EAGAIN && maxTime >= clock::now());
        }

        if (ret != 0) {
            switch (-ret) {
            case FI_EAGAIN:
                *error = FabricErrorTemplates::kTimeout.Generate();
                break;
            case FI_ENOENT:
                *error = FabricErrorTemplates::kConnectionRefusedError.Generate();
                break;
            default:
                *error = ErrorFromFabricInternal("HandleFiCommandAndWait", ret);
            }
            return;
        }
    }

  private:
    void CompletionThread();
};

}
}

#endif //ASAPO_FABRIC_CONTEXT_IMPL_H
