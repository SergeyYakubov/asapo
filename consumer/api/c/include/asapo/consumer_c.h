#ifndef __CONSUMER_C_H__
#define __CONSUMER_C_H__

#ifndef __CONSUMER_C_INTERFACE_IMPLEMENTATION__
typedef int asapoBool;
typedef void* asapoConsumer;
typedef void* asapoSourceCredentials;
typedef void* asapoError;
typedef void* asapoMessageMeta;
typedef void* asapoMessageData;
typedef void* asapoString;
typedef void* asapoStreamInfo;
typedef void* asapoStreamInfos;
typedef void* asapoIdList;
#include <time.h>
#include <stdint.h>
#endif
//! c version of asapo::ErrorType
enum asapoErrorType {
    kUnknownError = 0,
    kAsapoError,
    kHttpError,
    kIOError,
    kDBError,
    kReceiverError,
    kProducerError,
    kConsumerError,
    kMemoryAllocationError,
    kEndOfFile,
    kFabricError,
};
//! c version of asapo::StreamFilter
enum asapoStreamFilter {
    kAllStreams,
    kFinishedStreams,
    kUnfinishedStreams
};
//! c version of asapo::SourceType
enum asapoSourceType {
    kProcessed,
    kRaw
};
//! c version of asapo::NetworkConnectionType
enum asapoNetworkConnectionType {
    kUndefined,
    kAsapoTcp,
    kFabric
};
void asapoErrorExplain(const asapoError error, char* buf, size_t maxSize);
enum asapoErrorType asapoErrorGetType(const asapoError error);
void asapoClearError(asapoError* error);

asapoConsumer asapoCreateConsumer(const char* server_name,
                                  const char* source_path,
                                  asapoBool has_filesysytem,
                                  asapoSourceCredentials source,
                                  asapoError* error);
void asapoDeleteConsumer(asapoConsumer* consumer);
asapoString asapoConsumerGenerateNewGroupId(asapoConsumer consumer, asapoError* err);
asapoString asapoCreateString(const char* content);
void asapoStringAppend(asapoString str, const char* content);
const char* asapoStringC_str(const asapoString str);
size_t asapoStringSize(const asapoString str);
void asapoDeleteString(asapoString* str);

void asapoConsumerSetTimeout(asapoConsumer consumer, uint64_t timeout_ms);
asapoError asapoConsumerResetLastReadMarker(asapoConsumer consumer,
                                            const asapoString group_id,
                                            const char* stream);
asapoError asapoConsumerSetLastReadMarker(asapoConsumer consumer,
                                          const asapoString group_id,
                                          uint64_t value,
                                          const char* stream);
asapoError asapoConsumerAcknowledge(asapoConsumer consumer,
                                    const asapoString group_id,
                                    uint64_t id,
                                    const char* stream);
asapoError asapoConsumerNegativeAcknowledge(asapoConsumer consumer,
                                            const asapoString group_id,
                                            uint64_t id,
                                            uint64_t delay_ms,
                                            const char* stream);
asapoIdList asapoConsumerGetUnacknowledgedMessages(asapoConsumer consumer,
        asapoString group_id,
        uint64_t from_id,
        uint64_t to_id,
        const char* stream,
        asapoError* error);
void asapoDeleteIdList(asapoIdList* list);
size_t asapoIdListGetSize(const asapoIdList list);
uint64_t asapoIdListGetItem(const asapoIdList list,
                            size_t index);

void asapoConsumerForceNoRdma(asapoConsumer consumer);
enum asapoNetworkConnectionType asapoConsumerCurrentConnectionType(asapoConsumer consumer);


asapoStreamInfos asapoConsumerGetStreamList(asapoConsumer consumer,
                                            const char* from,
                                            enum asapoStreamFilter filter,
                                            asapoError* error);
const asapoStreamInfo asapoStreamInfosGetItem(const asapoStreamInfos infos,
                                              size_t index);
size_t asapoStreamInfosGetSize(const asapoStreamInfos infos);
void asapoDeleteStreamInfos(asapoStreamInfos* infos);

asapoError asapoConsumerDeleteStream(asapoConsumer consumer,
                                     const char* stream,
                                     asapoBool delete_meta,
                                     asapoBool error_on_not_exist);
uint64_t asapoConsumerGetCurrentSize(asapoConsumer consumer,
                                     const char* stream,
                                     asapoError* error);
uint64_t asapoConsumerGetCurrentDatasetCount(asapoConsumer consumer,
                                             const char* stream,
                                             asapoBool include_incomplete,
                                             asapoError* error);
asapoString asapoConsumerGetBeamtimeMeta(asapoConsumer consumer,
                                         asapoError* error);
asapoError asapoConsumerRetriveData(asapoConsumer consumer,
                                    asapoMessageMeta info,
                                    asapoMessageData* data);



asapoError asapoConsumerGetLast(asapoConsumer consumer,
                                asapoMessageMeta info,
                                asapoMessageData* data,
                                const char* stream);
asapoError asapoConsumerGetNext(asapoConsumer consumer,
                                asapoString group_id,
                                asapoMessageMeta info,
                                asapoMessageData* data,
                                const char* stream);
void asapoDeleteMessageData(asapoMessageData* data);
const char* asapoMessageDataGetAsChars(const asapoMessageData data);
asapoSourceCredentials asapoCreateSourceCredentials(enum asapoSourceType type,
        const char* beamtime,
        const char* beamline,
        const char* data_source,
        const char* token);
void asapoDeleteSourceCredentials(asapoSourceCredentials* cred);

asapoMessageMeta asapoCreateMessageMeta();
void asapoDeleteMessageMeta(asapoMessageMeta* meta);

const char* asapoMessageMetaGetName(const asapoMessageMeta md);
void asapoMessageMetaGetTimestamp(const asapoMessageMeta md,
                                  struct timespec* stamp);
uint64_t asapoMessageMetaGetSize(const asapoMessageMeta md);
uint64_t asapoMessageMetaGetId(const asapoMessageMeta md);
const char* asapoMessageMetaGetSource(const asapoMessageMeta md);
const char* asapoMessageMetaGetMetadata(const asapoMessageMeta md);
uint64_t asapoMessageMetaGetBuf_id(const asapoMessageMeta md);
uint64_t asapoMessageMetaGetDataset_Substream(const asapoMessageMeta md);

uint64_t asapoStreamInfoGetLast_id(const asapoStreamInfo info);
const char* asapoStreamInfoGetName(const asapoStreamInfo info);
asapoBool asapoStreamInfoGetFfinished(const asapoStreamInfo info);
const char* asapoStreamInfoGetNext_stream(const asapoStreamInfo info);
void asapoStreamInfoGetTimestampCreated(const asapoStreamInfo info,
                                        struct timespec* stamp);
void asapoStreamInfoGetTimestamoLastEntry(const asapoStreamInfo info,
                                          struct timespec* stamp);


#endif
