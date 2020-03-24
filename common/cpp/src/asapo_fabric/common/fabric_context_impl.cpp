#include <io/io_factory.h>
#include <cstring>
#include <rdma/fi_cm.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_rma.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rdma/fi_tagged.h>
#include "fabric_context_impl.h"
#include "fabric_memory_region_impl.h"

using namespace asapo;
using namespace fabric;

std::string __PRETTY_FUNCTION_TO_NAMESPACE__(const std::string& prettyFunction)
{
    auto functionParamBegin = prettyFunction.find('(');
    auto spaceBegin = prettyFunction.substr(0, functionParamBegin).find(' ');
    return prettyFunction.substr(spaceBegin + 1, functionParamBegin - spaceBegin - 1);
}

#define FI_OK(functionCall)                                     \
    do {                                                        \
        int tmp_fi_status = functionCall;                       \
        if(__builtin_expect(tmp_fi_status, FI_SUCCESS)) {       \
            std::string tmp_fi_s = #functionCall;               \
            *error = ErrorFromFabricInternal(__PRETTY_FUNCTION_TO_NAMESPACE__(__PRETTY_FUNCTION__) + " Line " + std::to_string(__LINE__) + ": " + tmp_fi_s.substr(0, tmp_fi_s.find('(')), tmp_fi_status);\
        return;                                                 \
        }                                                       \
    } while(0)

const uint32_t FabricContextImpl::kMinExpectedLibFabricVersion = FI_VERSION(1, 9);

FabricContextImpl::FabricContextImpl() : io__{ GenerateDefaultIO() } {

}

FabricContextImpl::~FabricContextImpl() {
    StopBackgroundThreads();

    if (endpoint_)
        fi_close(&endpoint_->fid);

    if (completion_queue_)
        fi_close(&completion_queue_->fid);

    if (address_vector_)
        fi_close(&address_vector_->fid);

    if (domain_)
        fi_close(&domain_->fid);

    if (fabric_)
        fi_close(&fabric_->fid);

    if (fabric_info_)
        fi_freeinfo(fabric_info_);
}

std::string FabricContextImpl::GetAddress() const {
    sockaddr_in sin{};
    size_t sin_size = sizeof(sin);
    fi_getname(&(endpoint_->fid), &sin, &sin_size);

    // TODO Maybe expose such a function to io__
    switch(sin.sin_family) {
        case AF_INET:
            return std::string(inet_ntoa(sin.sin_addr)) + ":" + std::to_string(ntohs(sin.sin_port));
        default:
            throw std::runtime_error("Unknown addr family: " + std::to_string(sin.sin_family));
    }
}

std::unique_ptr<FabricMemoryRegion> FabricContextImpl::ShareMemoryRegion(void* src, size_t size, Error* error) {
    fid_mr* mr{};
    auto region = std::unique_ptr<FabricMemoryRegionImpl>(new FabricMemoryRegionImpl());
    int ret = fi_mr_reg(domain_, src, size,
                        FI_REMOTE_READ | FI_REMOTE_WRITE | FI_SEND | FI_RECV,
                        0, 0, 0, &mr, region.get());

    if (ret != 0) {
        *error = ErrorFromFabricInternal("fi_mr_reg", ret);
        return nullptr;
    }

    region->SetArguments(mr, (uint64_t)src, size);
    return region;
}

void FabricContextImpl::Send(FabricAddress dstAddress, FabricMessageId messageId, const void* src, size_t size,
                             Error* error) {
    HandleFiCommandWithBasicTaskAndWait(fi_tsend, error,
                                        endpoint_, src, size, nullptr, dstAddress, messageId);
}

void FabricContextImpl::Recv(FabricAddress srcAddress, FabricMessageId messageId, void* dst, size_t size,
                             Error* error) {
    HandleFiCommandWithBasicTaskAndWait(fi_trecv, error,
                                        endpoint_, dst, size, nullptr, srcAddress, messageId, 0);
}

void FabricContextImpl::RawSend(FabricAddress dstAddress, const void* src, size_t size, Error* error) {
    HandleFiCommandWithBasicTaskAndWait(fi_send, error,
                                        endpoint_, src, size, nullptr, dstAddress);
}

void FabricContextImpl::RawRecv(FabricAddress srcAddress, void* dst, size_t size, Error* error) {
    HandleFiCommandWithBasicTaskAndWait(fi_recv, error,
                                        endpoint_, dst, size, nullptr, srcAddress);
}

void
FabricContextImpl::RdmaWrite(FabricAddress dstAddress, const MemoryRegionDetails* details, const void* buffer, size_t size,
                             Error* error) {
    HandleFiCommandWithBasicTaskAndWait(fi_write, error,
                                        endpoint_, buffer, size, nullptr, dstAddress, details->addr, details->key);

}

