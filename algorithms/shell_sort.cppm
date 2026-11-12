// algorithms/shell_sort.cppm
// ─────────────────────────────────────────────────────────────────────────────
// Shell sort with selectable gap sequences.
//
// Gap sequences available (pass GapSequence enum to sort() / visualise()):
//   Shell      — N/2, N/4, … 1  (original Shell 1959, O(n²) worst)
//   Knuth      — 1, 4, 13, 40, 121, …  (3^k−1)/2  (O(n^(3/2)))
//   Sedgewick  — 1, 5, 19, 41, 109, 209, …  (O(n^(4/3)) amortised)
//   Ciura      — 1, 4, 10, 23, 57, 132, 301, 701  (experimentally best known)
//   Custom     — caller provides an explicit list of gaps in descending order
//
// Visualisation:
//   Each (element, gap-partner) comparison is a frame.  The two elements being
//   insertion-sorted at gap distance are highlighted Gap (orange).  When a swap
//   occurs the pair is briefly highlighted Swap (red).  Fully sorted elements
//   at the end of the sequence are highlighted Sorted (green).
// ─────────────────────────────────────────────────────────────────────────────

export module algorithms.shell_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <cmath>;
import <cstddef>;
import <concepts>;
import <iostream>;

export enum class GapSequence {
    Shell,      // N/2, N/4, … 1
    Knuth,      // (3^k - 1) / 2
    Sedgewick,  // mixed formula, empirically fast
    Ciura,      // best known experimentally
    Custom,     // user supplies gaps vector
};

export template <typename T>
    requires std::totally_ordered<T>
class shell_sort {
public:
    // ── constructors ──────────────────────────────────────────────────────────

    // Standard construction: choose a named gap sequence.
    explicit shell_sort(std::vector<T> list,
                        GapSequence seq = GapSequence::Ciura)
        : m_list(std::move(list)), m_seq(seq)
    {}

    // Custom gap construction: caller provides gaps in descending order.
    // e.g. shell_sort<int>({…}, {701, 301, 132, 57, 23, 10, 4, 1})
    shell_sort(std::vector<T> list, std::vector<std::size_t> custom_gaps)
        : m_list(std::move(list)),
          m_seq(GapSequence::Custom),
          m_custom_gaps(std::move(custom_gaps))
    {}

    // ── sort ──────────────────────────────────────────────────────────────────

    void sort() {
        std::vector<std::size_t> gaps = build_gaps(m_list.size());
        do_sort(m_list, gaps, nullptr);
    }

    // ── visualise ─────────────────────────────────────────────────────────────

    void visualise() {
        std::vector<std::size_t> gaps = build_gaps(m_list.size());
        std::vector<SortFrame<T>> frames;

        // Opening frame: show the gap sequence being used
        {
            SortFrame<T> f;
            f.values = m_list;
            f.label  = "Shell Sort — gaps: " + gaps_to_str(gaps);
            frames.push_back(std::move(f));
        }

        do_sort(m_list, gaps, &frames);

        // Final sorted frame
        SortFrame<T> ff;
        ff.values = m_list;
        for (std::size_t k = 0; k < m_list.size(); ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));

        std::string title = "Shell Sort (" + seq_name() + ")";
        SortVisualiser::run(frames, title);
    }

    void report_memory() const {
        std::cout << "Shell sort memory: "
                  << m_list.capacity() * sizeof(T) << " B (in-place)\n";
    }

    void print() const {
        std::cout << "[ ";
        for (std::size_t i = 0; i < m_list.size(); ++i) {
            std::cout << m_list[i];
            if (i + 1 < m_list.size()) std::cout << ", ";
        }
        std::cout << " ]\n";
    }

    // Returns the gap sequence that will be used for this instance's list size
    std::vector<std::size_t> gap_sequence() const {
        return build_gaps(m_list.size());
    }

    std::string seq_name() const {
        switch (m_seq) {
            case GapSequence::Shell:     return "Shell";
            case GapSequence::Knuth:     return "Knuth";
            case GapSequence::Sedgewick: return "Sedgewick";
            case GapSequence::Ciura:     return "Ciura";
            case GapSequence::Custom:    return "Custom";
        }
        return "Unknown";
    }

