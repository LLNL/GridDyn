#ifndef _ZLIBARGV_H
#define _ZLIBARGV_H

char **copy_argv(int argc, const char** argv);
void free_argv(int argc, char **new_argv);

#endif
