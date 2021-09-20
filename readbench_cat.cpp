#include "readbench_cat.h"

int input_desc;

static bool
simple_cat(
        /* Pointer to the buffer, used by reads and writes.  */
        char *buf,

        /* Number of characters preferably read or written by each read and write
           call.  */
        size_t bufsize,

        /* accumulator */
        unsigned long long &numbits) {
    /* Actual number of characters read, and therefore written.  */
    size_t n_read;

    /* Loop until the end of the file.  */

    while (true) {
        /* Read a block of input.  */

        n_read = safe_read(input_desc, buf, bufsize);
        if (n_read == SAFE_READ_ERROR) {
            return false;
        }

        /* End of this file?  */

        if (n_read == 0)
            return true;

        /* ~Write~ accumulate this block out.  */

        accumulate_ones(buf, n_read, numbits);
    }
}

/* i couldn't spot the important difference between cat's implementation and the naive one
 * but syscalls are taking 3-4x as long vs the cat util.  turns out the difference is having 
 * the buffer passed to read() on the stack or heap */
void impl_two_simple_cat(const char *filename, size_t readsize, [[maybe_unused]] size_t _bufmax,
                         unsigned long long &numbits, bool stack) {
    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        fputs("File error", stderr);
        exit(1);
    }

    auto fn = fileno(fd);

    posix_fadvise(fn, 0, 0, POSIX_FADV_SEQUENTIAL);

    input_desc = fn;

    if (stack) {
        char buf[readsize];
        simple_cat((char *) buf, readsize, numbits);
    } else {
        char *buf = new char[readsize];
        simple_cat(buf, readsize, numbits);
        delete buf;
    }
}
