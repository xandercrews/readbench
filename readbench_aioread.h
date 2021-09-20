//
// Created by achmed on 8/28/21.
//

#ifndef LOGDEMO_READBENCH_AIOREAD_H
#define LOGDEMO_READBENCH_AIOREAD_H

#include "readbench.h"
#include <cstddef>

void impl_four_aio_blip(const char *filename, [[maybe_unused]] const std::size_t __readsize,
                        [[maybe_unused]] const std::size_t _bufmax,
                        unsigned long long &numbits);

void impl_ninetynine_aio_messy(const char *filename, [[maybe_unused]] const std::size_t __readsize,
                               [[maybe_unused]] const std::size_t _bufmax, unsigned long long &retbits);

#endif //LOGDEMO_READBENCH_AIOREAD_H
