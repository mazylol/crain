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

#define COLOR_BOLD "\e[1m"
#define COLOR_OFF "\e[m"

struct Drop {
    int delay;
    int current;
    int active;
};

int rand_in_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

enum { ARG_CHARACTER,
       ARG_COLOR,
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
    int color;
};

struct StringAlias {
    char *key;
    int value;
};

struct StringAlias color_aliases[] = {
    // blue
    {"blue", COLOR_BLUE},
    {"b", COLOR_BLUE},
    // red
    {"red", COLOR_RED},
    {"r", COLOR_RED},
    // green
    {"green", COLOR_GREEN},
    {"g", COLOR_GREEN},
    // yellow
    {"yellow", COLOR_YELLOW},
    {"y", COLOR_YELLOW},
    // white
    {"white", COLOR_WHITE},
    {"w", COLOR_WHITE},
    {NULL, 0}};

int get_color_from_alias(const char *input) {
    for (int i = 0; color_aliases[i].key != NULL; i++) {
        if (strcmp(input, color_aliases[i].key) == 0) {
            return color_aliases[i].value;
        }
    }
    return -1;
}

void help(struct Argument arguments[]) {
    printf("crain: the worst storm this side of the silicon\n\n");

    for (int i = 0; arguments[i].name != NULL; i++) {
        // Skip hidden aliases (they are printed with their parent)
        if (arguments[i].description == NULL)
            continue;

        // Print Main Flag
        printf("--%s", arguments[i].name);

        // Print Short Flag
        if (arguments[i].alias != 0) {
            printf(", -%c", arguments[i].alias);
        }

        // Print Aliases
        for (int j = 0; arguments[j].name != NULL; j++) {
            if (i == j)
                continue; // Skip self
            if (arguments[j].id == arguments[i].id) {
                printf(", --%s", arguments[j].name);
            }
        }

        printf(": %s\n", arguments[i].description);
    }

    exit(0);
}

int parse_args(struct Argument arguments[], struct Config *config, int argc,
               char **argv) {
    for (int i = 1; i < argc; i++) {
        char *current = argv[i];
        int matched = 0;

        for (int j = 0; arguments[j].name != NULL; j++) {
            // Check Short Flag (Only if alias is not 0)
            int is_short = (arguments[j].alias != 0 &&
                            current[0] == '-' &&
                            current[1] == arguments[j].alias &&
                            current[2] == '\0');

            // Check Long Flag (Matches --name)
            int is_long = (strncmp(current, "--", 2) == 0 &&
                           strcmp(current + 2, arguments[j].name) == 0);

            if (is_short || is_long) {
                matched = 1;

                switch (arguments[j].id) {
                case ARG_COLOR:
                    if (i + 1 < argc) {
                        int color_val = get_color_from_alias(argv[i + 1]);

                        if (color_val != -1) {
                            config->color = color_val;
                        } else {
                            fprintf(stderr, "Unknown color '%s'. Defaulting to blue.\n", argv[i + 1]);
                            config->color = COLOR_BLUE;
                        }
                        i++;
                    }
                    break;

                case ARG_CHARACTER:
                    if (i + 1 < argc) {
                        config->character = argv[i + 1][0];
                        i++;
                    } else {
                        fprintf(stderr, "Error: %s requires a value\n", current);
                        return 1;
                    }
                    break;

                case ARG_HELP:
                    help(arguments);
                    return 0;
                }
                break; // Break inner loop (found match)
            }
        }

        if (!matched) {
            fprintf(stderr, "Unknown argument: %s\n", current);
            return -1;
        }
    }
    return 0;
}

int snprintf_wrap(char *restrict s, size_t maxlen, const char *restrict format, const char *arg1, const char *arg2) {
    size_t written = 0;
    const char *args[2] = {arg1, arg2};
    int toggle = 0;

    for (const char *p = format; *p != '\0'; p++) {
        if (*p == '%' && *(p + 1) == 's') {
            const char *val = args[toggle];
            toggle = !toggle;
            p++;

            while (*val) {
                if (maxlen > 0 && written < maxlen - 1) {
                    s[written] = *val;
                }
                written++;
                val++;
            }
        } else {
            if (maxlen > 0 && written < maxlen - 1) {
                s[written] = *p;
            }
            written++;
        }
    }

    if (maxlen > 0) {
        s[written < maxlen ? written : maxlen - 1] = '\0';
    }

    return written;
}

int main(int argc, char **argv) {
    struct Config config = {.character = '|', .splash = 1, .color = COLOR_BLUE};

    if (argc > 1) {
        char color_desc[83];
        snprintf_wrap(color_desc, 83, "set the color [%sb%slue, %sr%sed, %sg%sreen, %sy%sellow, %sw%shite]", COLOR_BOLD, COLOR_OFF);

        struct Argument arguments[] = {{ARG_COLOR, "color", color_desc, 0},
                                       {ARG_COLOR, "co", NULL, 0},
                                       {ARG_CHARACTER, "character", "set the character", 0},
                                       {ARG_CHARACTER, "ch", NULL, 0},
                                       //{ARG_NO_SPLASH, "no-splash", "disable splash", 's'},
                                       {ARG_HELP, "help", "view help", 'h'},
                                       {0, NULL, NULL, 0}};

        if (parse_args(arguments, &config, argc, argv) != 0) {
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
        init_pair(1, config.color, -1);
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
