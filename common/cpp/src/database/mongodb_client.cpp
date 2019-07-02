#include "mongodb_client.h"
#include "database/db_error.h"

namespace asapo {

using std::string;
using asapo::Database;

MongoDbInstance::MongoDbInstance() {
    mongoc_init ();
}

MongoDbInstance::~MongoDbInstance() {
    mongoc_cleanup ();
}

Error MongoDBClient::Ping() {
    bson_t* command, reply;
    bson_error_t error;
    bool retval;

    command = BCON_NEW ("ping", BCON_INT32 (1));
    retval = mongoc_client_command_simple (
                 client_, "admin", command, NULL, &reply, &error);

    bson_destroy (&reply);
    bson_destroy (command);

    return !retval ? DBErrorTemplates::kConnectionError.Generate() : nullptr;

}
MongoDBClient::MongoDBClient() {
    MongoDbInstance::Instantiate();
}

Error MongoDBClient::InitializeClient(const string& address) {
    auto uri_str = DBAddress(address);
    client_ = mongoc_client_new (uri_str.c_str());

    if (client_ == nullptr) {
        return DBErrorTemplates::kBadAddress.Generate();
    }
    return nullptr;

}

void MongoDBClient::InitializeCollection(const string& database_name,
                                         const string& collection_name) {
    collection_ = mongoc_client_get_collection (client_, database_name.c_str(),
                                                collection_name.c_str());

    write_concern_ = mongoc_write_concern_new ();
    mongoc_write_concern_set_w (write_concern_, MONGOC_WRITE_CONCERN_W_DEFAULT);
    mongoc_write_concern_set_journal (write_concern_, true);
    mongoc_collection_set_write_concern (collection_, write_concern_);
}

Error MongoDBClient::TryConnectDatabase() {
    auto err = Ping();
    if (err == nullptr) {
        connected_ = true;
    }
    return err;
}

Error MongoDBClient::Connect(const string& address, const string& database_name,
                             const string& collection_name) {
    if (connected_) {
        return DBErrorTemplates::kAlreadyConnected.Generate();
    }

    auto err = InitializeClient(address);
    if (err) {
        return err;
    }

    InitializeCollection(database_name, collection_name);

    err = TryConnectDatabase();
    if (err) {
        CleanUp();
    }
    return err;
}

string MongoDBClient::DBAddress(const string& address) const {
    return "mongodb://" + address + "/?appname=asapo";
}

void MongoDBClient::CleanUp() {
    mongoc_write_concern_destroy(write_concern_);
    mongoc_collection_destroy (collection_);
    mongoc_client_destroy (client_);
}

bson_p PrepareBsonDocument(const FileInfo& file, Error* err) {
    bson_error_t mongo_err;
    auto s = file.Json();
    auto json = reinterpret_cast<const uint8_t*>(s.c_str());
    auto bson = bson_new_from_json(json, -1, &mongo_err);
    if (!bson) {
        *err = DBErrorTemplates::kJsonParseError.Generate(mongo_err.message);
        return nullptr;
    }

    *err = nullptr;
    return bson_p{bson};
}

bson_p PrepareBsonDocument(const uint8_t* json, ssize_t len, Error* err) {
    bson_error_t mongo_err;
    auto bson = bson_new_from_json(json, len, &mongo_err);
    if (!bson) {
        *err = DBErrorTemplates::kJsonParseError.Generate(mongo_err.message);
        return nullptr;
    }

    *err = nullptr;
    return bson_p{bson};
}


Error MongoDBClient::InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const {
    bson_error_t mongo_err;
    if (!mongoc_collection_insert_one(collection_, document.get(), NULL, NULL, &mongo_err)) {
        if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            return ignore_duplicates ? nullptr : DBErrorTemplates::kDuplicateID.Generate();
        }
        return DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    return nullptr;
}


Error MongoDBClient::UpdateBsonDocument(uint64_t id, const bson_p& document, bool upsert) const {
    bson_error_t mongo_err;

    bson_t* opts = BCON_NEW ("upsert", BCON_BOOL(upsert));
    bson_t* selector = BCON_NEW ("_id", BCON_INT64 (id));

    Error err = nullptr;

    if (!mongoc_collection_replace_one(collection_, selector, document.get(), opts, NULL, &mongo_err)) {
        err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    bson_free (opts);
    bson_free (selector);

    return err;
}


Error MongoDBClient::Insert(const FileInfo& file, bool ignore_duplicates) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    Error err;
    auto document = PrepareBsonDocument(file, &err);
    if (err) {
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

Error MongoDBClient::Upsert(uint64_t id, const uint8_t* data, uint64_t size) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    Error err;
    auto document = PrepareBsonDocument(data, (ssize_t) size, &err);
    if (err) {
        return err;
    }

    if (!BSON_APPEND_INT64(document.get(), "_id", id)) {
        err = DBErrorTemplates::kInsertError.Generate("cannot assign document id ");
    }

    return UpdateBsonDocument(id, document, true);

}
Error MongoDBClient::InsertAsSubset(const FileInfo& file,
                                    uint64_t subset_id,
                                    uint64_t subset_size,
                                    bool ignore_duplicates) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    Error err;
    auto document = PrepareBsonDocument(file, &err);
    if (err) {
        return err;
    }
    auto query = BCON_NEW ("_id", BCON_INT64(subset_id));
    auto update = BCON_NEW ("$setOnInsert", "{",
                            "size", BCON_INT64 (subset_size),
                            "}",
                            "$addToSet", "{",
                            "images", BCON_DOCUMENT(document.get()), "}");


    bson_error_t mongo_err;
    if (!mongoc_collection_update (collection_, MONGOC_UPDATE_UPSERT, query, update, NULL, &mongo_err)) {
        err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    bson_destroy (query);
    bson_destroy (update);

    return err;
}

}
