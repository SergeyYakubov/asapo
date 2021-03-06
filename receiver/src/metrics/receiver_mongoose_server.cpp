#include "receiver_mongoose_server.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "../../../3d_party/mongoose/mongoose.h"
#include "../../../3d_party/mongoose/mongoose.c"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "../receiver_logger.h"
#include "asapo/common/io_error.h"

namespace asapo {

static void fn(struct mg_connection* c, int ev, void* ev_data, void* fn_data) {
    auto provider = static_cast<const ReceiverMetricsProvider*>(fn_data);
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*) ev_data;
        if (mg_http_match_uri(hm, "/metrics")) {
            mg_http_reply(c, 200, "", "%s", provider->Metrics().c_str());
        } else {
            mg_http_reply(c, 404, "", "%s", "Not found\n");
        }
    }
}

void asapo::ReceiverMongooseServer::ListenAndServe(std::string port,
        std::unique_ptr<ReceiverMetricsProvider> provider) {
    struct mg_mgr mgr;                            // Event manager
    mg_log_set("0");
    mg_mgr_init(&mgr);                            // Initialise event manager
    auto uri = "0.0.0.0:" + port;
    if (mg_http_listen(&mgr, uri.c_str(), fn, (void*) provider.get()) == NULL) {
        log__->Error("metrics server: cannot listen on port " + port);
        mg_mgr_free(&mgr);
        return;
    }
    for (;;) mg_mgr_poll(&mgr, 1000);                    // Infinite event loop
    mg_mgr_free(&mgr);
    return;
}

ReceiverMongooseServer::ReceiverMongooseServer() : log__{GetDefaultReceiverLogger()} {

}

}


