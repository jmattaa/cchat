#include "server.h"
#include "cchat_utils.h"
#include "logger.h"
#include <ncurses.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

void cchat_startserver(int argc, char **argv)
{
    if (argc < 2)
        cchat_logger_fatal(1, "Usage: %s s <port>\n", argv[0]);

#define port argv[2]

    cchat_logger_log("[Server Setup]\n");
    cchat_logger_log("Starting server on port: %s\n", port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        cchat_logger_fatal(1, "Failed to create socket\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        cchat_logger_fatal(1, "Failed to bind socket\n");
    if (listen(sockfd, 1) < 0)
        cchat_logger_fatal(1, "Failed to listen on socket\n");

    cchat_logger_log("[Server Setup]: Waiting for connection...\n");

    int client_sockfd;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    client_sockfd =
        accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sockfd < 0)
        cchat_logger_fatal(1, "Failed to accept connection\n");

    cchat_logger_log("[Server Setup] Connection accepted\n");

    initscr();
    raw();
    noecho();

    refresh();

    uint8_t server_running = 1;

    while (server_running)
    {
        clear();
        printw("Press 'ctrl+e to quit cchat\n\n");
        refresh();

        switch (getch())
        {
        case CCHAT_CTRL('e'):
            server_running = 0;
            break;
        }
    }

    close(sockfd);
    endwin();

    cchat_logger_log("[Server Shutdown] Server has been stopped.\n");

#undef port
}
