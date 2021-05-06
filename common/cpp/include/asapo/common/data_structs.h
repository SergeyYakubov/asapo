#ifndef ASAPO_message_meta_H
#define ASAPO_message_meta_H

#include <cinttypes>
#include <chrono>
#include <memory>
#include <vector>
#include <string>

#include "error.h"

namespace asapo {

const std::string kFinishStreamKeyword = "asapo_finish_stream";
const std::string kNoNextStreamKeyword = "asapo_no_next";

class JsonStringParser;

uint64_t NanosecsEpochFromTimePoint(std::chrono::system_clock::time_point);
uint64_t EpochNanosecsFromNow();
std::chrono::system_clock::time_point TimePointfromNanosec(uint64_t nanoseconds_from_epoch);
std::string IsoDateFromEpochNanosecs(uint64_t time_from_epoch_nanosec);
uint64_t NanosecsEpochFromISODate(std::string date_time);

bool TimeFromJson(const JsonStringParser &parser, const std::string &name, std::chrono::system_clock::time_point* val);

class MessageMeta {
 public:
  std::string name;
  std::chrono::system_clock::time_point timestamp;
  uint64_t size{0};
  uint64_t id{0};
  std::string source;
  std::string metadata;
  uint64_t buf_id{0};
  uint64_t dataset_substream{0};
  std::string Json() const;
  bool SetFromJson(const std::string &json_string);
  std::string FullName(const std::string &base_path) const;
};

struct StreamInfo {
  uint64_t last_id{0};
  std::string name;
  bool finished{false};
  std::string next_stream;
  std::chrono::system_clock::time_point timestamp_created;
  std::chrono::system_clock::time_point timestamp_lastentry;
  std::string Json() const;
  bool SetFromJson(const std::string &json_string);
};

using StreamInfos = std::vector<StreamInfo>;

inline bool operator==(const MessageMeta &lhs, const MessageMeta &rhs) {
    return (lhs.name == rhs.name &&
        lhs.id == rhs.id &&
        lhs.timestamp == rhs.timestamp &&
        lhs.size == rhs.size);
}

using MessageData = std::unique_ptr<uint8_t[]>;

using MessageMetas = std::vector<MessageMeta>;


using IdList = std::vector<uint64_t>;

struct DataSet {
  uint64_t id;
  uint64_t expected_size;
  MessageMetas content;
  bool SetFromJson(const std::string &json_string);
};

using SubDirList = std::vector<std::string>;

enum class SourceType {
  kProcessed,
  kRaw
};

Error GetSourceTypeFromString(std::string stype, SourceType* type);
std::string GetStringFromSourceType(SourceType type);

struct SourceCredentials {
  SourceCredentials(SourceType type,
                    std::string beamtime,
                    std::string beamline,
                    std::string data_source,
                    std::string token) :
      beamtime_id{std::move(beamtime)},
      beamline{std::move(beamline)},
      data_source{std::move(data_source)},
      user_token{std::move(token)},
      type{type} {};
  SourceCredentials() {};
  static const std::string kDefaultDataSource;
  static const std::string kDefaultBeamline;
  static const std::string kDefaultBeamtimeId;
  std::string beamtime_id;
  std::string beamline;
  std::string data_source;
  std::string user_token;
  SourceType type = SourceType::kProcessed;
  std::string GetString() {
      return (type == SourceType::kRaw ? std::string("raw") : std::string("processed")) + "%" + beamtime_id + "%"
          + beamline + "%" + data_source + "%" + user_token;
  };
};

struct DeleteStreamOptions {
 private:
  enum DeleteStreamFlags : uint64_t {
    kDeleteMeta = 1 << 0,
    kErrorOnNotFound = 1 << 1,
  };
 public:
  DeleteStreamOptions() = default;
  DeleteStreamOptions(bool delete_meta,bool error_on_not_exist):delete_meta{delete_meta},error_on_not_exist{error_on_not_exist}{};
  bool delete_meta{true};
  bool error_on_not_exist{true};
  uint64_t Encode() {
      uint64_t flag = 0;
      flag = delete_meta ? flag | DeleteStreamFlags::kDeleteMeta:flag;
      flag = error_on_not_exist ? flag | DeleteStreamFlags::kErrorOnNotFound:flag;
      return flag;
  };
  void Decode(uint64_t flag) {
      delete_meta = (flag & DeleteStreamFlags::kDeleteMeta) > 0;
      error_on_not_exist = (flag & DeleteStreamFlags::kErrorOnNotFound) > 0;
  };
  std::string Json() {
      return std::string("{\"ErrorOnNotExist\":")+(error_on_not_exist?"true":"false")+",\"DeleteMeta\":"
      +(delete_meta?"true":"false")+"}";
  }
};

enum IngestModeFlags : uint64_t {
  kTransferData = 1 << 0,
  kTransferMetaDataOnly = 1 << 1,
  kStoreInFilesystem = 1 << 2,
  kStoreInDatabase = 1 << 3,
};

const uint64_t kDefaultIngestMode = kTransferData | kStoreInFilesystem | kStoreInDatabase;

class ClientProtocol {
 private:
  std::string version_;
  std::string discovery_version_;
  std::string name_;
 public:
  ClientProtocol(std::string version, std::string name, std::string discovery_version) : version_{version},
                                                                                         name_{name} {
      discovery_version_ = discovery_version;
  };
  ClientProtocol() = delete;
  virtual std::string GetString() = 0;
  const std::string &GetVersion() const {
      return version_;
  }
  const std::string &GetDiscoveryVersion() const {
      return discovery_version_;
  }
  const std::string &GetName() const {
      return name_;
  }
};

class ConsumerProtocol final : public ClientProtocol {
 private:
  std::string authorizer_version_;
  std::string file_transfer_service_version_;
  std::string broker_version_;
  std::string rds_version_;
 public:
  ConsumerProtocol(std::string version,
                   std::string discovery_version,
                   std::string authorizer_version,
                   std::string file_transfer_service_version,
                   std::string broker_version,
                   std::string rds_version)
      : ClientProtocol(version, "consumer protocol", discovery_version) {
      authorizer_version_ = authorizer_version;
      file_transfer_service_version_ = file_transfer_service_version;
      broker_version_ = broker_version;
      rds_version_ = rds_version;
  }
  const std::string &GetAuthorizerVersion() const {
      return authorizer_version_;
  }
  const std::string &GetFileTransferServiceVersion() const {
      return file_transfer_service_version_;
  }
  const std::string &GetRdsVersion() const {
      return rds_version_;
  }
  const std::string &GetBrokerVersion() const {
      return broker_version_;
  };
  ConsumerProtocol() = delete;
  std::string GetString() override {
      return std::string();
  }
};

class ProducerProtocol final : public ClientProtocol {
 private:
  std::string receiver_version_;
 public:
  ProducerProtocol(std::string version,
                   std::string discovery_version,
                   std::string receiver_version)
      : ClientProtocol(version, "producer protocol", discovery_version) {
      receiver_version_ = receiver_version;
  };
  const std::string &GetReceiverVersion() const {
      return receiver_version_;
  }
  ProducerProtocol() = delete;
  std::string GetString() override {
      return std::string();
  }
};

}

#endif //ASAPO_message_meta_H
