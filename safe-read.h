//
// Created by achmed on 8/23/21.
//


#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


#define SAFE_READ_ERROR ((size_t) -1)

/* Read up to COUNT bytes at BUF from descriptor FD, retrying if interrupted.
   Return the actual number of bytes read, zero for EOF, or SAFE_READ_ERROR
   upon error.  */
extern size_t safe_read(int fd, void *buf, size_t count);


#ifdef __cplusplus
}
#endif