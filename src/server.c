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
    if (argc < 3)
        cchat_logger_fatal(1, "Usage: %s s <port>\n", argv[0]);

#define port argv[2]

    cchat_logger_log("[Server Setup]\n");
    cchat_logger_log("Starting server on port: %s\n", port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int client_sockfd = -1;
    if (sockfd < 0)
        cchat_logger_error("Failed to create socket\n");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(port));

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cchat_logger_error("Failed to bind socket\n");
        goto cleanup;
    }
    if (listen(sockfd, 1) < 0)
    {
        cchat_logger_error("Failed to listen on socket\n");
        goto cleanup;
    }

    cchat_logger_log("[Server Setup]: Waiting for connection...\n");

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    client_sockfd =
        accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sockfd < 0)
    {
        cchat_logger_error("Failed to accept connection\n");
        goto cleanup;
    }

    cchat_logger_log("[Server Setup] Connection accepted\n");

    char buf[1024];
    int bytes_read = read(client_sockfd, buf, sizeof(buf) - 1);
    if (bytes_read < 0)
    {
        cchat_logger_error("Failed to read from client\n");
        goto cleanup;
    }
    buf[bytes_read] = '\0';

    // I'm too proud of this shi cuz if you try to connect from like a browser
    // you'll get the invalid tingy and it's 🔥
    if (strncmp(buf, CCHAT_CONNECTED_MSG, strlen(CCHAT_CONNECTED_MSG)) != 0)
    {
        cchat_logger_error(CCHAT_INVALID_MSG "\n");
        send(client_sockfd, CCHAT_INVALID_MSG, strlen(CCHAT_INVALID_MSG), 0);
        goto cleanup;
    }

    // it's real easy to bypass this shi but it just makes sure that the
    // connection is from cchat
    // if you'd just take the binary and do strings you'd find the auth
    // msg
    cchat_logger_log("[Server Setup] Connection authenticated\n");

    initscr();
    raw();
    noecho();
    nodelay(stdscr, TRUE);

    refresh();

    fd_set read_fds;
    int max_fd = (client_sockfd > STDIN_FILENO) ? client_sockfd : STDIN_FILENO;
    struct timeval tv;

    uint8_t server_running = 1;
    while (server_running)
    {
        clear();
        mvprintw(0, 0, "Press 'ctrl+e' to quit cchat\n\n");
        refresh();

        FD_ZERO(&read_fds);
        FD_SET(client_sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);

        if (activity < 0)
        {
            cchat_logger_error("Select error\n");
            goto closewin;
        }

        if (FD_ISSET(client_sockfd, &read_fds))
        {
            bytes_read = read(client_sockfd, buf, sizeof(buf) - 1);
            if (bytes_read < 0)
            {
                cchat_logger_error("Failed to read from client\n");
                goto closewin;
            }
            else if (bytes_read > 0)
            {
                buf[bytes_read] = '\0';
                printw("%s\n", buf);

                if (strncmp(buf, CCHAT_DISCONNECT_MSG,
                            strlen(CCHAT_DISCONNECT_MSG)) == 0)
                {
                    server_running = 0;
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds))
        {
            switch (getch())
            {
            case CCHAT_CTRL('e'):
                server_running = 0;
                break;
            default:
                break;
            }
        }
    }
closewin:
    endwin();

cleanup:
    if (client_sockfd < 0)
        close(client_sockfd);
    if (sockfd < 0)
        close(sockfd);

    cchat_logger_log("[Server Shutdown] Server has been stopped.\n");

#undef port
}
