#ifndef ASAPO_MONGO_DATABASE_H
#define ASAPO_MONGO_DATABASE_H

#include "mongoc.h"

#include <string>

#include "database/database.h"

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

class MongoDBClient final : public Database {
  public:
    MongoDBClient();
    Error Connect(const std::string& address, const std::string& database,
                  const std::string& collection) override;
    Error Insert(const FileInfo& file, bool ignore_duplicates) const override;
    Error InsertAsSubset(const FileInfo& file, uint64_t subset_id, uint64_t subset_size,
                         bool ignore_duplicates) const override;
    Error Upsert(uint64_t id, const uint8_t* data, uint64_t size) const override;
    ~MongoDBClient() override;
  private:
    mongoc_client_t* client_{nullptr};
    mongoc_collection_t* collection_{nullptr};
    mongoc_write_concern_t* write_concern_;
    bool connected_{false};
    void CleanUp();
    std::string DBAddress(const std::string& address) const;
    Error InitializeClient(const std::string& address);
    void InitializeCollection(const std::string& database_name,
                              const std::string& collection_name);
    Error Ping();
    Error TryConnectDatabase();
    Error InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const;
    Error UpdateBsonDocument(uint64_t id, const bson_p& document, bool upsert) const;

};

}

#endif //ASAPO_MONGO_DATABASE_H
