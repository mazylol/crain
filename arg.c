#include "arg.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

                case ARG_ACCELERATION:
                    if (i + 1 < argc) {
                        char *endptr;
                        float val = strtof(argv[i + 1], &endptr);

                        if (*endptr == '\0') {
                            config->acceleration = val;
                        } else {
                            fprintf(stderr, "Invalid number for acceleration: %s\n", argv[i + 1]);
                            return 1;
                        }

                        i++;
                    } else {
                        fprintf(stderr, "Error: --acceleration requires a value\n");
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
