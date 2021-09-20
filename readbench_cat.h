//
// Created by achmed on 8/31/21.
//

#ifndef LOGDEMO_READBENCH_CAT_H
#define LOGDEMO_READBENCH_CAT_H

#include "safe-read.h"
#include "readbench.h"

using namespace std;

static bool simple_cat( char *buf, size_t bufsize, unsigned long long &numbits);
    void impl_two_simple_cat(const char *filename, size_t readsize, [[maybe_unused]] size_t _bufmax,
                             unsigned long long &numbits, bool stack = false);
#endif //LOGDEMO_READBENCH_CAT_H
