// algorithms/bubble_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
//      Now works for std::string and any totally_ordered T.
// FIX: HUD labels used std::to_string(T) — replaced with value_to_str().

export module algorithms.bubble_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class bubble_sort {
public:
    explicit bubble_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        const std::size_t n = m_list.size();
        for (std::size_t i = 0; i < n; ++i) {
            bool swapped = false;
            for (std::size_t j = 0; j < n - i - 1; ++j) {
                if (m_list[j] > m_list[j + 1]) {
                    std::swap(m_list[j], m_list[j + 1]);
                    swapped = true;
                }
            }
            if (!swapped) break;
        }
    }

    // FIX: no longer constrained to arithmetic T
    void visualise() {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        const std::size_t n = work.size();

        for (std::size_t i = 0; i < n; ++i) {
            bool swapped = false;
            for (std::size_t j = 0; j < n - i - 1; ++j) {
                // Compare frame
                {
                    SortFrame<T> f;
                    f.values = work;
                    f.highlights = {{ j, HighlightRole::Compare },
                                    { j + 1, HighlightRole::Compare }};
                    f.label = "Pass " + std::to_string(i + 1)
                              + "  comparing [" + std::to_string(j)
                              + "] " + value_to_str(work[j])
                              + " vs " + value_to_str(work[j + 1]);
                    frames.push_back(std::move(f));
                }
                if (work[j] > work[j + 1]) {
                    std::swap(work[j], work[j + 1]);
                    swapped = true;
                    SortFrame<T> f;
                    f.values = work;
                    f.highlights = {{ j, HighlightRole::Swap },
                                    { j + 1, HighlightRole::Swap }};
                    f.label = "Pass " + std::to_string(i + 1)
                              + "  swapped [" + std::to_string(j)
                              + "] and [" + std::to_string(j + 1) + "]";
                    frames.push_back(std::move(f));
                }
            }
            // Mark sorted tail
            {
                SortFrame<T> f;
                f.values = work;
                for (std::size_t k = n - i - 1; k < n; ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                f.label = "Pass " + std::to_string(i + 1) + " complete";
                frames.push_back(std::move(f));
            }
            if (!swapped) break;
        }

        // Final frame
        SortFrame<T> f;
        f.values = work;
        for (std::size_t k = 0; k < n; ++k)
            f.highlights.push_back({ k, HighlightRole::Sorted });
        f.label = "Sorted!";
        frames.push_back(std::move(f));

        m_list = work;
        SortVisualiser::run(frames, "Bubble Sort");
    }

    void report_memory() const {
        std::cout << "Bubble sort memory: " << m_list.capacity() * sizeof(T) << " B\n";
    }

    void print() const {
        std::cout << "[ ";
        for (std::size_t i = 0; i < m_list.size(); ++i) {
            std::cout << m_list[i];
            if (i + 1 < m_list.size()) std::cout << ", ";
        }
        std::cout << " ]\n";
    }

private:
    std::vector<T> m_list;
};
