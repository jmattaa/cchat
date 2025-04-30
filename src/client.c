#include "client.h"
#include "cchat_utils.h"
#include "logger.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define QUIT_MSG "/quit"

static volatile sig_atomic_t keep_running = 1;

static void sigint_handler(int _) { keep_running = 0; }

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
    cchat_logger_log("Type '" QUIT_MSG "' to disconnect\n");

    signal(SIGINT, sigint_handler);

    fd_set fds;
    char buf[1024];
    char msg[1024];

    ssize_t n;

    uint8_t running = 1;
    while (running && keep_running)
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(sockfd, &fds);

        if (select(sockfd + 1, &fds, NULL, NULL, NULL) < 0)
            break;

        if (FD_ISSET(sockfd, &fds))
        {
            n = recv(sockfd, buf, sizeof(buf), 0);
            if (n < 0)
            {
                cchat_logger_error("Failed to receive message\n");
                running = 0;
                break;
            }
            if (n == 0)
            {
                cchat_logger_error("Server disconnected\n");
                running = 0;
                break;
            }

            buf[n] = '\0';

            printf("other: %s", buf);
        }

        if (FD_ISSET(STDIN_FILENO, &fds))
        {
            n = read(STDIN_FILENO, msg, sizeof(msg));
            if (n <= 0)
                break;

            if (strncmp(msg, QUIT_MSG, strlen(QUIT_MSG)) == 0)
                running = 0;

            if (msg[0] == '\n' || msg[0] == '\r')
                continue;
            msg[n] = '\0';
            if (send(sockfd, msg, strlen(msg), 0) < 0)
            {
                cchat_logger_error("Failed to send message\n");
                running = 0;
                break;
            }
        }
    }

cleanup:
    if (sockfd >= 0)
        close(sockfd);
#undef port
#undef host
}
