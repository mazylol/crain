#ifndef UTIL_H
#define UTIL_H

#include <string.h>

int rand_in_range(int min, int max);
int small_snprintf(char *restrict s, size_t maxlen, const char *restrict format, const char *arg1, const char *arg2);

#endif // UTIL_H
