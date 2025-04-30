#include "server.h"
#include "cchat_utils.h"
#include "logger.h"
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
    int portnum = atoi(port);
    if (portnum <= 0 || portnum > 65535)
        cchat_logger_fatal(1, "Invalid port number: %s\n", port);

    cchat_logger_log("[Server Setup]\n");
    cchat_logger_log("Starting server on port: %d\n", portnum);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int client_sockfd = -1;
    if (sockfd < 0)
        cchat_logger_error("Failed to create socket\n");

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(portnum);

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
    int n = read(client_sockfd, buf, sizeof(buf) - 1);
    if (n < 0)
    {
        cchat_logger_error("Failed to read from client\n");
        goto cleanup;
    }
    buf[n] = '\0';

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

    fd_set fds;
    char msg[1024];

    uint8_t server_running = 1;
    while (server_running)
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(client_sockfd, &fds);

        if (select(client_sockfd + 1, &fds, NULL, NULL, NULL) < 0)
            break;

        if (FD_ISSET(client_sockfd, &fds))
        {
            buf[0] = '\0';
            n = read(client_sockfd, buf, sizeof(buf) - 1);
            if (n < 0)
            {
                cchat_logger_error("Failed to receive message\n");
                server_running = 0;
                break;
            }
            if (n == 0)
            {
                cchat_logger_error("Client disconnected\n");
                server_running = 0;
                break;
            }

            buf[n] = '\0';

            printf("other: %s", buf);
        }

        if (FD_ISSET(STDIN_FILENO, &fds))
        {
            n = read(STDIN_FILENO, msg, sizeof(msg) - 1);
            if (n <= 0)
                break;

            if (msg[0] == '\n' || msg[0] == '\r')
                continue;

            msg[n] = '\0';
            if (send(client_sockfd, msg, n, 0) < 0)
            {
                cchat_logger_error("Failed to send message\n");
                server_running = 0;
                break;
            }
        }
    }

cleanup:
    if (client_sockfd >= 0)
        close(client_sockfd);
    if (sockfd >= 0)
        close(sockfd);

    cchat_logger_log("[Server Shutdown] Server has been stopped.\n");

#undef port
}
