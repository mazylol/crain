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
#include <time.h>
#include <unistd.h>

#include "arg.h"
#include "util.h"

#define MAX_DROPS 1000

struct Drop {
    float x;
    float y;
    float vy;
    int active;
};

int main(int argc, char **argv) {
    struct Config config = {
        .character = '|',
        .splash = 1,
        .color = COLOR_BLUE,
        .acceleration = 0.025f};

    if (argc > 1) {
        char color_desc[133];
        small_snprintf(color_desc, 133, "set the color [%sb%slue, %sr%sed, %sg%sreen, %sy%sellow, %sm%sagenta, %sc%syan, %sb%slac%sk%s, %sw%shite]", "\e[1m", "\e[m");

        struct Argument arguments[] = {
            {ARG_COLOR, "color", color_desc, 0},
            {ARG_COLOR, "co", NULL, 0},
            {ARG_CHARACTER, "character", "set the character", 0},
            {ARG_CHARACTER, "ch", NULL, 0},
            {ARG_ACCELERATION, "acceleration", "set gravity intensity (e.g. 0.025)", 'a'},
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

    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    struct Drop drops[MAX_DROPS];
    for (int i = 0; i < MAX_DROPS; i++) {
        drops[i].active = 0;
    }

    int spawn_timer = 0;

    attron(COLOR_PAIR(1));

    while (1) {
        int ch = getch();
        if (ch == 'q')
            break;

        erase();

        if (spawn_timer <= 0) {
            int drops_to_make = rand_in_range(1, 4);

            for (int k = 0; k < drops_to_make; k++) {
                for (int i = 0; i < MAX_DROPS; i++) {
                    if (drops[i].active == 0) {
                        drops[i].active = 1;
                        drops[i].x = rand_in_range(0, max_x - 1);
                        drops[i].y = 0;

                        drops[i].vy = (float)rand_in_range(25, 50) / 100.0f;
                        break;
                    }
                }
            }
            spawn_timer = rand_in_range(5, 10);
        } else {
            spawn_timer--;
        }

        for (int i = 0; i < MAX_DROPS; i++) {
            if (drops[i].active) {

                drops[i].vy += config.acceleration;

                drops[i].y += drops[i].vy;

                if (drops[i].y >= max_y) {
                    drops[i].active = 0;
                    continue;
                }

                mvaddch((int)drops[i].y, (int)drops[i].x, config.character);
            }
        }

        refresh();

        napms(33); // ~30 FPS
    }

    attroff(COLOR_PAIR(1));
    endwin();

    return 0;
}
