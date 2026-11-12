// features/sort_types.cppm
// Shared algorithm identifiers — imported by time_misc and memory_misc to
// eliminate the duplicate SortAlgorithm enum that existed in both modules.

export module features.sort_types;

import <string>;
import <unordered_map>;
import <string_view>;
import <optional>;

export enum class SortAlgorithm {
    bubble,
    stable_merge,
    unstable_merge,
    insertion,
    optimal_quick,
    naive_quick,
    heap,
    bogo,
    lsd_radix,
    msd_radix,
    shell,
    user_introsort,
    introsort,
    unknown
};

// Returns the SortAlgorithm for a given name, or unknown if not found.
// Accepts string_view; no heap allocation needed for lookup.
export SortAlgorithm algorithm_from_name(std::string_view name) {
    // Build the map once; static lifetime inside a function is safe in modules.
    static const std::unordered_map<std::string, SortAlgorithm> map {
        { "bubble",         SortAlgorithm::bubble         },
        { "stable_merge",   SortAlgorithm::stable_merge   },
        { "unstable_merge", SortAlgorithm::unstable_merge },
        { "insertion",      SortAlgorithm::insertion      },
        { "optimal_quick",  SortAlgorithm::optimal_quick  },
        { "naive_quick",    SortAlgorithm::naive_quick    },
        { "heap",           SortAlgorithm::heap           },
        { "bogo",           SortAlgorithm::bogo           },
        { "lsd_radix",      SortAlgorithm::lsd_radix      },
        { "msd_radix",      SortAlgorithm::msd_radix      },
        { "shell",          SortAlgorithm::shell          },
        { "user_introsort", SortAlgorithm::user_introsort },
        { "introsort",      SortAlgorithm::introsort      },
    };

    auto it = map.find(std::string(name)); // string_view → string for lookup
    return (it != map.end()) ? it->second : SortAlgorithm::unknown;
}
