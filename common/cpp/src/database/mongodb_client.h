#ifndef ASAPO_MONGO_DATABASE_H
#define ASAPO_MONGO_DATABASE_H

#include "mongoc.h"

#include <string>

#include "asapo/database/database.h"

namespace asapo {

// An attempt to automize mongoc_init/mongoc_cleanup.
// One has to be carefull with cleanup order - since it is called after main exits
// it may conflict with cleanup of libraries that mongodb uses (ssl,sasl which are not used now)
// see http://mongoc.org/libmongoc/current/init-cleanup.html
class MongoDbInstance {
  public:
    static void Instantiate() {
        static MongoDbInstance instance;
    }
  private:
    MongoDbInstance();
    ~MongoDbInstance();
};

class BsonDestroyFunctor {
  public:
    void operator()(bson_t* bson) const {
        bson_destroy(bson);
    }
};

// using _bson_t here to avoid GNU compiler warnings
using bson_p = std::unique_ptr<_bson_t, BsonDestroyFunctor>;

enum class GetRecordMode {
  kById,
  kLast,
  kEarliest
};

class MongoDBClient final : public Database {
  public:
    MongoDBClient();
    Error Connect(const std::string& address, const std::string& database) override;
    Error Insert(const std::string& collection, const FileInfo& file, bool ignore_duplicates) const override;
    Error InsertAsSubset(const std::string& collection, const FileInfo& file, uint64_t subset_id, uint64_t subset_size,
                         bool ignore_duplicates) const override;
    Error Upsert(const std::string& collection, uint64_t id, const uint8_t* data, uint64_t size) const override;
    Error GetById(const std::string& collection, uint64_t id, FileInfo* file) const override;
    Error GetDataSetById(const std::string& collection, uint64_t id_in_set, uint64_t id, FileInfo* file) const override;
    Error GetStreamInfo(const std::string& collection, StreamInfo* info) const override;
    Error GetLastStream(StreamInfo* info) const override;
    ~MongoDBClient() override;
  private:
    mongoc_client_t* client_{nullptr};
    mutable mongoc_collection_t* current_collection_{nullptr};
    mutable std::string current_collection_name_;
    std::string database_name_;
    mongoc_write_concern_t* write_concern_{nullptr};
    bool connected_{false};
    void CleanUp();
    std::string DBAddress(const std::string& address) const;
    Error InitializeClient(const std::string& address);
    void UpdateCurrentCollectionIfNeeded(const std::string& collection_name) const ;
    Error Ping();
    Error TryConnectDatabase();
    Error InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const;
    Error UpdateBsonDocument(uint64_t id, const bson_p& document, bool upsert) const;
    Error AddBsonDocumentToArray(bson_t* query, bson_t* update, bool ignore_duplicates) const;
    Error GetRecordFromDb(const std::string& collection, uint64_t id, GetRecordMode mode, std::string* res) const;
    Error UpdateStreamInfo(const char *str,StreamInfo* info) const;

};

}

#endif //ASAPO_MONGO_DATABASE_H
