
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

#include <event2/event.h>

#include "logging.h"
#include "daemonize.h"
#include "server.h"

int main(int argc, char **argv) {

    bool do_daemonize = false;
    if(do_daemonize) {
        if (sigignore(SIGHUP) == -1) {
            perror("Failed to ignore SIGHUP");
        }
        if (daemonize() == -1) {
            fprintf(stderr, "failed to daemon() in order to daemonize\n");
            exit(1);
        }
    }

    LOG_INFO<<"server starting ...";

    http::Server server{6666, 1024, 2};
    server.Run();

    return 0;
}

