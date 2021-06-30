#include "asapo/json_parser/json_parser.h"
#include "mongodb_client.h"
#include "encoding.h"

#include <chrono>
#include <regex>

#include "asapo/database/db_error.h"
#include "asapo/common/data_structs.h"

#include "asapo/common/internal/version.h"

namespace asapo {

using asapo::Database;

void
my_logger (mongoc_log_level_t log_level,
           const char* log_domain,
           const char* message,
           void* user_data) {
    /* smaller values are more important */
    if (log_level < MONGOC_LOG_LEVEL_CRITICAL) {
        mongoc_log_default_handler (log_level, log_domain, message, user_data);
    }
}

MongoDbInstance::MongoDbInstance() {
    mongoc_log_set_handler (my_logger, NULL);
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

Error MongoDBClient::InitializeClient(const std::string& address) {
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

Error MongoDBClient::UpdateCurrentCollectionIfNeeded(const std::string& collection_name) const {
    if (collection_name == current_collection_name_) {
        return nullptr;
    }
    if (current_collection_ != nullptr) {
        mongoc_collection_destroy(current_collection_);
    }

    auto encoded_name = EncodeColName(collection_name);
    if (encoded_name.size() > maxCollectionNameLength) {
        return DBErrorTemplates::kWrongInput.Generate("collection name too long");
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

Error MongoDBClient::Connect(const std::string& address, const std::string& database_name) {
    if (connected_) {
        return DBErrorTemplates::kAlreadyConnected.Generate();
    }

    auto err = InitializeClient(address);
    if (err) {
        CleanUp();
        return err;
    }

    database_name_ = EncodeDbName(database_name);

    if (database_name_.size() > maxDbNameLength) {
        CleanUp();
        return DBErrorTemplates::kWrongInput.Generate("data source name too long");
    }

    err = TryConnectDatabase();
    if (err) {
        CleanUp();
    }
    return err;
}

std::string MongoDBClient::DBAddress(const std::string& address) const {
    return "mongodb://" + address + "/?appname=asapo";
}

void MongoDBClient::CleanUp() {
    if (write_concern_) {
        mongoc_write_concern_destroy(write_concern_);
        write_concern_ = nullptr;
    }
    if (current_collection_) {
        mongoc_collection_destroy(current_collection_);
        current_collection_ = nullptr;
    }
    if (client_) {
        mongoc_client_destroy(client_);
        client_ = nullptr;
    }
}

bson_p PrepareBsonDocument(const MessageMeta& file, Error* err) {
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

bson_p PrepareUpdateDocument(const uint8_t* json, Error* err) {
    JsonStringParser parser{std::string(reinterpret_cast<const char*>(json))};
    std::string json_flat;
    auto parser_err = parser.GetFlattenedString("meta", ".", &json_flat);
    if (parser_err) {
        *err = DBErrorTemplates::kJsonParseError.Generate("cannof flatten meta " + parser_err->Explain());
        return nullptr;
    }
    bson_error_t mongo_err;
    auto bson_meta =
        bson_new_from_json(reinterpret_cast<const uint8_t*>(json_flat.c_str()), static_cast<ssize_t>(json_flat.size()),
                           &mongo_err);
    if (!bson_meta) {
        *err = DBErrorTemplates::kJsonParseError.Generate(mongo_err.message);
        return nullptr;
    }
    return bson_p{bson_meta};
}

bson_p PrepareInjestDocument(const uint8_t* json, ssize_t len, Error* err) {
    bson_error_t mongo_err;
    if (json == nullptr) {
        *err = TextError("empty metadata");
        return nullptr;
    }

    auto bson_meta = bson_new_from_json(json, len, &mongo_err);
    if (!bson_meta) {
        *err = DBErrorTemplates::kJsonParseError.Generate(mongo_err.message);
        return nullptr;
    }
    auto bson = bson_new();
    if (!BSON_APPEND_DOCUMENT(bson, "meta", bson_meta)
            || !BSON_APPEND_UTF8(bson, "schema_version", GetDbSchemaVersion().c_str())) {
        *err = DBErrorTemplates::kInsertError.Generate("cannot add schema version ");
        bson_destroy(bson_meta);
        return nullptr;
    }

    *err = nullptr;
    bson_destroy(bson_meta);
    return bson_p{bson};
}

Error MongoDBClient::InsertBsonDocument(const bson_p& document, bool ignore_duplicates) const {
    bson_error_t mongo_err;
    bson_p insert_opts{bson_new ()};

    if (current_session_) {
        if (!mongoc_client_session_append (current_session_, insert_opts.get(), &mongo_err)) {
            return DBErrorTemplates::kInsertError.Generate(std::string("Could not add session to opts: ") + mongo_err.message);
        }
    }

    if (!mongoc_collection_insert_one(current_collection_, document.get(), insert_opts.get(), NULL, &mongo_err)) {
        if (mongo_err.code == MONGOC_ERROR_DUPLICATE_KEY) {
            return ignore_duplicates ? nullptr : DBErrorTemplates::kDuplicateID.Generate();
        }
        return DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }
    return nullptr;
}

bool documentWasChanged(bson_t* reply) {
    bson_iter_t iter;
    bson_iter_init_find(&iter, reply, "upsertedCount");
    auto n_upsert = bson_iter_int32(&iter);
    bson_iter_init_find(&iter, reply, "modifiedCount");
    auto n_mod = bson_iter_int32(&iter);
    bson_iter_init_find(&iter, reply, "matchedCount");
    auto n_matched = bson_iter_int32(&iter);
    return n_mod + n_upsert + n_matched > 0;
}

Error MongoDBClient::ReplaceBsonDocument(const std::string& id, const bson_p& document, bool upsert) const {
    bson_error_t mongo_err;

    bson_t* opts = BCON_NEW ("upsert", BCON_BOOL(upsert));
    bson_t* selector = BCON_NEW ("_id", BCON_UTF8(id.c_str()));
    bson_t reply;
    Error err = nullptr;

    if (!mongoc_collection_replace_one(current_collection_, selector, document.get(), opts, &reply, &mongo_err)) {
        err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    if (err == nullptr && !documentWasChanged(&reply)) {
        err = DBErrorTemplates::kWrongInput.Generate("cannot replace: metadata does not exist");
    }

    bson_free(opts);
    bson_free(selector);
    bson_destroy(&reply);

    return err;
}

Error MongoDBClient::UpdateBsonDocument(const std::string& id, const bson_p& document, bool upsert) const {
    bson_error_t mongo_err;

    bson_t* opts = BCON_NEW ("upsert", BCON_BOOL(upsert));
    bson_t* selector = BCON_NEW ("_id", BCON_UTF8(id.c_str()));
    bson_t* update = BCON_NEW ("$set", BCON_DOCUMENT(document.get()));

    bson_t reply;
    Error err = nullptr;
    if (!mongoc_collection_update_one(current_collection_, selector, update, opts, &reply, &mongo_err)) {
        err = DBErrorTemplates::kInsertError.Generate(mongo_err.message);
    }

    if (err == nullptr && !documentWasChanged(&reply)) {
        err = DBErrorTemplates::kWrongInput.Generate("cannot update: metadata does not exist");
    }

    bson_free(opts);
    bson_free(selector);
    bson_destroy(&reply);
    bson_destroy(update);

    return err;
}

Error MongoDBClient::GetNextId(const std::string& stream, uint64_t* id) const {
    mongoc_find_and_modify_opts_t* opts;
    std::string  collection_name = "auto_id_counters";
    auto collection = mongoc_client_get_collection(client_, database_name_.c_str(), collection_name.c_str());
    mongoc_collection_set_write_concern(collection, write_concern_);

    bson_t reply;
    bson_error_t error;
    bson_t query = BSON_INITIALIZER;
    bson_t* update;
    bool success;
    BSON_APPEND_UTF8 (&query, "_id", stream.c_str());
    update = BCON_NEW ("$inc", "{", "curIndex", BCON_INT64(1), "}");
    opts = mongoc_find_and_modify_opts_new ();
    mongoc_find_and_modify_opts_set_update (opts, update);
    if (current_session_) {
        bson_p extra_opts{bson_new()};
        if (!mongoc_client_session_append(current_session_, extra_opts.get(), &error)) {
            return DBErrorTemplates::kInsertError.Generate(
                       std::string("Could not add session to opts: ") + error.message);
        }
        success = mongoc_find_and_modify_opts_append(opts, extra_opts.get());
        if (!success) {
            return DBErrorTemplates::kInsertError.Generate(std::string(
                    "mongoc_find_and_modify_opts_append: cannot append options"));
        }
    }
    mongoc_find_and_modify_opts_set_flags(opts,
                                          mongoc_find_and_modify_flags_t(MONGOC_FIND_AND_MODIFY_UPSERT | MONGOC_FIND_AND_MODIFY_RETURN_NEW));
    success = mongoc_collection_find_and_modify_with_opts (
                  collection, &query, opts, &reply, &error);
    Error err;
    if (success) {
        bson_iter_t iter;
        bson_iter_t iter_idx;
        auto found = bson_iter_init(&iter, &reply) && bson_iter_find_descendant(&iter, "value.curIndex", &iter_idx)
                     && BSON_ITER_HOLDS_INT64(&iter_idx);
        if (found) {
            *id = static_cast<uint64_t>(bson_iter_int64(&iter_idx));
        } else {
            err = DBErrorTemplates::kInsertError.Generate(std::string("cannot extract auto id"));
        }
    } else {
//        auto str = bson_as_relaxed_extended_json(&reply, NULL);
//        printf("%s",str);
//        bson_free(str);

        err = DBErrorTemplates::kInsertError.Generate(std::string("cannot get auto id:") + error.message );
    }
    bson_destroy (&reply);
    bson_destroy (update);
    bson_destroy (&query);
    mongoc_find_and_modify_opts_destroy (opts);
    mongoc_collection_destroy(collection);
    return err;
}

Error MongoDBClient::InsertWithAutoId(const MessageMeta& file,
                                      uint64_t* id_inserted) const {
    uint64_t id;
    auto err = GetNextId(current_collection_name_, &id);
    if (err != nullptr) {
        return err;
    }

    auto meta_new = file;
    meta_new.id = id;
    return Insert(current_collection_name_, meta_new, false, id_inserted);
}

Error MongoDBClient::Insert(const std::string& collection, const MessageMeta& file, bool ignore_duplicates,
                            uint64_t* id_inserted) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

    if (file.id == 0) {
        return InsertWithAutoId(file, id_inserted);
    }

    auto document = PrepareBsonDocument(file, &err);
    if (err) {
        return err;
    }

    err = InsertBsonDocument(document, ignore_duplicates);
    if (!err && id_inserted) {
        *id_inserted = file.id;
    }
    return err;
}

MongoDBClient::~MongoDBClient() {
    CleanUp();
}

bson_p PrepareBsonDocument(const uint8_t* json, ssize_t len, const std::string& id_encoded, MetaIngestMode mode,
                           Error* err) {
    bson_p document;
    if (mode.op == MetaIngestOp::kUpdate) {
        document = PrepareUpdateDocument(json, err);
    } else {
        document = PrepareInjestDocument(json, len, err);
    }
    if (*err) {
        return nullptr;
    }
    if (mode.op != MetaIngestOp::kUpdate) {
        if (!BSON_APPEND_UTF8(document.get(), "_id", id_encoded.c_str())) {
            *err = DBErrorTemplates::kInsertError.Generate("cannot assign document id ");
            return nullptr;
        }
    }
    return document;
}

Error MongoDBClient::InsertMeta(const std::string& collection, const std::string& id, const uint8_t* data,
                                uint64_t size,
                                MetaIngestMode mode) const {
    if (!connected_) {
        return DBErrorTemplates::kNotConnected.Generate();
    }

    auto err = UpdateCurrentCollectionIfNeeded(collection);
    if (err) {
        return err;
    }

    auto id_encoded = EncodeColName(id);
    auto document = PrepareBsonDocument(data, (ssize_t) size, id_encoded, mode, &err);
    if (err) {
        return err;
    }

    switch (mode.op) {
    case MetaIngestOp::kInsert:
        return InsertBsonDocument(document, false);
    case asapo::MetaIngestOp::kReplace:
        return ReplaceBsonDocument(id_encoded, document, mode.upsert);
    case MetaIngestOp::kUpdate:
        return UpdateBsonDocument(id_encoded, document, mode.upsert);
        break;
    }
    return DBErrorTemplates::kWrongInput.Generate("unknown op");
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

Error MongoDBClient::InsertAsDatasetMessage(const std::string& collection, const MessageMeta& file,
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
        BCON_NEW ("$and", "[", "{", "_id", BCON_INT64(static_cast<int64_t>(file.id)), "}", "{", "messages.dataset_substream",
                  "{", "$ne",
                  BCON_INT64(static_cast<int64_t>(file.dataset_substream)), "}", "}", "]");
    auto update = BCON_NEW ("$setOnInsert", "{",
                            "size", BCON_INT64(static_cast<int64_t>(dataset_size)),
                            "schema_version", GetDbSchemaVersion().c_str(),
                            "timestamp", BCON_INT64((int64_t) NanosecsEpochFromTimePoint(file.timestamp)),
                            "}",
                            "$addToSet", "{",
                            "messages", BCON_DOCUMENT(document.get()), "}");

    err = AddBsonDocumentToArray(query, update, ignore_duplicates);

    bson_destroy(query);
    bson_destroy(update);

    return err;
}

Error MongoDBClient::GetRecordFromDb(const std::string& collection, uint64_t id, const std::string& string_id,
                                     GetRecordMode mode,
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
    case GetRecordMode::kByStringId:
        filter = BCON_NEW ("_id", BCON_UTF8(string_id.c_str()));
        opts = BCON_NEW ("limit", BCON_INT64(1));
        break;
    case GetRecordMode::kById:
        filter = BCON_NEW ("_id", BCON_INT64(static_cast<int64_t>(id)));
        opts = BCON_NEW ("limit", BCON_INT64(1));
        break;
    case GetRecordMode::kLast:
        filter = BCON_NEW (NULL);
        opts = BCON_NEW ("limit", BCON_INT64(1), "sort", "{", "_id", BCON_INT64(-1), "}");
        break;
    case GetRecordMode::kEarliest:
        filter = BCON_NEW (NULL);
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

Error MongoDBClient::GetById(const std::string& collection, uint64_t id, MessageMeta* file) const {
    std::string record_str;
    auto err = GetRecordFromDb(collection, id, "", GetRecordMode::kById, &record_str);
    if (err) {
        return err;
    }

    if (!file->SetFromJson(record_str)) {
        DBErrorTemplates::kJsonParseError.Generate(record_str);
    }
    return nullptr;
}

Error MongoDBClient::GetDataSetById(const std::string& collection,
                                    uint64_t id_in_set,
                                    uint64_t id,
                                    MessageMeta* file) const {
    std::string record_str;
    auto err = GetRecordFromDb(collection, id, "", GetRecordMode::kById, &record_str);
    if (err) {
        return err;
    }

    DataSet dataset;
    if (!dataset.SetFromJson(record_str)) {
        DBErrorTemplates::kJsonParseError.Generate(record_str);
    }

    for (const auto& message_meta : dataset.content) {
        if (message_meta.dataset_substream == id_in_set) {
            *file = message_meta;
            return nullptr;
        }
    }

    return DBErrorTemplates::kNoRecord.Generate();

}

Error UpdateStreamInfoFromEarliestRecord(const std::string& earliest_record_str,
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

Error UpdateFinishedStreamInfo(const std::string& metadata,
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

Error UpdateFinishedInfo(const std::string& last_record_str, const JsonStringParser& parser, StreamInfo* info) {
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

Error UpdateStreamInfoFromLastRecord(const std::string& last_record_str,
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

    return UpdateFinishedInfo(last_record_str, parser, info);

}

Error StreamInfoFromDbResponse(const std::string& last_record_str,
                               const std::string& earliest_record_str,
                               StreamInfo* info) {
    std::chrono::system_clock::time_point timestamp_created;

    auto err = UpdateStreamInfoFromLastRecord(last_record_str, info);
    if (err) {
        return err;
    }

    return UpdateStreamInfoFromEarliestRecord(earliest_record_str, info);

}

Error MongoDBClient::GetStreamInfo(const std::string& collection, StreamInfo* info) const {
    std::string last_record_str, earliest_record_str;
    auto err = GetRecordFromDb(collection, 0, "", GetRecordMode::kLast, &last_record_str);
    if (err) {
        if (err
                == DBErrorTemplates::kNoRecord) { // with noRecord error it will return last_id = 0 which can be used to understand that the stream is not started yet
            *info = StreamInfo{};
            return nullptr;
        }
        return err;
    }
    err = GetRecordFromDb(collection, 0, "", GetRecordMode::kEarliest, &earliest_record_str);
    if (err) {
        return err;
    }

    return StreamInfoFromDbResponse(last_record_str, earliest_record_str, info);
}

bool MongoCollectionIsDataStream(const std::string& stream_name) {
    std::string prefix = std::string(kDBDataCollectionNamePrefix) + "_";
    return stream_name.rfind(prefix, 0) == 0;
}

Error MongoDBClient::UpdateLastStreamInfo(const char* str, StreamInfo* info) const {
    auto collection_name = DecodeName(str);
    if (!MongoCollectionIsDataStream(collection_name)) {
        return nullptr;
    }
    StreamInfo next_info;
    auto err = GetStreamInfo(collection_name, &next_info);
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

    if (err != nullptr) {
        info->name = DecodeName(info->name);
    }

    bson_destroy(opts);
    mongoc_database_destroy(database);
    return err;
}

Error MongoDBClient::DeleteCollections(const std::string& prefix) const {
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

Error MongoDBClient::DeleteCollection(const std::string& name) const {
    bson_error_t error;
    auto collection = mongoc_client_get_collection(client_, database_name_.c_str(), name.c_str());
    mongoc_collection_set_write_concern(collection, write_concern_);
    auto r = mongoc_collection_drop_with_opts(collection, NULL /* opts */, &error);
    mongoc_collection_destroy(collection);
    if (!r) {
        if (error.code == 26) {
            return DBErrorTemplates::kNoRecord.Generate(
                       "collection " + name + " not found in " + DecodeName(database_name_));
        } else {
            return DBErrorTemplates::kDBError.Generate(std::string(error.message) + ": " + std::to_string(error.code));
        }
    }
    return nullptr;
}

Error MongoDBClient::DeleteDocumentsInCollection(const std::string& collection_name,
                                                 const std::string& querystr) const {
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

Error MongoDBClient::DeleteStream(const std::string& stream) const {
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
    DeleteDocumentsInCollection("meta", "^" + EscapeQuery(stream_encoded) + "$");
    return err;
}

Error MongoDBClient::GetMetaFromDb(const std::string& collection, const std::string& id, std::string* res) const {
    std::string meta_str;
    auto err = GetRecordFromDb(collection, 0, EncodeColName(id), GetRecordMode::kByStringId, &meta_str);
    if (err) {
        return err;
    }
    auto parser = JsonStringParser(meta_str);
    err = parser.Embedded("meta").GetRawString(res);
    if (err) {
        return DBErrorTemplates::kJsonParseError.Generate(
                   "GetMetaFromDb: cannot parse database response: " + err->Explain());
    }
    return nullptr;
}

}
