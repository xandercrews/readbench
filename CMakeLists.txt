cmake_minimum_required(VERSION 3.21)
project(logdemo)
set(CMAKE_CXX_STANDARD 23)

####
# cmake includes
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake-include" ${CMAKE_MODULE_PATH})
####

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

message(STATUS "${_cxx_flags}")

function(find_static_library LIB_NAME OUT)

    if (WIN32 OR MSVC)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
    elseif (UNIX)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    endif()

    find_library(
            FOUND_${LIB_NAME}_STATIC
            ${LIB_NAME}
    )

    if (FOUND_${LIB_NAME}_STATIC)
        get_filename_component(ABS_FILE ${FOUND_${LIB_NAME}_STATIC} ABSOLUTE)
    else()
        message(SEND_ERROR "Unable to find library ${LIB_NAME}")
    endif()

    set(${OUT} ${ABS_FILE} PARENT_SCOPE)

endfunction()

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

#####
# global static opts

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(BUILD_SHARED_LIBS ON)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

####

#####
# Boost

# boost
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     OFF)
find_package(Boost 1.65.0 REQUIRED COMPONENTS program_options)

foreach(Component ${BoostComponents})
    string(TOUPPER "${Component}" ComponentUpper)
    message(STATUS "${ComponentUpper}: ${Boost_${ComponentUpper}_FOUND} - ${Boost_${ComponentUpper}_LIBRARY}")
endforeach()

if (NOT Boost_FOUND)
    message(FATAL_ERROR "could not find boost" )
endif()

include_directories(${Boost_INCLUDE_DIRS})

####


######
## libaio

find_library(AIO_LIBRARIES NAMES libaio.a REQUIRED)

####


#####
## pthreads

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
set(Threads_LIBRARIES Threads::Threads)
####


#####

add_compile_options(
        "-Wall" "-Wpedantic" "-Wextra" "-Wno-vla" "-fexceptions"
        "$<$<CONFIG:RELEASE>:-O3>"
        "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)
####

message(STATUS "aio lib(s): ${AIO_LIBRARIES}")

add_executable(readbench readbench.cpp readbench_cat.cpp readbench_mmap.cpp readbench_fread.cpp readbench_aioread.cpp safe-read.cpp safe-read.h readbench_cat.h)
target_link_libraries(readbench ${Boost_LIBRARIES} -static)
target_link_libraries(readbench ${Threads_LIBRARIES} rt -static)
