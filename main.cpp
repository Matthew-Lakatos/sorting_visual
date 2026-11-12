// main.cpp
// ─────────────────────────────────────────────────────────────────────────────
// Changes from previous revision:
//
//  NEW: Preset list menu — user can choose from 9 named demo lists instead of
//       (or before) typing values.  Presets cover int, double, and string types
//       so all visualisation paths are exercisable without manual input.
//
//  NEW: Shell sort (option "i") — after choosing shell sort the user is shown
//       the available gap sequences and can pick one by name.  A "custom" option
//       lets the user type their own descending gap list.
//
//  FIX: visualise() is now callable on string T — all algorithm modules have
//       had the `requires std::is_arithmetic_v<T>` constraint removed, and
//       sfml_visualiser uses rank-based bar heights.
//
//  FIX: run_algorithm() now accepts std::vector<std::string> and correctly
//       instantiates the algorithms on it.  LSD/MSD radix have explicit fallback
//       messaging for string T; all other algorithms work unchanged.
// ─────────────────────────────────────────────────────────────────────────────

import algorithms.bubble_sort;
import algorithms.merge_sort;
import algorithms.heap_sort;
import algorithms.lsd_radix_sort;
import algorithms.msd_radix_sort;
import algorithms.quick_sort;
import algorithms.bogo_sort;
import algorithms.insertion_sort;
import algorithms.shell_sort;

import features.time_misc;
import features.memory_misc;

import <iostream>;
import <vector>;
import <string>;
import <sstream>;
import <limits>;
import <variant>;
import <algorithm>;
import <cctype>;
import <optional>;
import <random>;
import <numeric>;

// ─────────────────────────────────────────────────────────────────────────────
// Preset lists
// ─────────────────────────────────────────────────────────────────────────────

struct PresetInfo {
    std::string name;
    std::string description;
};

static const PresetInfo PRESETS[] = {
    { "0",  "Manual input"                              },
    { "1",  "Random integers (50 elements)"            },
    { "2",  "Nearly sorted integers (50 elements)"     },
    { "3",  "Reverse-sorted integers (50 elements)"    },
    { "4",  "All-equal integers (50 elements)"         },
    { "5",  "Random floats in [0, 1) (50 elements)"    },
    { "6",  "Random words (26 English words)"          },
    { "7",  "Reverse-sorted words (26 English words)"  },
    { "8",  "Small random integers (15 elements)"      },
    { "9",  "Pipe-organ / mountain shape (50 elements)"},
};

enum class ListType { Int, Long, Double, String };

struct TypedList {
    ListType type;
    std::variant<
        std::vector<int>,
        std::vector<long>,
        std::vector<double>,
        std::vector<std::string>
    > data;
};

