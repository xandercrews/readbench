//
// Created by achmed on 8/28/21.
//

#include "readbench_mmap.h"
#include "readbench.h"

struct madvise_error : virtual boost::exception {
};

void madvise_region(boost::interprocess::mapped_region &region) {
    void *addr = region.get_address();
    std::size_t size = region.get_size();

    boost::shared_ptr<int> result(new int(0));
    *result = madvise(addr, size, MADV_WILLNEED | MADV_SEQUENTIAL);

    if (*result != 0) {
        throw madvise_error() << boost::errinfo_errno(*result);
    }
}

void impl_five_boost_mmap(const char *filename, [[maybe_unused]] std::size_t readsize, [[maybe_unused]] std::size_t bufmax, unsigned long long int &numbits) {
    boost::interprocess::file_mapping m_file
    (filename,
     boost::interprocess::read_only
     );

    boost::interprocess::mapped_region region
    (m_file,
     boost::interprocess::read_only
     );

    void *addr = region.get_address();
    std::size_t size = region.get_size();

    madvise_region(region);

    accumulate_ones((char*) addr, size, numbits);
}

namespace ip = boost::interprocess;