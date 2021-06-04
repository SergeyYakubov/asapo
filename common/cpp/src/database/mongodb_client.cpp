#include "asapo/json_parser/json_parser.h"
#include "mongodb_client.h"
#include "encoding.h"

#include <chrono>
#include <regex>

#include "asapo/database/db_error.h"
#include "asapo/common/data_structs.h"

namespace asapo {

using asapo::Database;

MongoDbInstance::MongoDbInstance() {
    mongoc_init();
}

MongoDbInstance::~MongoDbInstance() {
    mongoc_cleanup();
}

Error MongoDBClient::Ping() {
    bson_t* command, reply;
    bson_error_t error;
    bool retval;

    command = BCON_NEW ("ping", BCON_INT32(1));
    retval = mongoc_client_command_simple(
        client_, "admin", command, NULL, &reply, &error);

    bson_destroy(&reply);
    bson_destroy(command);

    return !retval ? DBErrorTemplates::kConnectionError.Generate() : nullptr;

}
MongoDBClient::MongoDBClient() {
    MongoDbInstance::Instantiate();
}

Error MongoDBClient::InitializeClient(const std::string &address) {
    auto uri_str = DBAddress(address);
    client_ = mongoc_client_new(uri_str.c_str());

    if (client_ == nullptr) {
        return DBErrorTemplates::kBadAddress.Generate();
    }

    write_concern_ = mongoc_write_concern_new();
    mongoc_write_concern_set_w(write_concern_, MONGOC_WRITE_CONCERN_W_DEFAULT);
    mongoc_write_concern_set_journal(write_concern_, true);

    return nullptr;

}

Error MongoDBClient::UpdateCurrentCollectionIfNeeded(const std::string &collection_name) const {
    if (collection_name == current_collection_name_) {
        return nullptr;
    }
    if (current_collection_ != nullptr) {
        mongoc_collection_destroy(current_collection_);
    }

    auto encoded_name  = EncodeColName(collection_name);
    if (encoded_name.size() > maxCollectionNameLength) {
        return DBErrorTemplates::kWrongInput.Generate("stream name too long");
    }

    current_collection_ = mongoc_client_get_collection(client_, database_name_.c_str(),
                                                       encoded_name.c_str());
    current_collection_name_ = collection_name;
    mongoc_collection_set_write_concern(current_collection_, write_concern_);

    return nullptr;

}

Error MongoDBClient::TryConnectDatabase() {
    auto err = Ping();
    if (err == nullptr) {
        connected_ = true;
    }
    return err;
}

Error MongoDBClient::Connect(const std::string &address, const std::string &database_name) {
    if (connected_) {
        return DBErrorTemplates::kAlreadyConnected.Generate();
    }

    auto err = InitializeClient(address);
    if (err) {
        return err;
    }

    database_name_ = EncodeDbName(database_name);

    if (database_name_.size() > maxDbNameLength) {
        return DBErrorTemplates::kWrongInput.Generate("data source name too long");
    }

    err = TryConnectDatabase();
    if (err) {
        CleanUp();
    }
    return err;
}

std::string MongoDBClient::DBAddress(const std::string &address) const {
    return "mongodb://" + address + "/?appname=asapo";
}

void MongoDBClient::CleanUp() {
    if (write_concern_) {
        mongoc_write_concern_destroy(write_concern_);
    }
    if (current_collection_) {
        mongoc_collection_destroy(current_collection_);
    }
    if (client_) {
        mongoc_client_destroy(client_);
    }
}

bson_p PrepareBsonDocument(const MessageMeta &file, Error* err) {
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
    if (json == nullptr) {
        *err = TextError("empty metadata");
        return nullptr;
    }

    auto bson = bson_new_from_json(json, len, &mongo_err);
    if (!bson) {
        *err = DBErrorTemplates::kJsonParseError.Generate(mongo_err.message);
        return nullptr;
    }

    *err = nullptr;
    return bson_p{bson};
}

Error MongoDBClient::InsertBsonDocument(const bson_p &document, bool ignore_duplicates) const {
    bson_error_t mongo_err;
    if (!mongoc_collection_insert_one(current_collection_, document.get(), NULL, NULL, &mongo_err)) {
        if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            return ignore_duplicates ? nullptr : DBErrorTemplates::kDuplicateID.Generate();
        }
        return DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    return nullptr;
}

Error MongoDBClient::UpdateBsonDocument(uint64_t id, const bson_p &document, bool upsert) const {
    bson_error_t mongo_err;

    bson_t* opts = BCON_NEW ("upsert", BCON_BOOL(upsert));
    bson_t* selector = BCON_NEW ("_id", BCON_INT64(id));

    Error err = nullptr;

    if (!mongoc_collection_replace_one(current_collection_, selector, document.get(), opts, NULL, &mongo_err)) {
        err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    bson_free(opts);
    bson_free(selector);

    return err;
}

Error MongoDBClient::Insert(const std::string &collection, const MessageMeta &file, bool ignore_duplicates) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

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

Error MongoDBClient::Upsert(const std::string &collection, uint64_t id, const uint8_t* data, uint64_t size) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

    auto document = PrepareBsonDocument(data, (ssize_t) size, &err);
    if (err) {
        return err;
    }

    if (!BSON_APPEND_INT64(document.get(), "_id", id)) {
        err = DBErrorTemplates::kInsertError.Generate("cannot assign document id ");
    }

    return UpdateBsonDocument(id, document, true);

}

Error MongoDBClient::AddBsonDocumentToArray(bson_t* query, bson_t* update, bool ignore_duplicates) const {
    Error err;
    bson_error_t mongo_err;
// first update may fail due to multiple threads try to create document at once, the second one should succeed
// https://jira.mongodb.org/browse/SERVER-14322
    if (!mongoc_collection_update(current_collection_, MONGOC_UPDATE_UPSERT, query, update, NULL, &mongo_err)) {
        if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            if (!mongoc_collection_update(current_collection_, MONGOC_UPDATE_UPSERT, query, update, NULL, &mongo_err)) {
                if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
                    err = ignore_duplicates ? nullptr : DBErrorTemplates::kDuplicateID.Generate();
                } else {
                    err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
                }
            }
        } else {
            err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
        }
    }
    return err;
}

Error MongoDBClient::InsertAsDatasetMessage(const std::string &collection, const MessageMeta &file,
                                            uint64_t dataset_size,
                                            bool ignore_duplicates) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

    auto document = PrepareBsonDocument(file, &err);
    if (err) {
        return err;
    }
    auto query =
        BCON_NEW ("$and", "[", "{", "_id", BCON_INT64(file.id), "}", "{", "messages.dataset_substream", "{", "$ne",
                  BCON_INT64(file.dataset_substream), "}", "}", "]");
    auto update = BCON_NEW ("$setOnInsert", "{",
                            "size", BCON_INT64(dataset_size),
                            "timestamp", BCON_INT64((int64_t) NanosecsEpochFromTimePoint(file.timestamp)),
                            "}",
                            "$addToSet", "{",
                            "messages", BCON_DOCUMENT(document.get()), "}");

    err = AddBsonDocumentToArray(query, update, ignore_duplicates);

    bson_destroy(query);
    bson_destroy(update);

    return err;
}

Error MongoDBClient::GetRecordFromDb(const std::string &collection, uint64_t id, GetRecordMode mode,
                                     std::string* res) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

    bson_error_t mongo_err;
    bson_t* filter;
    bson_t* opts;
    mongoc_cursor_t* cursor;
    const bson_t* doc;
    char* str;

    switch (mode) {
        case GetRecordMode::kById:filter = BCON_NEW ("_id", BCON_INT64(id));
            opts = BCON_NEW ("limit", BCON_INT64(1));
            break;
        case GetRecordMode::kLast:filter = BCON_NEW (NULL);
            opts = BCON_NEW ("limit", BCON_INT64(1), "sort", "{", "_id", BCON_INT64(-1), "}");
            break;
        case GetRecordMode::kEarliest:filter = BCON_NEW (NULL);
            opts = BCON_NEW ("limit", BCON_INT64(1), "sort", "{", "timestamp", BCON_INT64(1), "}");
            break;
    }

    cursor = mongoc_collection_find_with_opts(current_collection_, filter, opts, NULL);

    bool found = false;
    while (mongoc_cursor_next(cursor, &doc)) {
        str = bson_as_relaxed_extended_json(doc, NULL);
        *res = str;
        found = true;
        bson_free(str);
    }

    if (mongoc_cursor_error(cursor, &mongo_err)) {
        err = DBErrorTemplates::kDBError.Generate(mongo_err.message);
    } else {
        if (!found) {
            err = DBErrorTemplates::kNoRecord.Generate();
        }
    }

    mongoc_cursor_destroy(cursor);
    bson_destroy(filter);
    bson_destroy(opts);

    return err;
}

Error MongoDBClient::GetById(const std::string &collection, uint64_t id, MessageMeta* file) const {
    std::string record_str;
    auto err = GetRecordFromDb(collection, id, GetRecordMode::kById, &record_str);
    if (err) {
        return err;
    }

    if (!file->SetFromJson(record_str)) {
        DBErrorTemplates::kJsonParseError.Generate(record_str);
    }
    return nullptr;
}

