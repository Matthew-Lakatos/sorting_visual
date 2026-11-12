// features/memory_misc.cppm
// FIX: Duplicate SortAlgorithm enum and converter map removed; imported from
//      features.sort_types instead.
// FIX: All free functions are now exported.
// FIX: output_memory_complexity() now handles SortAlgorithm::unknown gracefully.
// FIX: `import <windows.h>` and `import <psapi.h>` are non-standard — system
//      headers that are not C++ module units must be included with #include
//      inside a module purview (or in a global module fragment). Using
//      module-linkage #include guards instead.
// IMPROVEMENT: report_process_memory_size() exposed as an export so callers
//              can invoke it directly.

export module features.memory_misc;

import features.sort_types;

import <iostream>;
import <string_view>;
import <cstddef>;

// Platform-specific memory headers must use #include, not import.
#ifdef _WIN32
#  include <windows.h>
#  include <psapi.h>
#elif __linux__
#  include <unistd.h>
#  include <sys/resource.h>
#endif

// Reports the peak resident-set / working-set size of the current process.
export void report_process_memory_size() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS memInfo;
    GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo));
    std::cout << "Process Working Set Size: "
              << memInfo.WorkingSetSize / 1024 << " KB\n";
#elif __linux__
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    std::cout << "Max Resident Set Size: " << usage.ru_maxrss << " KB\n";
#else
    std::cout << "Platform not supported by this implementation\n";
#endif
}

export void output_memory_complexity(std::string_view sorting_algorithm) {
    switch (algorithm_from_name(sorting_algorithm)) {
        case SortAlgorithm::bubble:
        case SortAlgorithm::insertion:
        case SortAlgorithm::heap:
        case SortAlgorithm::bogo:
        case SortAlgorithm::shell:
            std::cout << "Memory complexity: O(1) (in-place — gap sequence stored as a small static array)\n";
            break;
        case SortAlgorithm::stable_merge:
        case SortAlgorithm::unstable_merge:
            std::cout << "Memory complexity: O(n) (auxiliary arrays required)\n";
            break;
        case SortAlgorithm::optimal_quick:
        case SortAlgorithm::user_introsort:
        case SortAlgorithm::introsort:
            std::cout << "Memory complexity: O(log n) (tail-recursion optimised / depth-capped)\n";
            break;
        case SortAlgorithm::naive_quick:
            std::cout << "Memory complexity: O(n) worst case (unbalanced recursion)\n";
            break;
        case SortAlgorithm::lsd_radix:
            std::cout << "Memory complexity: O(n + k) where k = digit range (10 for decimal)\n";
            break;
        case SortAlgorithm::msd_radix:
            std::cout << "Memory complexity: O(n * k) where k = digit range\n";
            break;
        case SortAlgorithm::unknown:
            std::cout << "Unknown algorithm '" << sorting_algorithm
                      << "' — no memory complexity information available\n";
            break;
    }
}
