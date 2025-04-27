#include "client.h"
#include "logger.h"

void cchat_startclient(int argc, char **argv)
{
    if (argc < 3)
        cchat_logger_fatal(1, "Usage: %s c <port> <password>\n", argv[0]);
}
