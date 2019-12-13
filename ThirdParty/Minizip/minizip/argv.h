#ifndef MINIZIP_ARGV_H
#define MINIZIP_ARGV_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

char **copy_argv(int argc, const char** argv);
void free_argv(int argc, char **new_argv);

#ifdef __cplusplus
}
#endif

#endif /* MINIZIP_ARGV_H */
