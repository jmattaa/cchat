#include "client.h"
#include "cchat_utils.h"
#include "logger.h"
#include <arpa/inet.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void cchat_startclient(int argc, char **argv)
{
    if (argc < 4)
        cchat_logger_fatal(1, "Usage: %s c <host> <port>\n", argv[0]);

#define host argv[2]
#define port argv[3]

    cchat_logger_log("Starting client...\n");
    cchat_logger_log("Connecting to %s:%s\n", host, port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cchat_logger_fatal(1, "Failed to create socket\n");

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(atoi(port));

    if (inet_pton(AF_INET, host, &serv.sin_addr) <= 0)
    {
        cchat_logger_error("Invalid address\n");
        goto cleanup;
    }

    if (connect(sockfd, (struct sockaddr *)&serv, sizeof(serv)) < 0)
    {
        cchat_logger_error("Failed to connect to server\n");
        goto cleanup;
    }

    send(sockfd, CCHAT_CONNECTED_MSG, strlen(CCHAT_CONNECTED_MSG), 0);

    cchat_logger_log("Connected to server\n");

    initscr();
    raw();
    noecho();

    refresh();

    uint8_t running = 1;
    while (running)
    {
        clear();
        mvprintw(0, 0, "Press 'ctrl+e' to exit");
        refresh();

        switch (getch())
        {
        case CCHAT_CTRL('e'):
            running = 0;
            break;
        }
    }

    endwin();

cleanup:
    send(sockfd, CCHAT_DISCONNECT_MSG, strlen(CCHAT_DISCONNECT_MSG), 0);
    if (sockfd < 0)
        close(sockfd);
#undef port
#undef host
}
