#ifndef ASAPO_FABRIC_CONTEXT_IMPL_H
#define ASAPO_FABRIC_CONTEXT_IMPL_H

#include <io/io.h>
#include <rdma/fabric.h>
#include <rdma/fi_endpoint.h>
#include <memory>
#include <asapo_fabric/asapo_fabric.h>
#include <thread>
#include "task/fabric_waitable_task.h"
#include "../fabric_internal_error.h"
#include "task/fabric_alive_check_response_task.h"

namespace asapo {
namespace fabric {

#define FI_ASAPO_ADDR_NO_ALIVE_CHECK FI_ADDR_NOTAVAIL
#define FI_ASAPO_TAG_ALIVE_CHECK ((uint64_t) -1)

/**
 * TODO: State of the bandages used in asapo to use RXM
 * If you read this _in the future_ there are hopefully fixes for the following topics:
 * Since RXM is connectionless, we do not know when an disconnect occurs.
 *  - Therefore when we try to receive data, we have added a targetAddress to HandleFiCommandAndWait,
 *    which might check if the peer is still responding to pings when a timeout occurs.
 *
 * Another issue is that in order to send data all addresses have to be added in an addressVector,
 * unfortunately, this is also required to respond to a request.
 *  - So we added a handshake procedure that sends the local address of the client with a handshake to the server.
 *    This could be fixed by FI_SOURCE_ERR, which automatically
 *    adds new connections the AV which would obsolete the handshake.
 *    At the time of writing this, FI_SOURCE_ERR is not supported with verbs;ofi_rxm
 */


const static uint64_t kRecvTaggedAnyMatch = ~0ULL;
const static uint64_t kRecvTaggedExactMatch = 0;

// TODO Use a serialization framework
struct FabricHandshakePayload {
    // Hostnames can be up to 256 Bytes long. We also need to store the port number.
    char hostnameAndPort[512];
};

class FabricContextImpl : public FabricContext {
    friend class FabricSelfRequeuingTask;
    friend class FabricAliveCheckResponseTask;
  public:
    std::unique_ptr<IO> io__;

  protected:
    FabricAliveCheckResponseTask alive_check_response_task_;

    fi_info* fabric_info_{};
    fid_fabric* fabric_{};
    fid_domain* domain_{};
    fid_cq* completion_queue_{};
    fid_av* address_vector_{};
    fid_ep* endpoint_{};

    uint64_t requestEnqueueTimeoutMs_ = 10000; // 10 sec for queuing a task
    uint64_t requestTimeoutMs_ = 20000; // 20 sec to complete a task, otherwise a ping will be send
    uint32_t maxTimeoutRetires_ = 5; // Timeout retires, if one of them fails, the task will fail with a timeout

    std::unique_ptr<std::thread> completion_thread_;
    bool background_threads_running_ = false;
  private:
    // Unfortunately when a client disconnects on sockets, a weird completion is generated. See libfabric/#5795
    bool hotfix_using_sockets_ = false;
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

    /// Without message id - No alive check!
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

    // If the targetAddress is FI_ASAPO_ADDR_NO_ALIVE_CHECK and a timeout occurs, no further ping is being done.
    // Alive check is generally only necessary if you are trying to receive data or RDMA send.
    template<class FuncType, class... ArgTypes>
    inline void HandleFiCommandWithBasicTaskAndWait(FabricAddress targetAddress, Error* error,
                                                    FuncType func, ArgTypes... args) {
        FabricWaitableTask task;
        HandleFiCommandAndWait(targetAddress, &task, error, func, args...);
    }

    template<class FuncType, class... ArgTypes>
    inline void HandleFiCommandAndWait(FabricAddress targetAddress, FabricWaitableTask* task, Error* error,
                                       FuncType func, ArgTypes... args) {
        HandleRawFiCommand(task, error, func, args...);
        if (!(*error)) { // We successfully queued our request
            InternalWait(targetAddress, task, error);
        }
    }

    template<class FuncType, class... ArgTypes>
    inline void HandleRawFiCommand(void* context, Error* error, FuncType func, ArgTypes... args) {
        ssize_t ret;
        // Since handling timeouts is an overhead, we first try to send the data regularly
        ret = func(endpoint_, args..., context);
        if (ret == -FI_EAGAIN) {
            using namespace std::chrono;
            using clock = std::chrono::high_resolution_clock;
            auto maxTime = clock::now() + milliseconds(requestEnqueueTimeoutMs_);

            do {
                std::this_thread::sleep_for(milliseconds(3));
                ret = func(endpoint_, args..., context);
            } while (ret == -FI_EAGAIN && maxTime >= clock::now());
        }

        switch (-ret) {
        case FI_SUCCESS:
            // Success
            break;
        case FI_EAGAIN: // We felt trough our own timeout loop
            *error = IOErrorTemplates::kTimeout.Generate();
            break;
        case FI_ENOENT:
            *error = FabricErrorTemplates::kConnectionRefusedError.Generate();
            break;
        default:
            *error = ErrorFromFabricInternal("HandleRawFiCommand", ret);
            break;
        }
    }

  private:
    bool TargetIsAliveCheck(FabricAddress address);
    void CompletionThread();

    void InternalWait(FabricAddress targetAddress, FabricWaitableTask* task, Error* error);

    void InternalWaitWithAliveCheck(FabricAddress targetAddress, FabricWaitableTask* task, Error* error);

    void CompletionThreadHandleErrorAvailable(Error* error);

    void CancelTask(FabricWaitableTask* task, Error* error);
};

}
}

#endif //ASAPO_FABRIC_CONTEXT_IMPL_H
