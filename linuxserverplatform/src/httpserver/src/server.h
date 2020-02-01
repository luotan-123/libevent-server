
#pragma once

#include <vector>
#include <thread>

#include "worker.h"

namespace http {

class Server {
public:
    Server(int32_t port=6666, int32_t backlog=1024, int32_t workers=1);
    virtual ~Server();

    int Run();
    int Stop();

protected:
    int openServer();
    int startWorkers();
    int waitWorkers();

private:
    int fd_{-1};
    int32_t port_{6666};
    int32_t backlog_{1024};
    int32_t workers_n_{10};
    Worker workers_[100];       // TODO
    std::vector<std::shared_ptr<std::thread>> worker_threads_;
};

}
