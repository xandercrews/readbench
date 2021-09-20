//
// Created by achmed on 8/28/21.
//

#ifndef LOGDEMO_READBENCH_MMAP_H
#define LOGDEMO_READBENCH_MMAP_H

#include <boost/exception/all.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <sys/stat.h>
#include <sys/time.h>
#include <aio.h>
#include "safe-read.h"
#include <fcntl.h>
#include <cstdio>
#include <immintrin.h>
#include <cassert>
#include <iostream>
#include <boost/program_options.hpp>

#include "readbench_popcnt.h"

void impl_five_boost_mmap(const char *filename, std::size_t readsize, std::size_t bufmax, unsigned long long int &numbits);


#endif //LOGDEMO_READBENCH_MMAP_H
