#ifndef ERROR_H_
#define ERROR_H_

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define STANDARD_ERROR(msg) fprintf(stderr, "%s: %s\n", msg, strerror(errno))
#define LIBRARY_ERROR(lib, msg) fprintf(stderr, "%s: %s\n", lib, msg)
#define ERROR(msg) fprintf(stderr, "%s\n", msg)

#endif
