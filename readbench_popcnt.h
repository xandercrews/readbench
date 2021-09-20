//
// Created by achmed on 8/28/21.
//

#ifndef LOGDEMO_READBENCH_POPCNT_H
#define LOGDEMO_READBENCH_POPCNT_H

#include <sys/stat.h>
#include <sys/time.h>
#include <aio.h>
#include "safe-read.h"
#include <fcntl.h>
#include <cstdio>
#include "readbench_mmap.h"
#include <immintrin.h>
#include <cassert>
#include <iostream>
#include <boost/program_options.hpp>

#include "readbench_popcnt.h"
#include "readbench.h"


#define really_inline inline __attribute__((always_inline, unused))

really_inline void accumulate_ones(char *buf, std::size_t buflen, unsigned long long &accumulator) {
    auto cur = reinterpret_cast<u_int64_t *>(buf);
    std::size_t numints = buflen / sizeof(u_int64_t);
    std::size_t rem = buflen % sizeof(u_int64_t);
    for (std::size_t ix = 0; ix < numints; ++cur, ++ix) {
        accumulator += _mm_popcnt_u64(*cur);
    }
    /* count the last bits of a block that is not 8 byte aligned */
    char rembuf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    assert(rem < 8);
    if (rem > 0) {
        memcpy((void *) &(rembuf[8 - rem]), (void *) (cur), rem);
        accumulator += _mm_popcnt_u64(*(reinterpret_cast<u_int64_t *>(rembuf)));
    }
}


#endif //LOGDEMO_READBENCH_POPCNT_H
