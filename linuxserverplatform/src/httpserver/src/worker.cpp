
#include "worker.h"

#include <unistd.h>

#include "handlers.h"
#include "logging.h"

namespace http {

static void exit_cb_handler(int fd, short events, void* arg) {
    Worker* w = (Worker*)arg;
    char buf[1];
    if(read(fd, buf, 1) != 1) {
        LOG_ERROR<<"read error";
        return;
    }
    w->Exit();
}

int Worker::Init(int server_fd) {

    evbase_ = event_base_new();
    if (evbase_ == NULL) 
        return -1;

    if (pipe(exit_pipes_)) {
        return -1;
    }

    // HTTP handlers
    struct evhttp *http = evhttp_new(evbase_);
    if (http == NULL) return -1;
    int r = evhttp_accept_socket(http, server_fd);
    if (r != 0) return -1;
    for(request_handler_t* h=request_handlers; h && h->path; h++) {
        evhttp_set_cb(http, h->path, h->handler, h->arg);
    }
    /* We want to accept arbitrary requests, so we need to set a "generic"
     * cb.  We can also add callbacks for specific paths. */
    evhttp_set_gencb(http, general_request_handler, NULL);
 
    //ev_result_ = event_new(evbase_, pipes_[0], EV_READ|EV_PERSIST, result_cb_handler, (void*)this);
    //event_add(ev_result_, NULL);

    exit_ev_ = event_new(evbase_, exit_pipes_[0], EV_READ|EV_PERSIST, exit_cb_handler, (void*)this);
    event_add(exit_ev_, NULL);

    return 0;
}

void Worker::Run() {
    if(evbase_ == nullptr)
        return;
    
    event_base_dispatch(evbase_);
    event_free(exit_ev_);
    event_base_free(evbase_);
}

void Worker::Stop() {
    char buf[1] = {'-'};
    write(exit_pipes_[1], buf, 1);
}
void Worker::Exit() {
    if(evbase_) {
        event_base_loopexit(evbase_, NULL);
        //event_base_loopbreak(evbase_);
    }
}

}
