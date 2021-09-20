#include <boost/program_options.hpp>
#include <iostream>
#include <cassert>
#include <immintrin.h>

#include "safe-read.h"
#include <aio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "readbench_mmap.h"
#include "readbench_popcnt.h"
#include "readbench_fread.h"
#include "readbench_aioread.h"
#include "readbench_cat.h"
#include "readbench.h"


#define WAIT
#define SPINWAITS 50000


namespace pop = boost::program_options;

using namespace std;

double getdeltatime(struct timeval *begin, struct timeval *end) {
    return (end->tv_sec + end->tv_usec * 1.0 / 1000000) -
           (begin->tv_sec + begin->tv_usec * 1.0 / 1000000);
}

bool parse_cmdline(pop::variables_map &vm, int argc, char *argv[]) {
    pop::options_description generic("Generic options");
    generic.add_options()
            ("help,h", "help message");

    pop::options_description config("Configured");
    config.add_options()
            ("implementation", pop::value<int>()->required(), "implementation number");

    config.add_options()
            ("readsize", pop::value<size_t>()->default_value(1024 * 1024 * 4), "read io size in bytes");

    config.add_options()
            ("bufmax", pop::value<size_t>()->default_value(1024 * 1024 * 256), "maximum buffer size in bytes");

    config.add_options()
            ("input-file", pop::value<std::vector<std::string>>()->required(), "input file");

    pop::positional_options_description positional;
    positional.add("input-file", -1);

    pop::options_description cmdline_options;
    cmdline_options.add(config).add(generic);

    auto parsed = pop::command_line_parser(argc, argv).options(cmdline_options).positional(positional).run();
    pop::store(parsed, vm);

    if (vm.count("help")) {
        std::cout << cmdline_options << std::endl;
        return false;
    }

    pop::notify(vm);

    if (vm.count("implementation") != 1) {
        throw std::invalid_argument("Implementation number not specified");
    }

    if (vm.count("input-file") <= 0) {
        throw std::invalid_argument("No input files specified");
    }

    return true;
}


/* 
read file implementations
0- fread - heap buffer - single thread -
1- fread w/ fadvise sequential - heap buffer - single thread
2- ala simplecat -
3- posix AIO - single thread - aio.h
4- libaio - single thread - libaio.h
5- (c++ istream?) - single thread - iostream
6- mmap - single thread - mmap.h

 */



int main(int argc, char *argv[]) {
    pop::variables_map vm;

    if (!parse_cmdline(vm, argc, argv)) {
        return 1;
    }
    std::vector<std::string> inputs = vm["input-file"].as<std::vector<std::string >>();
    assert(inputs.size() == 1);

    const char *filename = inputs.at(0).c_str();
    size_t readsize = vm["readsize"].as<size_t>();
    size_t bufmax = vm["bufmax"].as<size_t>();
    unsigned long long numbits = 0ull;

    timespec before, after;
    clock_gettime(CLOCK_REALTIME, &before);

    switch (vm["implementation"].as<int>()) {
        case 0:
            cerr << "implementation: fread with a buffer on the heap" << endl;
            impl_one_fread(filename, readsize, bufmax, numbits, false);
            break;
        case 1:
            cerr << "implementation: fread with a buffer on the heap and fadvise_sequential" << endl;
            impl_one_fread(filename, readsize, bufmax, numbits, true);
            break;
        case 2:
            cerr << "implementation: copy paste code from 'cat' with a buffer on the heap" << endl;
            impl_two_simple_cat(filename, readsize, bufmax, numbits);
            break;
        case 3:
            cerr << "implementation: copy paste code from 'cat' with a buffer on the stack" << endl;
            impl_two_simple_cat(filename, readsize, bufmax, numbits, true);
            break;
        case 4:
            cerr << "implementation: refactored aio_read" << endl;
            impl_four_aio_blip(filename, readsize, bufmax, numbits);
            break;
        case 5:
            cerr << "implementation: boost-based mmap" << endl;
            impl_five_boost_mmap(filename, readsize, bufmax, numbits);
            break;
        case 99:
            cerr << "implementation: less refactored aio_read" << endl;
            impl_ninetynine_aio_messy(filename, readsize, bufmax, numbits);
            break;
        default:
            cerr << "implementation nyi" << endl;
            abort();
    };

    clock_gettime(CLOCK_REALTIME, &after);

    unsigned long long nanos = (after.tv_sec - before.tv_sec) * 1'000'000'000ull +
                               (after.tv_nsec - before.tv_nsec);
    FILE *fd = fopen(filename, "r");
    assert(fd != NULL);
    fseek(fd, 0, SEEK_END);
    unsigned long filesize = (unsigned long) ftell(fd);
    fclose(fd);
    unsigned long long mibps_1 = ((filesize * 1'000'000'000ull) / nanos) / 1'048'576ull;

    cerr << "num 1s (popcnt): " << numbits << endl;
    cerr << "file bytes: " << filesize << endl;
    cerr << "file transfer nanos: " << nanos << endl;
    cerr << "MB/s: " << mibps_1 << endl;
    cerr << "read chunk (b): " << readsize << endl;

    return 0;
}


// vim: set ts=4 sw=4 expandtab:
