#include "database/mongodb_client.h"

namespace hidra2 {

using std::string;
using hidra2::Database;

std::unique_ptr<Database> MongoDatabaseFactory::Create(DBError* err) const noexcept {
    std::unique_ptr<Database> p = nullptr;
    try {
        p.reset(new MongoDBClient());
        *err = DBError::kNoError;
    } catch (...) {         // we do not test this part
        *err = DBError::kMemoryError;
    }
    return p;
};


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

bson_p PrepareBsonDocument(const FileInfo& file, DBError* err) {
    auto s = file.Json();
    auto json = reinterpret_cast<const uint8_t*>(s.c_str());
    auto bson = bson_new_from_json(json, -1, nullptr);
    if (!bson) {
        *err = DBError::kInsertError;
        return nullptr;
    }

    *err = DBError::kNoError;
    return bson_p{bson};
}

DBError MongoDBClient::InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const {
    bson_error_t mongo_err;
    if (!mongoc_collection_insert_one(collection_, document.get(), NULL, NULL, &mongo_err)) {
        if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            return ignore_duplicates ? DBError::kNoError : DBError::kDuplicateID;
        }
        return DBError::kInsertError;
    }

    return DBError::kNoError;
}


DBError MongoDBClient::Insert(const FileInfo& file, bool ignore_duplicates) const {
    if (!connected_) {
        return DBError::kNotConnected;
    }

    DBError err;
    auto document = PrepareBsonDocument(file, &err);
    if (err != DBError::kNoError) {
        return err;
    }

    return InsertBsonDocument(document, ignore_duplicates);
}


MongoDBClient::~MongoDBClient() {
    if (!connected_) {
        return;
    }
    CleanUp();
}

}
