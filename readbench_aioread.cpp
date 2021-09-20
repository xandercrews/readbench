//
// Created by achmed on 8/28/21.
//

#include <sys/stat.h>
#include <sys/time.h>
#include <aio.h>
#include "safe-read.h"
#include <fcntl.h>
#include <cstdio>
#include "readbench_fread.h"
#include "readbench_popcnt.h"
#include "readbench_mmap.h"
#include <immintrin.h>
#include <cassert>
#include <iostream>
#include <boost/program_options.hpp>

#include "readbench_aioread.h"
#include "readbench_popcnt.h"
#include "readbench.h"

// temp
const size_t readsize = 2097152;
const size_t READSIZE = 2097152;

void impl_four_aio_blip(const char *filename, [[maybe_unused]] const std::size_t __readsize,
                        [[maybe_unused]] const std::size_t _bufmax,
                        unsigned long long &numbits) {
    char realbuf[2 * readsize];
    char *buf[2] = {realbuf, &realbuf[readsize]};

    memset((void *) buf[0], 0, readsize);
    memset((void *) buf[1], 0, readsize);

    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        fputs("File error", stderr);
        exit(1);
    }

    auto fn = fileno(fd);

    posix_fadvise(fn, 0, 0, POSIX_FADV_SEQUENTIAL);

    // int bufnum = 0;
    std::size_t cur = 0;

    aiocb cb[2];

    memset((void *) &cb[0], 0, sizeof(aiocb));
    memset((void *) &cb[1], 0, sizeof(aiocb));

    cb[0].aio_nbytes = readsize;
    cb[0].aio_fildes = fn;
    cb[0].aio_offset = 0;
    cb[0].aio_buf = buf[0];

    cb[1].aio_nbytes = readsize;
    cb[1].aio_fildes = fn;
    cb[1].aio_offset = readsize;
    cb[1].aio_buf = buf[1];

    int now = 0;

    aio_read(&cb[now]);
    aio_read(&cb[now + 1]);

    // struct timespec huge;
    // huge.tv_sec = 2;
    // huge.tv_nsec = 0;

    struct timespec tiny;
    tiny.tv_sec = 0;
    tiny.tv_nsec = 1;

    aiocb *p = &cb[now];

    unsigned long long totalwaits = 0;
    unsigned long long waits = 0;

    while (aio_error(&cb[now]) == EINPROGRESS) {
#ifndef WAIT
        aio_suspend(&p, 1, &tiny);
#else
        waits += 1;
        if (waits > SPINWAITS) {
            struct timespec rem;
            nanosleep(&tiny, &rem);
        }
#endif
    }

    off64_t offset = readsize * 2;

    while ((cur = aio_return(&cb[now]))) {
        if (cur == (std::size_t) -1) {
            perror("read failed");
            abort();
        }
        if (!cur)
            break;

        accumulate_ones((char *) cb[now].aio_buf, cur, numbits);

        cb[now].aio_offset = offset;
        offset += readsize;

        aio_read(&cb[now]);

        now = (now + 1) % 2;

        // p = &cb[now];

        waits = 0;

        while (aio_error(&cb[now]) == EINPROGRESS) {
#ifndef WAIT
            aio_suspend(&p, 1, &tiny);
#else
            waits += 1;
            if (waits > SPINWAITS) {
                struct timespec rem;
                nanosleep(&tiny, &rem);
            }
#endif
        }
        totalwaits += waits;
    }

    fclose(fd);

    std::cerr << "waits: " << totalwaits << std::endl;
}

void
impl_four_aio_blip_broken(const char *filename, const std::size_t readsize, [[maybe_unused]] const std::size_t _bufmax,
                          unsigned long long &numbits) {
    auto BUFFERSIZE = readsize;
    char buf[2][BUFFERSIZE];

    memset((void *) buf[0], 0, BUFFERSIZE);
    memset((void *) buf[1], 0, BUFFERSIZE);

    FILE *fd = fopen(filename, "r");

    if (fd == NULL) {
        fputs("File error", stderr);
        exit(1);
    }

    auto fn = fileno(fd);

    posix_fadvise(fn, 0, 0, POSIX_FADV_SEQUENTIAL);

    std::size_t cur = 0;

    aiocb cb[2];

    memset((void *) &cb[0], 0, sizeof(aiocb));
    memset((void *) &cb[1], 0, sizeof(aiocb));

    cb[0].aio_nbytes = BUFFERSIZE;
    cb[0].aio_fildes = fn;
    cb[0].aio_offset = 0;
    cb[0].aio_buf = buf[0];

    cb[1].aio_nbytes = BUFFERSIZE;
    cb[1].aio_fildes = fn;
    cb[1].aio_offset = BUFFERSIZE;
    cb[1].aio_buf = buf[1];

    int now = 0;

    aio_read(&cb[now]);
    aio_read(&cb[now + 1]);

    struct timespec huge;
    huge.tv_sec = 2;
    huge.tv_nsec = 0;

    aiocb *p = &cb[now];

    while (aio_error(&cb[now]) == EINPROGRESS) {
        aio_suspend(&p, 1, &huge);
    }

    off64_t offset = BUFFERSIZE * 2;

    while ((cur = aio_return(&cb[now]))) {
        if (cur == (std::size_t) -1) {
            perror("read failed");
            abort();
        }
        if (!cur)
            break;

        accumulate_ones((char *) cb[now].aio_buf, cb[now].aio_nbytes, numbits);

        cb[now].aio_offset = offset;
        offset += BUFFERSIZE;

        aio_read(&cb[now]);

        now = (now + 1) % 2;

        p = &cb[now];

        while (aio_error(&cb[now]) == EINPROGRESS) {
            aio_suspend(&p, 1, &huge);
        }
    }

    fclose(fd);
}

