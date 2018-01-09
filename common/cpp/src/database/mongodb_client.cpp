#include "database/mongodb_client.h"

namespace hidra2 {

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

DBError MongoDBClient::InitializeClient(const std::string& address) {
    auto uri_str = DBAddress(address);
    client_ = mongoc_client_new (uri_str.c_str());
    if (client_ == nullptr) {
        return DBError::kBadAddress;
    }
    return DBError::kNoError;

}

void MongoDBClient::InitializeCollection(const std::string& database_name,
                                         const std::string& collection_name) {
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

DBError MongoDBClient::Connect(const std::string& address, const std::string& database_name,
                               const std::string& collection_name) {
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

std::string MongoDBClient::DBAddress(const std::string& address) const {
    return "mongodb://" + address + "/?appname=hidra2";
}

void MongoDBClient::CleanUp() {
    mongoc_collection_destroy (collection_);
    mongoc_client_destroy (client_);
}

DBError MongoDBClient::Import(const FileInfos& files) const {
    if (!connected_) {
        return DBError::kNotConnected;
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