Error MongoDBClient::GetDataSetById(const std::string &collection,
                                    uint64_t id_in_set,
                                    uint64_t id,
                                    MessageMeta* file) const {
    std::string record_str;
    auto err = GetRecordFromDb(collection, id, GetRecordMode::kById, &record_str);
    if (err) {
        return err;
    }

    DataSet dataset;
    if (!dataset.SetFromJson(record_str)) {
        DBErrorTemplates::kJsonParseError.Generate(record_str);
    }

    for (const auto &message_meta : dataset.content) {
        if (message_meta.dataset_substream == id_in_set) {
            *file = message_meta;
            return nullptr;
        }
    }

    return DBErrorTemplates::kNoRecord.Generate();

}

Error UpdateStreamInfoFromEarliestRecord(const std::string &earliest_record_str,
                                         StreamInfo* info) {
    std::chrono::system_clock::time_point timestamp_created;
    auto parser = JsonStringParser(earliest_record_str);
    auto ok = TimeFromJson(parser, "timestamp", &timestamp_created);
    if (!ok) {
        return DBErrorTemplates::kJsonParseError.Generate(
            "UpdateStreamInfoFromEarliestRecord: cannot parse timestamp in response: " + earliest_record_str);
    }
    info->timestamp_created = timestamp_created;
    return nullptr;
}

Error UpdateFinishedStreamInfo(const std::string &metadata,
                               StreamInfo* info) {
    info->finished = true;
    auto parser = JsonStringParser(metadata);
    std::string next_stream;
    auto err = parser.GetString("next_stream", &next_stream);
    if (err) {
        return DBErrorTemplates::kJsonParseError.Generate(
            "UpdateFinishedStreamInfo: cannot parse finished stream meta response: " + metadata);
    }
    if (next_stream != kNoNextStreamKeyword) {
        info->next_stream = next_stream;
    }
    return nullptr;
}

Error UpdateFinishedInfo(const std::string &last_record_str, const JsonStringParser &parser, StreamInfo* info) {
    std::string name;
    parser.GetString("name", &name);
    if (name != kFinishStreamKeyword) {
        return nullptr;
    }
    std::string metadata;
    if (parser.Embedded("meta").GetRawString(&metadata) != nullptr) {
        return DBErrorTemplates::kJsonParseError.Generate(
            "UpdateStreamInfoFromLastRecord: cannot parse metadata in response: " + last_record_str);
    }
    return UpdateFinishedStreamInfo(metadata, info);
}

Error UpdateStreamInfoFromLastRecord(const std::string &last_record_str,
                                     StreamInfo* info) {
    auto parser = JsonStringParser(last_record_str);
    std::chrono::system_clock::time_point timestamp_last;
    uint64_t id;

    if (!TimeFromJson(parser, "timestamp", &timestamp_last)) {
        return DBErrorTemplates::kJsonParseError.Generate(
            "UpdateStreamInfoFromLastRecord: cannot parse timestamp in response: " + last_record_str);
    }
    if (parser.GetUInt64("_id", &id) != nullptr) {
        return DBErrorTemplates::kJsonParseError.Generate(
            "UpdateStreamInfoFromLastRecord: cannot parse _id in response: " + last_record_str);
    }

    info->timestamp_lastentry = timestamp_last;
    info->last_id = id;

    return UpdateFinishedInfo(last_record_str,parser,info);

}

Error StreamInfoFromDbResponse(const std::string &last_record_str,
                               const std::string &earliest_record_str,
                               StreamInfo* info) {
    std::chrono::system_clock::time_point timestamp_created;

    auto err = UpdateStreamInfoFromLastRecord(last_record_str, info);
    if (err) {
        return err;
    }

    return UpdateStreamInfoFromEarliestRecord(earliest_record_str, info);

}

Error MongoDBClient::GetEncodedStreamInfo(const std::string &collection_encoded, StreamInfo* info) const {
    std::string last_record_str, earliest_record_str;
    auto err = GetRecordFromDb(collection_encoded, 0, GetRecordMode::kLast, &last_record_str);
    if (err) {
        if (err
            == DBErrorTemplates::kNoRecord) { // with noRecord error it will return last_id = 0 which can be used to understand that the stream is not started yet
            *info = StreamInfo{};
            return nullptr;
        }
        return err;
    }
    err = GetRecordFromDb(collection_encoded, 0, GetRecordMode::kEarliest, &earliest_record_str);
    if (err) {
        return err;
    }

    return StreamInfoFromDbResponse(last_record_str, earliest_record_str, info);
}

Error MongoDBClient::GetStreamInfo(const std::string &collection, StreamInfo* info) const {
    std::string collection_encoded = EncodeColName(collection);
    return GetEncodedStreamInfo(collection_encoded,info);
}

bool MongoCollectionIsDataStream(const std::string &stream_name) {
    std::string prefix = std::string(kDBDataCollectionNamePrefix) + "_";
    return stream_name.rfind(prefix, 0) == 0;
}

