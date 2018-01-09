#include "mongoc.h"


#include "database/mongodb_client.h"

namespace hidra2 {

DBError MongoDB::Connect(const std::string& address, const std::string& database_name,
                         const std::string& collection_name ) {
    const char* uri_str = address.c_str();
    mongoc_client_t* client;
    mongoc_database_t* database;
    mongoc_collection_t* collection;
    bson_t* command, reply, *insert;
    bson_error_t error;
    char* str;
    bool retval;

    /*
     * Required to initialize libmongoc's internals
     */
    mongoc_init ();

    /*
     * Create a new client instance
     */
    client = mongoc_client_new (uri_str);

    /*
     * Register the application name so we can track it in the profile logs
     * on the server. This can also be done from the URI (see other examples).
     */
    mongoc_client_set_appname (client, "connect-example");

    /*
     * Get a handle on the database "db_name" and collection "coll_name"
     */
    database = mongoc_client_get_database (client, database_name.c_str());
    collection = mongoc_client_get_collection (client, database_name.c_str(), collection_name.c_str());

    /*
     * Do work. This example pings the database, prints the result as JSON and
     * performs an insert
     */
    command = BCON_NEW ("ping", BCON_INT32 (1));

    retval = mongoc_client_command_simple (
                 client, "admin", command, NULL, &reply, &error);

    if (!retval) {
        fprintf (stderr, "%s\n", error.message);
        return DBError::kConnectionError;
    }

    str = bson_as_json (&reply, NULL);
    printf ("%s\n", str);

    insert = BCON_NEW ("hello", BCON_UTF8 ("world"));

    if (!mongoc_collection_insert_one (collection, insert, NULL, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
    }

    bson_destroy (insert);
    bson_destroy (&reply);
    bson_destroy (command);
    bson_free (str);

    /*
     * Release our handles and clean up libmongoc
     */
    mongoc_collection_destroy (collection);
    mongoc_database_destroy (database);
    mongoc_client_destroy (client);
    mongoc_cleanup ();

    return DBError::kNoError;
}

DBError MongoDB::Import(const FileInfos& files) const {
    return DBError::kNoError;
}


MongoDB::~MongoDB() {

}



}
