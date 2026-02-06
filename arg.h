#ifndef ARG_H
#define ARG_H

struct Argument {
    int id;
    char *name;
    char *description;
    char alias;
};

enum { ARG_CHARACTER,
       ARG_COLOR,
       // ARG_NO_SPLASH,
       ARG_ACCELERATION,
       ARG_HELP };

struct Config {
    char character;
    int splash;
    int color;
    float acceleration;
};

struct StringAlias {
    char *key;
    int value;
};

int parse_args(struct Argument arguments[], struct Config *config, int argc,
               char **argv);

#endif // ARG_H
