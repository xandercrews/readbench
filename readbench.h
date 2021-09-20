//
// Created by achmed on 8/28/21.
//

#ifndef LOGDEMO_READBENCH_H
#define LOGDEMO_READBENCH_H

#include <sys/stat.h>
#include <sys/time.h>
#include <aio.h>
#include "readbench.h"
#include "readbench_aioread.h"
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
#include "readbench.h"
#include <cstddef>

double getdeltatime(struct timeval *begin, struct timeval *end);

#endif //LOGDEMO_READBENCH_H
