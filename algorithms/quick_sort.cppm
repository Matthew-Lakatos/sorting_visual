// algorithms/quick_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
// FIX: std::to_string(pivot) / std::to_string(v[r]) etc. — replaced with
//      value_to_str() so string T compiles.

export module algorithms.quick_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <cmath>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class quick_sort {
public:
    explicit quick_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        if (m_list.empty()) return;
        quicksort(m_list, 0, m_list.size() - 1);
    }

    // FIX: no arithmetic constraint
    void visualise() {
        if (m_list.empty()) return;
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        quicksort_frames(work, 0, work.size() - 1, frames);
        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < work.size(); ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));
        m_list = work;
        SortVisualiser::run(frames, "Quick Sort");
    }

    void report_memory() const {
        std::size_t rec = static_cast<std::size_t>(
            std::log2(m_list.size() > 1 ? (double)m_list.size() : 2.0))
            * 3 * sizeof(std::size_t);
        std::cout << "Quick sort memory: "
                  << m_list.capacity() * sizeof(T) + rec << " B\n";
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

    static void quicksort(std::vector<T>& v, std::size_t l, std::size_t r) {
        if (l >= r) return;
        std::size_t pi = partition(v, l, r);
        if (pi > 0) quicksort(v, l, pi - 1);
        quicksort(v, pi + 1, r);
    }

    static std::size_t partition(std::vector<T>& v, std::size_t l, std::size_t r) {
        T pivot = v[r];
        std::size_t i = l;
        for (std::size_t j = l; j < r; ++j)
            if (v[j] < pivot) std::swap(v[i++], v[j]);
        std::swap(v[i], v[r]);
        return i;
    }

    static void quicksort_frames(std::vector<T>& v,
                                  std::size_t l, std::size_t r,
                                  std::vector<SortFrame<T>>& frames) {
        if (l >= r) {
            SortFrame<T> f;
            f.values = v;
            f.highlights.push_back({ l, HighlightRole::Sorted });
            f.label = "Single element [" + std::to_string(l)
                      + "] = " + value_to_str(v[l]) + " in place";
            frames.push_back(std::move(f));
            return;
        }

        // Show pivot selection
        // FIX: value_to_str(v[r]) instead of std::to_string(v[r])
        {
            SortFrame<T> f;
            f.values = v;
            f.highlights.push_back({ r, HighlightRole::Pivot });
            f.label = "Pivot = " + value_to_str(v[r])
                      + "  partitioning [" + std::to_string(l)
                      + ".." + std::to_string(r) + "]";
            frames.push_back(std::move(f));
        }

        T pivot = v[r];
        std::size_t i = l;
        for (std::size_t j = l; j < r; ++j) {
            {
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ r, HighlightRole::Pivot   });
                f.highlights.push_back({ j, HighlightRole::Compare });
                f.highlights.push_back({ i, HighlightRole::Compare });
                // FIX: value_to_str(v[j]) and value_to_str(pivot)
                f.label = "Compare " + value_to_str(v[j])
                          + " < pivot " + value_to_str(pivot) + " ?";
                frames.push_back(std::move(f));
            }
            if (v[j] < pivot) {
                std::swap(v[i], v[j]);
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ r, HighlightRole::Pivot });
                f.highlights.push_back({ i, HighlightRole::Swap  });
                f.highlights.push_back({ j, HighlightRole::Swap  });
                f.label = "Swap [" + std::to_string(i)
                          + "] and [" + std::to_string(j) + "]";
                frames.push_back(std::move(f));
                ++i;
            }
        }
        std::swap(v[i], v[r]);
        {
            SortFrame<T> f;
            f.values = v;
            f.highlights.push_back({ i, HighlightRole::Sorted });
            f.label = "Pivot " + value_to_str(v[i])
                      + " placed at [" + std::to_string(i) + "]";
            frames.push_back(std::move(f));
        }

        if (i > 0) quicksort_frames(v, l, i - 1, frames);
        quicksort_frames(v, i + 1, r, frames);
    }
};
