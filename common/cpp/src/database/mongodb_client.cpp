#include "database/mongodb_client.h"

#include <string>

using std::string;

namespace hidra2 {

struct BsonDestroyFunctor {
    void operator() (_bson_t* bson) const {
        bson_destroy(bson);
    }
};
using bson_p = std::unique_ptr<bson_t, BsonDestroyFunctor>;

MongoDbInstance::MongoDbInstance() {
    mongoc_init ();
}

MongoDbInstance::~MongoDbInstance() {
    mongoc_cleanup ();
}

DBError MongoDBClient::Ping() {
    bson_t* command, reply;
    bson_error_t error;
    bool retval;

    command = BCON_NEW ("ping", BCON_INT32 (1));
    retval = mongoc_client_command_simple (
                 client_, "admin", command, NULL, &reply, &error);

    bson_destroy (&reply);
    bson_destroy (command);

    return !retval ? DBError::kConnectionError : DBError::kNoError;

}
MongoDBClient::MongoDBClient() {
    MongoDbInstance::Instantiate();
}

DBError MongoDBClient::InitializeClient(const string& address) {
    auto uri_str = DBAddress(address);
    client_ = mongoc_client_new (uri_str.c_str());
    if (client_ == nullptr) {
        return DBError::kBadAddress;
    }
    return DBError::kNoError;

}

void MongoDBClient::InitializeCollection(const string& database_name,
                                         const string& collection_name) {
    collection_ = mongoc_client_get_collection (client_, database_name.c_str(),
                                                collection_name.c_str());
}

DBError MongoDBClient::TryConnectDatabase() {
    auto err = Ping();
    if (err == DBError::kNoError) {
        connected_ = true;
    }
    return err;
}

DBError MongoDBClient::Connect(const string& address, const string& database_name,
                               const string& collection_name) {
    if (connected_) {
        return DBError::kAlreadyConnected;
    }

    auto err = InitializeClient(address);
    if (err != DBError::kNoError) {
        return err;
    }

    InitializeCollection(database_name, collection_name);

    err = TryConnectDatabase();
    if (err != DBError::kNoError) {
        CleanUp();
    }
    return err;
}

string MongoDBClient::DBAddress(const string& address) const {
    return "mongodb://" + address + "/?appname=hidra2";
}

void MongoDBClient::CleanUp() {
    mongoc_collection_destroy (collection_);
    mongoc_client_destroy (client_);
}

string JsonFromFileInfo(const FileInfo& file) {
    auto periods = file.modify_date.time_since_epoch().count();
    string s = "{\"_id\":" + std::to_string(file.id) + ","
               "\"size\":" + std::to_string(file.size) + ","
               "\"base_name\":\"" + file.base_name + "\","
               "\"lastchange\":" + std::to_string(periods) + ","
               "\"relative_path\":\"" + file.relative_path + "\"}";
    return s;
}

DBError MongoDBClient::Insert(const FileInfo& file) const {
    if (!connected_) {
        return DBError::kNotConnected;
    }

    bson_error_t err;
    auto s = JsonFromFileInfo(file);
    const char* json = s.c_str();

    bson_p update{bson_new_from_json((const uint8_t*)json, -1, &err)};

    if (!mongoc_collection_insert_one(collection_, update.get(), NULL, NULL, &err)) {
        if (err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            return DBError::kDuplicateID;
        }
        return DBError::kInsertError;
    }

    return DBError::kNoError;
}


MongoDBClient::~MongoDBClient() {
    if (!connected_) {
        return;
    }
    CleanUp();
}

}