static TypedList make_preset(int choice) {
    std::mt19937 rng(42); // fixed seed for reproducibility

    switch (choice) {
        case 1: {
            std::uniform_int_distribution<int> dist(1, 500);
            std::vector<int> v(50);
            for (auto& x : v) x = dist(rng);
            return { ListType::Int, v };
        }
        case 2: {
            std::vector<int> v(50);
            std::iota(v.begin(), v.end(), 1);
            // Swap ~10% of elements
            std::uniform_int_distribution<int> di(0, 49);
            for (int i = 0; i < 5; ++i) std::swap(v[di(rng)], v[di(rng)]);
            return { ListType::Int, v };
        }
        case 3: {
            std::vector<int> v(50);
            std::iota(v.begin(), v.end(), 1);
            std::reverse(v.begin(), v.end());
            return { ListType::Int, v };
        }
        case 4: {
            std::vector<int> v(50, 42);
            return { ListType::Int, v };
        }
        case 5: {
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            std::vector<double> v(50);
            for (auto& x : v) x = dist(rng);
            return { ListType::Double, v };
        }
        case 6: {
            std::vector<std::string> words = {
                "banana", "apple", "cherry", "date", "elderberry",
                "fig", "grape", "honeydew", "kiwi", "lemon",
                "mango", "nectarine", "orange", "papaya", "quince",
                "raspberry", "strawberry", "tangerine", "ugli", "vanilla",
                "watermelon", "xigua", "yuzu", "zucchini", "apricot", "blueberry"
            };
            std::shuffle(words.begin(), words.end(), rng);
            return { ListType::String, words };
        }
        case 7: {
            std::vector<std::string> words = {
                "zucchini", "yuzu", "xigua", "watermelon", "vanilla",
                "ugli", "tangerine", "strawberry", "raspberry", "quince",
                "papaya", "orange", "nectarine", "mango", "lemon",
                "kiwi", "honeydew", "grape", "fig", "elderberry",
                "date", "cherry", "banana", "apricot", "apple", "blueberry"
            };
            return { ListType::String, words };
        }
        case 8: {
            std::uniform_int_distribution<int> dist(1, 99);
            std::vector<int> v(15);
            for (auto& x : v) x = dist(rng);
            return { ListType::Int, v };
        }
        case 9: {
            // Pipe-organ: 1 2 3 … 25 25 … 3 2 1
            std::vector<int> v;
            for (int i = 1; i <= 25; ++i) v.push_back(i);
            for (int i = 25; i >= 1; --i) v.push_back(i);
            return { ListType::Int, v };
        }
        default:
            return { ListType::String, std::vector<std::string>{} };
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Manual input parsing
// ─────────────────────────────────────────────────────────────────────────────

static std::string strip_sentinel(const std::string& input) {
    std::istringstream iss(input);
    std::string token, result;
    while (iss >> token) {
        if (token == "/0") break;
        if (!result.empty()) result += ' ';
        result += token;
    }
    return result;
}

static TypedList parse_input(const std::string& raw) {
    std::string cleaned = strip_sentinel(raw);
    std::istringstream iss(cleaned);
    std::vector<std::string> tokens;
    std::string tok;
    while (iss >> tok) tokens.push_back(tok);

    if (tokens.empty())
        return { ListType::String, std::vector<std::string>{} };

    // Try int
    {
        bool ok = true; std::vector<int> v;
        for (const auto& t : tokens) {
            try {
                std::size_t pos;
                long long n = std::stoll(t, &pos);
                if (pos != t.size() || n < std::numeric_limits<int>::min()
                                    || n > std::numeric_limits<int>::max())
                { ok = false; break; }
                v.push_back(static_cast<int>(n));
            } catch (...) { ok = false; break; }
        }
        if (ok) return { ListType::Int, v };
    }
    // Try long
    {
        bool ok = true; std::vector<long> v;
        for (const auto& t : tokens) {
            try {
                std::size_t pos;
                long n = std::stol(t, &pos);
                if (pos != t.size()) { ok = false; break; }
                v.push_back(n);
            } catch (...) { ok = false; break; }
        }
        if (ok) return { ListType::Long, v };
    }
    // Try double
    {
        bool ok = true; std::vector<double> v;
        for (const auto& t : tokens) {
            try {
                std::size_t pos;
                double n = std::stod(t, &pos);
                if (pos != t.size()) { ok = false; break; }
                v.push_back(n);
            } catch (...) { ok = false; break; }
        }
        if (ok) return { ListType::Double, v };
    }
    return { ListType::String, std::vector<std::string>(tokens.begin(), tokens.end()) };
}

// ─────────────────────────────────────────────────────────────────────────────
// Shell sort gap selection helper (called from run_algorithm)
// ─────────────────────────────────────────────────────────────────────────────

static std::pair<GapSequence, std::vector<std::size_t>>
prompt_gap_sequence(std::size_t list_size)
{
    std::cout << "\nChoose gap sequence for Shell sort:\n"
              << "  (1) Ciura      — 701, 301, 132, 57, 23, 10, 4, 1          [recommended]\n"
              << "  (2) Sedgewick  — 1, 5, 19, 41, 109, 209, …                [fast in practice]\n"
              << "  (3) Knuth      — 1, 4, 13, 40, 121, …  (3^k-1)/2          [classic]\n"
              << "  (4) Shell      — N/2, N/4, … 1                             [original 1959]\n"
              << "  (5) Custom     — enter your own descending gaps            [advanced]\n"
              << "> ";

    std::string choice;
    std::getline(std::cin, choice);
    choice.erase(0, choice.find_first_not_of(" \t"));

    if (choice == "2") return { GapSequence::Sedgewick, {} };
    if (choice == "3") return { GapSequence::Knuth,     {} };
    if (choice == "4") return { GapSequence::Shell,     {} };
    if (choice == "5") {
        std::cout << "Enter gap values separated by spaces (descending, must include 1):\n> ";
        std::string line;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        std::vector<std::size_t> gaps;
        std::size_t g;
        while (iss >> g) gaps.push_back(g);
        if (gaps.empty() || gaps.back() != 1) {
            std::cout << "1 not found — appending automatically\n";
            gaps.push_back(1);
        }
        return { GapSequence::Custom, gaps };
    }
    return { GapSequence::Ciura, {} }; // default
}

// ─────────────────────────────────────────────────────────────────────────────
// Algorithm dispatch
// ─────────────────────────────────────────────────────────────────────────────

template <typename T>
static void run_algorithm(std::vector<T> list,
                          const std::string& choice,
                          bool do_visualise)
{
    auto start = take_time();

    if (choice == "a") {
        bubble_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "b") {
        merge_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "bs") {
        merge_sort<T> s{list};
        do_visualise ? s.visualise(true) : (s.sort(true), s.print());
        s.report_memory();
    } else if (choice == "c") {
        insertion_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "d") {
        quick_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "e") {
        heap_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "f") {
        bogo_sort<T> s{list};
        do_visualise ? s.visualise(5000.0, 300) : (void)s.sort();
        s.report_memory();
    } else if (choice == "g") {
        lsd_radix_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "h") {
        msd_radix_sort<T> s{list};
        do_visualise ? s.visualise() : (s.sort(), s.print());
        s.report_memory();
    } else if (choice == "i") {
        // Shell sort: prompt for gap sequence
        auto [seq, custom_gaps] = prompt_gap_sequence(list.size());
        if (seq == GapSequence::Custom) {
            shell_sort<T> s{ list, custom_gaps };
            std::cout << "Using custom gaps: ";
            for (std::size_t g : custom_gaps) std::cout << g << ' ';
            std::cout << '\n';
            do_visualise ? s.visualise() : (s.sort(), s.print());
            s.report_memory();
        } else {
            shell_sort<T> s{ list, seq };
            std::cout << "Using " << s.seq_name() << " gaps: ";
            for (std::size_t g : s.gap_sequence()) std::cout << g << ' ';
            std::cout << '\n';
            do_visualise ? s.visualise() : (s.sort(), s.print());
            s.report_memory();
        }
    } else {
        std::cout << "Unknown algorithm '" << choice << "'\n";
        return;
    }

    auto   end = take_time();
    double ms  = total_time_calc(start, end);
    output_time(ms);
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "\n╔══════════════════════════════════════╗\n"
              <<   "║        Sorting Central  (SFML)       ║\n"
              <<   "╚══════════════════════════════════════╝\n\n";

    // ── Step 1: choose input source ──────────────────────────────────────────
    std::cout << "Choose input source:\n";
    for (const auto& p : PRESETS)
        std::cout << "  (" << p.name << ") " << p.description << '\n';
    std::cout << "> ";

    std::string preset_choice;
    std::getline(std::cin, preset_choice);
    preset_choice.erase(0, preset_choice.find_first_not_of(" \t"));

    TypedList typed;
    bool using_preset = false;

    if (preset_choice != "0" && !preset_choice.empty()) {
        int n = 0;
        try { n = std::stoi(preset_choice); } catch (...) {}
        if (n >= 1 && n <= 9) {
            typed        = make_preset(n);
            using_preset = true;
            std::cout << "\nLoaded preset: " << PRESETS[n].description << '\n';
        }
    }

    if (!using_preset) {
        std::cout << "Enter space-separated values (terminate with /0):\n> ";
        std::string input;
        std::getline(std::cin, input);
        typed = parse_input(input);
    }

    std::cout << "Detected type : ";
    switch (typed.type) {
        case ListType::Int:    std::cout << "int\n";    break;
        case ListType::Long:   std::cout << "long\n";   break;
        case ListType::Double: std::cout << "double\n"; break;
        case ListType::String: std::cout << "string\n"; break;
    }
    std::cout << "List size     : "
              << std::visit([](const auto& v){ return v.size(); }, typed.data)
              << "\n\n";

    // ── Step 2: choose algorithm ──────────────────────────────────────────────
    std::cout << "Choose a sorting algorithm:\n"
              << "  (a)   Bubble sort\n"
              << "  (b)   Merge sort (unstable)\n"
              << "  (bs)  Merge sort (stable)\n"
              << "  (c)   Insertion sort\n"
              << "  (d)   Quick sort\n"
              << "  (e)   Heap sort\n"
              << "  (f)   Bogo sort\n"
              << "  (g)   LSD Radix sort  [integers only]\n"
              << "  (h)   MSD Radix sort  [integers & strings]\n"
              << "  (i)   Shell sort      [gap sequence selectable]\n"
              << "> ";
    std::string algorithm;
    std::getline(std::cin, algorithm);
    for (char& c : algorithm) c = static_cast<char>(std::tolower(c));
    algorithm.erase(0, algorithm.find_first_not_of(" \t"));
    std::cout << '\n';

    // ── Step 3: visualise? ────────────────────────────────────────────────────
    std::cout << "Open SFML visualisation window? (y/n) > ";
    std::string vis_choice;
    std::getline(std::cin, vis_choice);
    bool do_visualise = !vis_choice.empty() && std::tolower(vis_choice[0]) == 'y';
    std::cout << '\n';

    // ── Step 4: dispatch ──────────────────────────────────────────────────────
    std::visit([&](auto& list) {
        run_algorithm(std::move(list), algorithm, do_visualise);
    }, typed.data);

    // ── Step 5: complexity info ───────────────────────────────────────────────
    static const std::pair<std::string, std::string> algo_map[] = {
        {"a","bubble"},         {"b","unstable_merge"}, {"bs","stable_merge"},
        {"c","insertion"},      {"d","naive_quick"},    {"e","heap"},
        {"f","bogo"},           {"g","lsd_radix"},      {"h","msd_radix"},
        {"i","shell"},
    };
    for (const auto& [key, name] : algo_map) {
        if (algorithm == key) {
            std::cout << '\n';
            output_time_complexity(name);
            output_memory_complexity(name);
            break;
        }
    }

    return 0;
}
