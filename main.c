/*
    crain
    Copyright (C) 2026 [Landon / mazylol]

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

struct Drop {
    int delay;
    int current;
    int active;
};

int rand_in_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

enum { ARG_CHARACTER,
       // ARG_NO_SPLASH,
       ARG_HELP };

struct Argument {
    int id;
    char *name;
    char *description;
    char alias;
};

struct Config {
    char character;
    int splash;
};

void help(struct Argument arguments[]) {
    printf("crain: the worst storm this side of the silicon\n\n");

    for (int i = 0; arguments[i].name != NULL; i++) {
        printf("--%s, -%c: %s\n", arguments[i].name, arguments[i].alias, arguments[i].description);
    }

    exit(0);
}

int parse_args(struct Config *config, struct Argument arguments[], int argc,
               char **argv) {
    for (int i = 1; i < argc; i++) {
        char *current = argv[i];
        int matched = 0;

        for (int j = 0; arguments[j].name != NULL; j++) {
            int is_short = (current[0] == '-' && current[1] == arguments[j].alias &&
                            current[2] == '\0');
            int is_long = (strncmp(current, "--", 2) == 0 &&
                           strcmp(current + 2, arguments[j].name) == 0);

            if (is_short || is_long) {
                matched = 1;

                switch (arguments[j].id) {
                case ARG_CHARACTER:
                    if (i + 1 < argc) {
                        config->character = argv[i + 1][0];
                        i++;
                    } else {
                        fprintf(stderr, "Error: --character requires a value\n");
                        return 1;
                    }
                    break;

                    // case ARG_NO_SPLASH:
                    //     config->splash = 0;
                    //     break;

                case ARG_HELP:
                    help(arguments);
                    return 0;
                }
                break;
            }
        }

        if (!matched) {
            printf("Unknown argument: %s\n", current);
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    struct Config config = {.character = '|', .splash = 1};

    struct Argument arguments[] = {
        {ARG_CHARACTER, "character", "set the character", 'c'},
        //{ARG_NO_SPLASH, "no-splash", "disable splash", 's'},
        {ARG_HELP, "help", "view help", 'h'},
        {0, NULL, NULL, 0}};

    if (argc > 1) {
        if (parse_args(&config, arguments, argc, argv) != 0) {
            fprintf(stderr, "Error: Failed to parse arguments\n");
            return 1;
        }
    }

    srand(time(NULL));

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, 1);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_BLUE, -1);
    }

    int x, y;
    getmaxyx(stdscr, y, x);

    struct Drop **array = malloc(y * sizeof(struct Drop *));
    for (int i = 0; i < y; i++) {
        array[i] = calloc(x, sizeof(struct Drop));
    }

    int spawn = 200;

    attron(COLOR_PAIR(1));

    while (1) {
        int ch = getch();
        if (ch == 'q')
            break;

        // gravity
        for (int j = y - 1; j >= 0; j--) {
            for (int i = 0; i < x; i++) {
                if (array[j][i].active == 0)
                    continue;

                // delay
                if (array[j][i].current < array[j][i].delay) {
                    array[j][i].current += 50;
                    continue;
                }
                array[j][i].current = 0;

                // move drop
                if (j == y - 1) {
                    array[j][i].active = 0; // hit floor
                } else {
                    array[j + 1][i] = array[j][i]; // move down
                    array[j][i].active = 0;        // clear old spot
                }
            }
        }

        // spawning
        if (spawn <= 0) {
            int drops_to_make = rand_in_range(1, 3);

            for (int k = 0; k < drops_to_make; k++) {
                int col = rand_in_range(0, x - 1);

                if (array[0][col].active == 0) {
                    array[0][col].active = 1;
                    array[0][col].delay = rand_in_range(100, 400); // random speed
                    array[0][col].current = 0;
                }
            }

            // wait between spawns (200ms to 1000ms)
            spawn = rand_in_range(200, 1000);
        } else {
            spawn -= 50;
        }

        erase();

        for (int j = 0; j < y; j++) {
            for (int i = 0; i < x; i++) {
                if (j == y - 1 && i == x - 1)
                    continue;

                if (array[j][i].active == 1) {
                    mvaddch(j, i, config.character);
                }
            }
        }

        refresh();
        napms(50);
    }

    attroff(COLOR_PAIR(1));
    endwin();
    for (int i = 0; i < y; i++) {
        free(array[i]);
    }
    free(array);

    return 0;
}
