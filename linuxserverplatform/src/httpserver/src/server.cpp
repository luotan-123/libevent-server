
#include "server.h"

#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#include "logging.h"

#define _MIN_(a, b) (a)>=(b)?(b):(a)
#define _MAX_(a, b) (a)>=(b)?(a):(b)

namespace http {

Server::Server(int32_t port, int32_t backlog, int32_t workers) : port_(port), backlog_(backlog), workers_n_(workers) {
}

Server::~Server() {
}

int Server::openServer() {
    int nfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfd < 0) {
        LOG_ERROR<<"socket error ";
        return -1;
    }
 
    int one = 1;
    int r = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(int));
 
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);
 
    r = bind(nfd, (struct sockaddr*)&addr, sizeof(addr));
    if (r < 0) {
        LOG_ERROR<<"bind error "<<port_;
        return -1;
    }
    r = listen(nfd, backlog_);
    if (r < 0) {
        LOG_ERROR<<"listen error";
        return -1;
    }
 
    int flags;
    if ((flags = fcntl(nfd, F_GETFL, 0)) < 0
        || fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG_ERROR<<"fcntl O_NONBLOCK error";
        return -1;
    }
 
    return nfd;
}
int Server::startWorkers() {
    workers_n_ = _MIN_(_MAX_(workers_n_, 1), 100);	// [1，100]
    for(int i=0; i<workers_n_; i++) {
        workers_[i].Init(fd_);
        worker_threads_.push_back(std::shared_ptr<std::thread>(new std::thread(&Worker::Run, &workers_[i])));
    }
    return 0;
}

int Server::waitWorkers() {
    for(int i=0; i<workers_n_; i++) {
        worker_threads_[i]->join();
        worker_threads_[i].reset();
    }
    return 0;
}

int Server::Run() {
    fd_ = openServer();
    if(fd_ == -1) {
        return -1;    
    }
    startWorkers();
    waitWorkers();
    return 0;
}

int Server::Stop() {
    for(int i=0; i<workers_n_; i++) {
        workers_[i].Stop();
    }
    return 0;
}

}
