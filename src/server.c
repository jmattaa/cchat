#include "server.h"
#include "logger.h"
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *generate_pass_for_server();

void cchat_startserver(int argc, char **argv)
{
    if (argc < 3)
        cchat_logger_fatal(1, "Usage: %s s <port> [password]\n", argv[0]);

    uint8_t gen_password = (argc == 3);
    char *password =
        gen_password ? generate_pass_for_server() : strdup(argv[3]);

    cchat_logger_log("[Server Setup]\n");
    cchat_logger_log("Starting server on port: %s\n", argv[2]);
    cchat_logger_log("Password: \x1b[1m\x1b[4m%s\x1b[0m\n", password);
    cchat_logger_log("Press Enter to start server...\n");

    getchar();

    initscr();
    raw();
    noecho();

    clear();
    printw("\n\n");
    printw("Press 'p' to toggle password\n");
    printw("Press 'q' to quit server\n\n");
    printw("Port: %s\n", argv[2]);

    printw("Status: Listening for connections...\n");
    refresh();

    uint8_t server_running = 1;
    uint8_t password_visible = 0;

    while (server_running)
    {
        int ch = getch();
        switch (ch)
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
            server_running = 0;
            break;

        default:
            break;
        }
    }

    endwin();
    free(password);

    cchat_logger_log("[Server Shutdown] Server has been stopped.\n");
}

char *generate_pass_for_server()
{
#define PASS_LENGTH 32
    char *pass = malloc(PASS_LENGTH);

    srand(time(NULL));
    for (int i = 0; i < PASS_LENGTH - 1; i++)
        pass[i] = 'a' + rand() % 26;
    pass[PASS_LENGTH - 1] = '\0';

    return pass;
#undef PASS_LENGTH
}