Error MongoDBClient::UpdateCurrentLastStreamInfo(const std::string &collection_name, StreamInfo* info) const {
    StreamInfo next_info;
    auto err = GetEncodedStreamInfo(collection_name, &next_info);
    std::string prefix = std::string(kDBDataCollectionNamePrefix) + "_";
    if (err) {
        return err;
    }
    if (next_info.timestamp_created > info->timestamp_created) {
        next_info.name = collection_name.substr(prefix.size());
        *info = next_info;
    }
    return nullptr;
}

Error MongoDBClient::UpdateLastStreamInfo(const char* str, StreamInfo* info) const {
    std::string collection_name{str};
    if (MongoCollectionIsDataStream(collection_name)) {
        auto err = UpdateCurrentLastStreamInfo(collection_name, info);
        if (err) {
            return err;
        }
    }
    return nullptr;
}

Error MongoDBClient::GetLastStream(StreamInfo* info) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    mongoc_database_t* database;
    bson_t* opts;
    char** strv;
    bson_error_t error;

    database = mongoc_client_get_database(client_, database_name_.c_str());
    opts = BCON_NEW ("nameOnly", BCON_BOOL(true));
    auto zero_time = std::chrono::system_clock::from_time_t(0);
    info->timestamp_created = zero_time;
    info->timestamp_lastentry = zero_time;
    Error err;
    if ((strv = mongoc_database_get_collection_names_with_opts(
        database, opts, &error))) {
        for (auto i = 0; strv[i]; i++) {
            err = UpdateLastStreamInfo(strv[i], info);
            if (err) {
                break;
            }
        }
        bson_strfreev(strv);
    } else {
        err = DBErrorTemplates::kDBError.Generate(error.message);
    }

    if (err!= nullptr) {
        info->name = DecodeName(info->name);
    }

    bson_destroy(opts);
    mongoc_database_destroy(database);
    return err;
}

Error MongoDBClient::DeleteCollections(const std::string &prefix) const {
    mongoc_database_t* database;
    char** strv;
    bson_error_t error;
    std::string querystr = "^" + EscapeQuery(prefix);
    bson_t* query = BCON_NEW ("name", BCON_REGEX(querystr.c_str(), "i"));
    bson_t* opts = BCON_NEW ("nameOnly", BCON_BOOL(true), "filter", BCON_DOCUMENT(query));
    database = mongoc_client_get_database(client_, database_name_.c_str());
    Error err;
    if ((strv = mongoc_database_get_collection_names_with_opts(
        database, opts, &error))) {
        for (auto i = 0; strv[i]; i++) {
            DeleteCollection(strv[i]);
        }
        bson_strfreev(strv);
    } else {
        err = DBErrorTemplates::kDBError.Generate(error.message);
    }

    bson_destroy(opts);
    bson_destroy(query);
    mongoc_database_destroy(database);
    return nullptr;
}

Error MongoDBClient::DeleteCollection(const std::string &name) const {
    bson_error_t error;
    auto collection = mongoc_client_get_collection(client_, database_name_.c_str(), name.c_str());
    mongoc_collection_set_write_concern(collection, write_concern_);
    auto r = mongoc_collection_drop_with_opts(collection, NULL /* opts */, &error);
    mongoc_collection_destroy(collection);
    if (!r) {
        if (error.code == 26) {
            return DBErrorTemplates::kNoRecord.Generate("collection " + name + " not found in " + DecodeName(database_name_));
        } else {
            return DBErrorTemplates::kDBError.Generate(std::string(error.message) + ": " + std::to_string(error.code));
        }
    }
    return nullptr;
}

Error MongoDBClient::DeleteDocumentsInCollection(const std::string &collection_name,
                                                 const std::string &querystr) const {
    auto collection = mongoc_client_get_collection(client_, database_name_.c_str(), collection_name.c_str());
    mongoc_collection_set_write_concern(collection, write_concern_);
    bson_error_t error;
    auto query = BCON_NEW ("_id", BCON_REGEX(querystr.c_str(), "i"));
    if (!mongoc_collection_delete_many(collection, query, NULL, NULL, &error)) {
        return DBErrorTemplates::kDBError.Generate(error.message);
    }
    mongoc_collection_destroy(collection);
    bson_destroy(query);
    return nullptr;
}

Error MongoDBClient::DeleteStream(const std::string &stream) const {
    auto stream_encoded = EncodeColName(stream);
    std::string data_col = std::string(kDBDataCollectionNamePrefix) + "_" + stream_encoded;
    std::string inprocess_col = "inprocess_" + stream_encoded;
    std::string acks_col = "acks_" + stream_encoded;
    current_collection_name_ = "";
    auto err = DeleteCollection(data_col);
    if (err == nullptr) {
        DeleteCollections(inprocess_col);
        DeleteCollections(acks_col);
        std::string querystr = ".*_" + EscapeQuery(stream_encoded) + "$";
        DeleteDocumentsInCollection("current_location", querystr);
    }
    return err;
}

}
