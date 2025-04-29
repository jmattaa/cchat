#include "server.h"
#include "cchat_utils.h"
#include "logger.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static char *generate_pass_for_server();

void cchat_startserver(int argc, char **argv)
{
    if (argc < 3)
        cchat_logger_fatal(1, "Usage: %s s <port> [password]\n", argv[0]);

    uint8_t gen_password = (argc == 3);
    char *password =
        gen_password ? generate_pass_for_server() : strdup(argv[3]);

    cchat_logger_log("[Server Setup]\n");
    cchat_logger_log("Starting server on port: %s\n", argv[2]);

    initscr();
    raw();
    noecho();

    clear();
    printw("\n");
    printw("Press 'p' to toggle password\n");
    printw("Press 'q' to quit server\n\n");
    printw("Port: %s\n", argv[2]);

    printw("Waiting for connection...\n");
    refresh();

    uint8_t server_running = 1;
    uint8_t password_visible = 0;
    uint8_t connected = 0;

    while (!connected)
    {
        switch (getch())
        {
        case 'p':
            if (!password_visible)
            {
                move(8, 0);
                clrtoeol();
                printw("Password: %s", password);
                password_visible = 1;
                refresh();
            }
            else
            {
                move(8, 0);
                clrtoeol();
                printw("Password: ***");
                password_visible = 0;
                refresh();
            }
            break;

        case 'q':
            connected = 1; // stop this loop
            server_running = 0; // skip next loop
            break; 

        default:
            break;
        }
    }

    while (server_running)
    {
        switch (getch())
        {
        case CCHAT_CTRL('e'):
            server_running = 0;
            break;
        }

        clear();
        printw("Press 'ctrl+e to quit cchat\n\n");
        refresh();
    }

    endwin();
    free(password);

    cchat_logger_log("[Server Shutdown] Server has been stopped.\n");
}

static char *generate_pass_for_server()
{
#define PASS_LENGTH 32
    char *pass = malloc(PASS_LENGTH);

    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    srand(ts.tv_nsec);
    for (int i = 0; i < PASS_LENGTH - 1; i++)
        pass[i] = 'a' + rand() % 26;
    pass[PASS_LENGTH - 1] = '\0';

    return pass;
#undef PASS_LENGTH
}
