
#pragma once

#include <event2/event.h>

namespace http {

class Worker {
public:
    int Init(int server_fd);
    void Run();
    void Stop();

    void Exit();

private:
    struct event_base *evbase_{nullptr};
    struct event *exit_ev_{nullptr};
    struct event *timer_event_{nullptr};

    int exit_pipes_[2];
};

}
