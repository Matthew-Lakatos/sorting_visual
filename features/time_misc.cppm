// features/time_misc.cppm
// FIX: take_time() previously tried to assign a time_point to a double — that
//      doesn't compile. Return type is now std::chrono::steady_clock::time_point.
// FIX: total_time_calc() now returns double explicitly and takes const-ref args.
// FIX: Duplicate SortAlgorithm enum and converter map removed; imported from
//      features.sort_types instead.
// FIX: All functions are now properly exported so main.cpp can call them.
// FIX: output_time() takes its argument by value (it's a scalar — no need for
//      a non-const reference).
// IMPROVEMENT: output_time_complexity() now reports when an algorithm name is
//              not recognised rather than silently doing nothing.

export module features.time_misc;

import features.sort_types;

import <chrono>;
import <iostream>;
import <string_view>;

// Returns the current time as a steady_clock time_point.
// BUG WAS: `double time = std::chrono::high_resolution_clock::now();`
//           — a time_point cannot be assigned to double.
export std::chrono::steady_clock::time_point take_time() {
    return std::chrono::steady_clock::now();
}

// Returns elapsed milliseconds between two time_points.
// BUG WAS: `auto&` parameter type for start/end was fine, but the return type
//           was already correct; keeping explicit types for clarity.
export double total_time_calc(
    const std::chrono::steady_clock::time_point& start,
    const std::chrono::steady_clock::time_point& end)
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

// Prints elapsed time with a tidy label.
// BUG WAS: `double&` — scalar values should not be passed as non-const refs.
export void output_time(double duration) {
    std::cout << "Time taken for the algorithm to execute: "
              << duration << " ms\n";
}

export void output_time_complexity(std::string_view sorting_algorithm) {
    switch (algorithm_from_name(sorting_algorithm)) {
        case SortAlgorithm::bubble:
            std::cout << "Time complexity: O(n^2) average/worst, O(n) best\n";
            break;
        case SortAlgorithm::stable_merge:
        case SortAlgorithm::unstable_merge:
            std::cout << "Time complexity: O(n log n) all cases\n";
            break;
        case SortAlgorithm::insertion:
            std::cout << "Time complexity: O(n^2) average/worst, O(n) best\n";
            break;
        case SortAlgorithm::optimal_quick:
            std::cout << "Time complexity: O(n log n) all cases\n";
            break;
        case SortAlgorithm::naive_quick:
            std::cout << "Time complexity: O(n log n) average, O(n^2) worst (poor pivot)\n";
            break;
        case SortAlgorithm::heap:
            std::cout << "Time complexity: O(n log n) all cases\n";
            break;
        case SortAlgorithm::bogo:
            std::cout << "Time complexity: O(n * n!) average, O(unbounded) worst, O(n) best\n";
            break;
        case SortAlgorithm::lsd_radix:
        case SortAlgorithm::msd_radix:
            std::cout << "Time complexity: O(n * d) where d = digits of largest value\n"
                      << "  NOTE: not suitable for floating-point, strings of varying length, or bools\n";
            break;
        case SortAlgorithm::shell:
            std::cout << "Time complexity: O(n^(4/3)) amortised (Sedgewick gap), O(n log n) best, O(n^2) worst\n";
            break;
        case SortAlgorithm::user_introsort:
        case SortAlgorithm::introsort:
            std::cout << "Time complexity: O(n log n) amortised\n";
            break;
        case SortAlgorithm::shell:
            std::cout << "Time complexity: O(n^(4/3)) amortised (Sedgewick/Ciura gap), O(n log n) best, O(n^2) worst (Shell gaps)\n";
            break;
        case SortAlgorithm::unknown:
            std::cout << "Unknown algorithm '" << sorting_algorithm
                      << "' — no time complexity information available\n";
            break;
    }
}
