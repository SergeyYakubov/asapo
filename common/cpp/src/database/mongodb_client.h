#ifndef HIDRA2_MONGO_DATABASE_H
#define HIDRA2_MONGO_DATABASE_H

#include "mongoc.h"

#include <string>

#include "database/database.h"

namespace hidra2 {

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
    DBError Connect(const std::string& address, const std::string& database,
                    const std::string& collection) override;
    DBError Insert(const FileInfo& file, bool ignore_duplicates) const override;
    ~MongoDBClient() override;
  private:
    mongoc_client_t* client_{nullptr};
    mongoc_collection_t* collection_{nullptr};
    bool connected_{false};
    void CleanUp();
    std::string DBAddress(const std::string& address) const;
    DBError InitializeClient(const std::string& address);
    void InitializeCollection(const std::string& database_name,
                              const std::string& collection_name);
    DBError Ping();
    DBError TryConnectDatabase();
    DBError InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const;
};

}

#endif //HIDRA2_MONGO_DATABASE_H