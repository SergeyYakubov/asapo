#define __CONSUMER_C_INTERFACE_IMPLEMENTATION__
#include <consumer.h>
typedef asapo::Consumer* asapoConsumer;
typedef asapo::SourceCredentials* asapoSourceCredentials;
typedef asapo::ErrorInterface* asapoError;
#include "consumer_c.h"
#include <algorithm>

extern c {
  static void timePointToTimeSpec(std::chrono::system_clock::time_point tp,
				  struct timespec& stamp) {
    stamp.tv_sec = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    stamp.tv_nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count() % 1000000000;
  }

  
  void asapoErrorExplain(const asapoError error, char* buf, size_t maxSize) {
    auto explanation = error->Explain().substr(0, maxSize-1);
    std::copy(explanation.begin(), explanation.end(), buf);
    buf[explanation.size()] = '\0';
  }
  enum asapoErrorType asapoErrorGetType(const asapoError error) {
    return static_cast<asapoErrorType>(error->GetErrorType);
  }
  void asapoClearError(asapoError* error) {
    delete *error;
    error = nullptr;
  }

  asapoConsumer asapoCreateConsumer(const char* server_name,
				    const char* source_path,
				    _Bool has_filesysytem,
				    asapoSourceCredentials source,
				    asapoError* error) {
    asapo::Error err; 
    auto c = asapo::ConsumerFactory::CreateConsumer(server_name,
						    source_path,
						    has_filesysytem,
						    source_path,
						    &err);
    if (err) {
      error = err.release();
    }
  
    return c.release():
  }
  void asapoDeleteConsumer(asapoConsumer* consumer) {
    delete *consumer;
    consumer = nullptr;
  }

  asapoSourceCredentials asapoCreateSourceCredentials(const char* type,
						      const char* beamtime,
						      const char* beamline,
						      const char* data_source,
						      const char* token) {
    asapo::SourceType t;
    auto error = asapo::GetSourceTypeFromString(type,&t);
    return new asapo::SourceCredentials(t, beamtime, beamline,
					data_source, token);
  }
  void asapoDeleteSourceCredentials(asapoSourceCredentials* cred) {
    delete *cred;
    cred = nullptr;
  }

  const char* asapoMessageMetaGetName(const asapoMessageMeta md) {
    return md->name.c_str();
  }

  void asapoMessageMetaGetTimestamp(const asapoMessageMeta md,
				    struct timespec* stamp) {
    timePointToTimeSpec(md->timestamp, stamp);
  }
  
  uint64_t asapoMessageMetaGetSize(const asapoMessageMeta md) {
    return md->size;
  }
  uint64_t asapoMessageMetaGetId(const asapoMessageMeta md) {
    return md->id;
  }
  const char* asapoMessageMetaGetSource(const asapoMessageMeta md) {
    return md->source.c_str();
  }
  const char* asapoMessageMetaGetMetaData(const asapoMessageMeta md) {
    return md->metadata.c_str();
  }
  uint64_t asapoMessageMetaGetBuf_id(const asapoMessageMeta md) {
    return md->buf_id;
  }
  uint64_t asapoMessageMetaGetDataset_Substream(const asapoMessageMeta md) {
    return md->dataset_substream;
  }

  
}
