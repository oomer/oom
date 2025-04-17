#pragma once

#include <string>       // For std::string
#include <filesystem>   // For std::filesystem
// This is a set of common oomer utilities for file queue operations

#include "../efsw/src/efsw/FileSystem.hpp" // For file watching
#include "../efsw/src/efsw/System.hpp" // For file watching
#include "../efsw/include/efsw/efsw.hpp" // For file watching

#include "../bella_engine_sdk/src/dl_core/dl_string.h"
#include "../bella_engine_sdk/src/dl_core/dl_fs.h"

namespace oom {
    namespace filesystem {
        /// A class that manages a queue of files to render with both FIFO order and fast lookups
    }
}

        