void impl_ninetynine_aio_messy(const char *filename, [[maybe_unused]] const std::size_t __readsize,
                               [[maybe_unused]] const std::size_t _bufmax, unsigned long long &retbits) {
    struct timeval begin, end;

    static const auto BUFFER_SIZE = READSIZE;

    gettimeofday(&begin, NULL);

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open error");
    }

    // Advise the kernel of our access pattern.
    posix_fadvise(fd, 0, 0, 1);  // FDADVICE_SEQUENTIAL

    char buf[2][BUFFER_SIZE + 1];

    std::size_t bytes_read = 0;
    std::size_t cur = 0;

    u_int64_t numbits = 0;

    // int bufnum = 0;

    aiocb cb[2];

    cb[0].aio_nbytes = BUFFER_SIZE;
    cb[0].aio_fildes = fd;
    cb[0].aio_offset = 0;
    cb[0].aio_buf = buf[0];

    cb[1].aio_nbytes = BUFFER_SIZE;
    cb[1].aio_fildes = fd;
    cb[1].aio_offset = BUFFER_SIZE;
    cb[1].aio_buf = buf[1];

    int now = 0;
    int later = 1;

    aio_read(&cb[now]);
    aio_read(&cb[later]);

#ifdef WAIT
    struct timespec tiny;
    tiny.tv_sec = 0;
    tiny.tv_nsec = 1;
#endif

    struct timespec huge;
    huge.tv_sec = 2;
    huge.tv_nsec = 0;

    uint64_t waits = 0;
    uint64_t totalwaits = 0;

    waits = 0;

    aiocb *p = &cb[now];

    while (aio_error(&cb[now]) == EINPROGRESS) {
        aio_suspend(&p, 1, &huge);
#ifdef WAIT
        waits += 1;
        if (waits > SPINWAITS) {
            struct timespec rem;
            nanosleep(&tiny, &rem);
        }
#endif
    }

    struct stat64 stats;

    fstat64(fd, &stats);

#ifndef NDEBUG
    auto file_size = stats.st_size;
#endif

    totalwaits = waits;

    off64_t offset = BUFFER_SIZE * 2;

    while ((cur = aio_return(&cb[now]))) {
        if (cur == (std::size_t) -1)
            perror("read failed");
        if (!cur)
            break;
        bytes_read += cur;
        assert(bytes_read == file_size || cur == BUFFER_SIZE);

        auto end = (u_int64_t *) (buf[now] + cur);
        for (u_int64_t *ix = (u_int64_t *) buf[now]; ix < end; ++ix) {
            numbits += _mm_popcnt_u64(*ix);
        }

        cb[now].aio_offset = offset;
        offset += BUFFER_SIZE;

        aio_read(&cb[now]);

        now = (now + 1) % 2;

        waits = 0;

        aiocb *p = &cb[now];

        while (aio_error(&cb[now]) == EINPROGRESS) {
            aio_suspend(&p, 1, &huge);
#ifdef WAIT
            waits += 1;
            if (waits > SPINWAITS) {
                struct timespec rem;
                nanosleep(&tiny, &rem);
            }
#endif
        }
        totalwaits += waits;
    }

    gettimeofday(&end, NULL);

    auto tm = getdeltatime(&begin, &end);
    printf("%luB %.0fMB %.0fMB/s\n", bytes_read, (bytes_read * 1.0) / (1024 * 1024),
           (bytes_read * 1.0) / (tm * 1024 * 1024));

    std::cout << "waits: " << totalwaits << std::endl;
    std::cout << "1s: " << numbits << std::endl;
    close(fd);

    retbits = numbits;
}