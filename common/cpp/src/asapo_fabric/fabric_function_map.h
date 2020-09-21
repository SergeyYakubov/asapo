#ifndef ASAPO_FABRIC_DYNAMIC_CALLS_H
#define ASAPO_FABRIC_DYNAMIC_CALLS_H

#include <rdma/fabric.h>

struct FabricFunctionMap {
    bool is_init_ = false;

    uint32_t(*fi_version)();
    fi_info* (*fi_dupinfo)(const fi_info* info);
    void(*fi_freeinfo)(fi_info* info);
    int(*fi_getinfo)(uint32_t version, const char* node, const char* service,
                     uint64_t flags, const fi_info* hints, fi_info** info);
    int(*fi_fabric)(fi_fabric_attr* attr, fid_fabric** fabric, void* context);
    const char* (*fi_strerror)(int errnum);
};

FabricFunctionMap& gffm();

#endif //ASAPO_FABRIC_DYNAMIC_CALLS_H
