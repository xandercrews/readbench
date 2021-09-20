//
// Created by achmed on 8/28/21.
//

#include "readbench_fread.h"
#include "readbench.h"


void
impl_one_fread(const char *filename, std::size_t readsize, [[maybe_unused]] std::size_t _bufmax,
               unsigned long long &numbits,
               bool fadvise = false) {
    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        fputs("File error", stderr);
        exit(1);
    }

    auto fn = fileno(fd);

    if (fadvise)
        posix_fadvise(fn, 0, 0, POSIX_FADV_SEQUENTIAL);

    char *buf = new char[readsize];

    while (true) {
        auto result = fread(buf, 1, readsize, fd);
        if (result < readsize) {
            buf[result] = '\0';
        }
        accumulate_ones(buf, result, numbits);
        if (result < readsize) {
            break;
        }
    }

    delete buf;
}