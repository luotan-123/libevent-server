
#pragma once

#include <event2/http.h>
#include <event2/http_struct.h>

void general_request_handler(struct evhttp_request *req, void *arg);
void ping_handler(struct evhttp_request *req, void *args);
void post_handler(struct evhttp_request *req, void *args);

typedef void (*_request_handler_)(struct evhttp_request *req, void *arg);
struct request_handler_t {
    const char* path;
    _request_handler_ handler;
    void* arg;
};

static request_handler_t request_handlers [] = {
    { "/ping", ping_handler, NULL },
    { "/post", post_handler, NULL },
    // ... more uri handlers ...
    { NULL, NULL, NULL},
};


