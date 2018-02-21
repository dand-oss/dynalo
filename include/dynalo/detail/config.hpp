#pragma once

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(_LINUX)
    #define DYNALO_HAS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
    #define DYNALO_HAS_WINDOWS
#else
    #error "dynalo/detail/config.hpp OS Not Supported"
#endif

#define DYNALO_VERSION_MAJOR 1
#define DYNALO_VERSION_MINOR 0
#define DYNALO_VERSION_PATCH 2
#define DYNALO_VERSION       0x010002  /**< major minor patch*/
