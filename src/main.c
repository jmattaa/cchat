#include "client.h"
#include "logger.h"
#include "server.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    cchat_logger_log("Starting cchat... \n", argc);
    if (argc >= 2 && argv[1][0] == 's')
    {
        cchat_startserver(argc, argv);
        return 0;
    }

    if (argc >= 2 && argv[1][0] == 'c')
    {
        cchat_startclient(argc, argv);
        return 0;
    }

    cchat_logger_error("To start a server, use:\n"
                       "\t\x1b[1m%s s <port>\x1b[0m\n",
                       argv[0]);

    cchat_logger_error("To join a server, use:\n"
                       "\t\x1b[1m%s c <host> <port>\x1b[0m\n",
                       argv[0]);
    return 1;
}