private:
    std::vector<T>           m_list;
    GapSequence              m_seq;
    std::vector<std::size_t> m_custom_gaps; // only used when m_seq == Custom

    // ── gap sequence builders ─────────────────────────────────────────────────

    std::vector<std::size_t> build_gaps(std::size_t n) const {
        switch (m_seq) {
            case GapSequence::Shell:     return gaps_shell(n);
            case GapSequence::Knuth:     return gaps_knuth(n);
            case GapSequence::Sedgewick: return gaps_sedgewick(n);
            case GapSequence::Ciura:     return gaps_ciura(n);
            case GapSequence::Custom:    return validated_custom(n);
        }
        return gaps_ciura(n);
    }

    static std::vector<std::size_t> gaps_shell(std::size_t n) {
        std::vector<std::size_t> g;
        for (std::size_t h = n / 2; h >= 1; h /= 2)
            g.push_back(h);
        return g;
    }

    static std::vector<std::size_t> gaps_knuth(std::size_t n) {
        // Grow sequence (3^k − 1) / 2 up to ≤ n/3
        std::vector<std::size_t> g;
        std::size_t h = 1;
        while (h <= n / 3) {
            g.push_back(h);
            h = h * 3 + 1;
        }
        std::reverse(g.begin(), g.end());
        if (g.empty()) g.push_back(1);
        return g;
    }

    static std::vector<std::size_t> gaps_sedgewick(std::size_t n) {
        // Two interleaved formulas:
        //   4^k + 3·2^(k−1) + 1   (k ≥ 1)
        //   9·(2^k − 2^(k/2)) + 1  (k ≥ 0, even)
        // Pre-computed up to values needed for lists ≤ 10^9 elements:
        static const std::size_t table[] = {
            1, 5, 19, 41, 109, 209, 505, 929, 2161, 3905,
            8929, 16001, 36289, 64769, 146305, 260609,
            587521, 1045505, 2354689, 4188161,
            9427969, 16764929, 37730305, 67084289
        };
        std::vector<std::size_t> g;
        for (std::size_t v : table)
            if (v < n) g.push_back(v);
        if (g.empty()) g.push_back(1);
        std::reverse(g.begin(), g.end());
        return g;
    }

    static std::vector<std::size_t> gaps_ciura(std::size_t n) {
        static const std::size_t ciura[] = {
            701, 301, 132, 57, 23, 10, 4, 1
        };
        std::vector<std::size_t> g;
        for (std::size_t v : ciura)
            if (v < n) g.push_back(v);
        if (g.empty()) g.push_back(1);
        return g;
    }

    std::vector<std::size_t> validated_custom(std::size_t /*n*/) const {
        if (m_custom_gaps.empty()) {
            std::cout << "[shell_sort] Custom gap list is empty — falling back to Ciura\n";
            return gaps_ciura(m_list.size());
        }
        // Ensure 1 is in the sequence; if the caller omitted it, append it
        std::vector<std::size_t> g = m_custom_gaps;
        if (g.back() != 1) g.push_back(1);
        return g;
    }

    // ── core sort logic ───────────────────────────────────────────────────────
    // frames == nullptr → no recording (plain sort path)

    static void do_sort(std::vector<T>& v,
                        const std::vector<std::size_t>& gaps,
                        std::vector<SortFrame<T>>* frames)
    {
        const std::size_t n = v.size();
        for (std::size_t gap : gaps) {
            if (frames) {
                // Announce this gap pass
                SortFrame<T> gf;
                gf.values = v;
                gf.label  = "Gap = " + std::to_string(gap);
                frames->push_back(std::move(gf));
            }

            // Insertion sort at this gap distance
            for (std::size_t i = gap; i < n; ++i) {
                T key = v[i];
                std::size_t j = i;

                // Show the element being picked up
                if (frames) {
                    SortFrame<T> f;
                    f.values = v;
                    f.highlights.push_back({ i, HighlightRole::Gap });
                    if (j >= gap)
                        f.highlights.push_back({ j - gap, HighlightRole::Gap });
                    f.label = "gap=" + std::to_string(gap)
                              + "  picking [" + std::to_string(i)
                              + "] = " + value_to_str(key);
                    frames->push_back(std::move(f));
                }

                while (j >= gap && v[j - gap] > key) {
                    // Show the comparison / shift
                    if (frames) {
                        SortFrame<T> f;
                        f.values = v;
                        f.highlights.push_back({ j,       HighlightRole::Swap });
                        f.highlights.push_back({ j - gap, HighlightRole::Swap });
                        f.label = "gap=" + std::to_string(gap)
                                  + "  shift [" + std::to_string(j - gap)
                                  + "] = " + value_to_str(v[j - gap])
                                  + " right";
                        frames->push_back(std::move(f));
                    }

                    v[j] = v[j - gap];
                    j -= gap;
                }
                v[j] = key;

                // Show the placement
                if (frames) {
                    SortFrame<T> f;
                    f.values = v;
                    f.highlights.push_back({ j, HighlightRole::Gap });
                    f.label = "gap=" + std::to_string(gap)
                              + "  placed " + value_to_str(key)
                              + " at [" + std::to_string(j) + "]";
                    frames->push_back(std::move(f));
                }
            }

            // After each full gap pass, mark the progressively-sorted state.
            // After the gap=1 pass the array is fully sorted.
            if (frames && gap == 1) {
                SortFrame<T> f;
                f.values = v;
                for (std::size_t k = 0; k < n; ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                f.label = "Gap=1 pass complete — array fully sorted";
                frames->push_back(std::move(f));
            }
        }
    }

    // ── helpers ───────────────────────────────────────────────────────────────

    static std::string gaps_to_str(const std::vector<std::size_t>& gaps) {
        std::string s = "[";
        for (std::size_t i = 0; i < gaps.size(); ++i) {
            if (i) s += ", ";
            s += std::to_string(gaps[i]);
        }
        return s + "]";
    }
};
