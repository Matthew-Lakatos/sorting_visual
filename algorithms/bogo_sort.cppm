// algorithms/bogo_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
//      Labels only use attempt counts, not T values, so no further changes needed.

export module algorithms.bogo_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <chrono>;
import <random>;
import <ranges>;
import <string>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class bogo_sort {
public:
    explicit bogo_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort(double max_ms = 5000.0) {
        std::mt19937 gen(std::random_device{}());
        auto start = std::chrono::steady_clock::now();
        while (!std::ranges::is_sorted(m_list)) {
            std::ranges::shuffle(m_list, gen);
            double e = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
            if (e >= max_ms) {
                std::cout << "Bogo sort timed out after " << max_ms << " ms\n";
                return;
            }
        }
    }

    // FIX: no arithmetic constraint
    void visualise(double max_ms = 5000.0, std::size_t max_frames = 300) {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        std::mt19937 gen(std::random_device{}());
        auto start    = std::chrono::steady_clock::now();
        std::size_t   attempt = 0;
        bool          capped  = false;

        while (!std::ranges::is_sorted(work)) {
            double e = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count();
            if (e >= max_ms) {
                SortFrame<T> f;
                f.values = work;
                f.label  = "Timed out after " + std::to_string(attempt) + " attempts";
                frames.push_back(std::move(f));
                break;
            }
            std::ranges::shuffle(work, gen);
            ++attempt;

            if (!capped) {
                std::vector<T> sorted_ref = work;
                std::sort(sorted_ref.begin(), sorted_ref.end());
                SortFrame<T> f;
                f.values = work;
                for (std::size_t k = 0; k < work.size(); ++k)
                    f.highlights.push_back({ k,
                        work[k] == sorted_ref[k]
                            ? HighlightRole::Sorted
                            : HighlightRole::Compare });
                f.label = "Attempt " + std::to_string(attempt) + " — not sorted";
                frames.push_back(std::move(f));

                if (frames.size() >= max_frames) {
                    capped = true;
                    SortFrame<T> cf;
                    cf.values = work;
                    cf.label  = "Frame limit reached — still shuffling...";
                    frames.push_back(std::move(cf));
                }
            }
        }

        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < work.size(); ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted after " + std::to_string(attempt) + " attempts!";
        frames.push_back(std::move(ff));

        m_list = work;
        SortVisualiser::run(frames, "Bogo Sort");
    }

    void report_memory() const {
        std::cout << "Bogo sort memory: "
                  << m_list.capacity() * sizeof(T) + 2500 << " B\n";
    }

    void print() const {
        std::cout << "[ ";
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            std::cout << *it;
            if (std::next(it) != m_list.end()) std::cout << ", ";
        }
        std::cout << " ]\n";
    }

private:
    std::vector<T> m_list;
};
