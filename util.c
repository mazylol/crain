#include "util.h"

#include <stdlib.h>

int rand_in_range(int min, int max) {
    return rand() % (max - min + 1) + min;
}

int small_snprintf(char *restrict s, size_t maxlen, const char *restrict format, const char *arg1, const char *arg2) {
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