void FabricContextImpl::InitCommon(const std::string& networkIpHint, uint16_t serverListenPort, Error* error) {
    const bool isServer = serverListenPort != 0;

    // The server must know where the packages are coming from, FI_SOURCE allows this.
    uint64_t additionalFlags = isServer ? FI_SOURCE : 0;

    fi_info* hints = fi_allocinfo();
    // We somehow have to know if we should allocate a dummy sockets domain or a real verbs domain
    if (networkIpHint == "127.0.0.1") {
        // sockets mode
        hints->fabric_attr->prov_name = strdup("sockets");
    } else {
        // verbs mode
        hints->fabric_attr->prov_name = strdup("verbs;ofi_rxm");
        hints->caps = FI_TAGGED | FI_RMA | additionalFlags;
    }

    if (isServer) {
        hints->src_addr = strdup(networkIpHint.c_str());
    } else {
        hints->dest_addr = strdup(networkIpHint.c_str());
    }

    hints->domain_attr->mr_mode = FI_MR_ALLOCATED | FI_MR_VIRT_ADDR | FI_MR_PROV_KEY;
    hints->ep_attr->type = FI_EP_RDM;
    hints->addr_format = FI_SOCKADDR_IN;

    int ret = fi_getinfo(
            kMinExpectedLibFabricVersion, networkIpHint.c_str(), isServer ? std::to_string(serverListenPort).c_str() : nullptr,
            additionalFlags, hints, &fabric_info_);

    if (ret) {
        if (ret == -FI_ENODATA) {
            *error = FabricErrorTemplates::kNoDeviceFoundError.Generate();
        } else {
            *error = ErrorFromFabricInternal("fi_getinfo", ret);
        }
        fi_freeinfo(hints);
        return;
    }
    // fprintf(stderr, fi_tostr(fabric_info_, FI_TYPE_INFO)); // Print the found fabric details

    // We have to reapply the memory mode because they get resetted
    fabric_info_->domain_attr->mr_mode = hints->domain_attr->mr_mode;
    fi_freeinfo(hints);

    FI_OK(fi_fabric(fabric_info_->fabric_attr, &fabric_, nullptr));
    FI_OK(fi_domain(fabric_, fabric_info_, &domain_, nullptr));

    fi_av_attr av_attr{};
    FI_OK(fi_av_open(domain_, &av_attr, &address_vector_, nullptr));

    fi_cq_attr cq_attr{};
    if (serverListenPort) {
        // The server must know where the data is coming from(FI_SOURCE) and what the tag is(MessageId).
        cq_attr.format = FI_CQ_FORMAT_TAGGED;
    }
    FI_OK(fi_cq_open(domain_, &cq_attr, &completion_queue_, nullptr));

    FI_OK(fi_endpoint(domain_, fabric_info_, &endpoint_, nullptr));
    FI_OK(fi_ep_bind(endpoint_, &address_vector_->fid, 0));
    FI_OK(fi_ep_bind(endpoint_, &completion_queue_->fid, FI_RECV | FI_SEND));

    FI_OK(fi_enable(endpoint_));

    StartBackgroundThreads();
}

void FabricContextImpl::StartBackgroundThreads() {
    background_threads_running_ = true;

    completion_thread_ = io__->NewThread("ASAPO/FI/CQ", [this]() {
        CompletionThread();
    });
}

void FabricContextImpl::StopBackgroundThreads() {
    background_threads_running_ = false;
    if (completion_thread_) {
        completion_thread_->join();
        completion_thread_ = nullptr;
    }
}

void FabricContextImpl::CompletionThread() {
    Error error;
    fi_cq_tagged_entry entry;
    FabricAddress tmpAddress;
    while(background_threads_running_ && !error) {
        ssize_t ret;
        ret = fi_cq_readfrom(completion_queue_, &entry, 1, &tmpAddress);
        if (ret == -FI_EAGAIN) {
            // Unfortunately the verbs;ofi_rxm provider does not support waitset
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::this_thread::yield();
            continue; // No data
        }

        if (ret == -FI_EAVAIL) {
            fi_cq_err_entry errEntry{};
            ret = fi_cq_readerr(completion_queue_, &errEntry, 0);
            if (ret != 1) {
                error = ErrorFromFabricInternal("Unknown error while fi_cq_readerr", ret);
            } else {
                auto task = (FabricWaitableTask*)(errEntry.op_context);
                task->HandleErrorCompletion(&errEntry);
            }

            continue;
        }

        if (ret != 1) {
            error = ErrorFromFabricInternal("Unknown error while fi_cq_readfrom", ret);
            break;
        }

        auto task = (FabricWaitableTask*)(entry.op_context);
        task->HandleCompletion(&entry, tmpAddress);
    }

    if (error) {
        throw std::runtime_error("ASAPO Fabric CompletionThread exited with error: " + error->Explain());
    }
}
