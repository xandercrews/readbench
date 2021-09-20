//
// Created by achmed on 8/28/21.
//

#ifndef LOGDEMO_READBENCH_FREAD_H
#define LOGDEMO_READBENCH_FREAD_H

#include <sys/stat.h>
#include <sys/time.h>
#include <aio.h>
#include "safe-read.h"
#include <fcntl.h>
#include <cstdio>
#include "readbench_popcnt.h"
#include "readbench_mmap.h"
#include <immintrin.h>
#include <cassert>
#include <iostream>
#include <boost/program_options.hpp>

#include "readbench_popcnt.h"

void
impl_one_fread(const char *filename, std::size_t readsize, [[maybe_unused]] std::size_t _bufmax,
               unsigned long long &numbits, bool fadvise);

#endif //LOGDEMO_READBENCH_FREAD_H
