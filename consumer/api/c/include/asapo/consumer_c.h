#ifndef __CONSUMER_C_H__
#define __CONSUMER_C_H__

#ifndef __CONSUMER_C_INTERFACE_IMPLEMENTATION__
typedef void* asapoConsumer;
typedef void* asapoSourceCredentials;
typedef void* asapoError;
typedef void* asapoMessageMeta;
typedef void* asapoMessageData;
typedef void* asapoGroupId;
#include <time.h>
#include <stdint.h>
#endif
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
void asapoErrorExplain(const asapoError error, char *buf, size_t maxSize);
enum asapoErrorType asapoErrorGetType(const asapoError error);
void asapoClearError(asapoError* error);

asapoConsumer asapoCreateConsumer(const char* server_name,
				  const char* source_path,
				  int has_filesysytem,
				  asapoSourceCredentials source,
				  asapoError* error);
void asapoDeleteConsumer(asapoConsumer* consumer);
asapoGroupId asapoConsumerGenerateNewGroupId(asapoConsumer consumer,asapoError* err);
void asapoDeleteGroupId(asapoGroupId* id);
void asapoConsumerSetTimeout(asapoConsumer consumer, uint64_t timeout_ms);
asapoError asapoConsumerGetLast(asapoConsumer consumer,
	                                asapoMessageMeta info,
	                                asapoMessageData* data,
	                                const char* stream);
void asapoDeleteMessageData(asapoMessageData* data);
const char*asapoMessageDataGetAsChars(const asapoMessageData data);
asapoSourceCredentials asapoCreateSourceCredentials(const char* type,
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

#endif
