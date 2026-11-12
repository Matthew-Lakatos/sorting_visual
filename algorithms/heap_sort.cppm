// algorithms/heap_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
// FIX: std::to_string(work[0]) / std::to_string(work[i]) in labels — replaced
//      with value_to_str() so string T compiles.

export module algorithms.heap_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <cmath>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class heap_sort {
public:
    explicit heap_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        if (m_list.empty()) return;
        for (std::size_t i = m_list.size() / 2; i-- > 0;)
            sift_down(m_list, i, m_list.size());
        for (std::size_t i = m_list.size() - 1; i > 0; --i) {
            std::swap(m_list[0], m_list[i]);
            sift_down(m_list, 0, i);
        }
    }

    // FIX: no arithmetic constraint
    void visualise() {
        if (m_list.empty()) return;
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        const std::size_t n = work.size();

        // Phase 1: build max-heap
        {
            SortFrame<T> f;
            f.values = work;
            f.label  = "Phase 1: Building max-heap";
            frames.push_back(std::move(f));
        }
        for (std::size_t i = n / 2; i-- > 0;)
            sift_down_frames(work, i, n, frames, "Heapify");
        {
            SortFrame<T> f;
            f.values = work;
            f.label  = "Max-heap built — root "
                       + value_to_str(work[0]) + " is largest";
            frames.push_back(std::move(f));
        }

        // Phase 2: extract
        for (std::size_t i = n - 1; i > 0; --i) {
            {
                SortFrame<T> f;
                f.values = work;
                f.highlights.push_back({ 0, HighlightRole::Swap });
                f.highlights.push_back({ i, HighlightRole::Swap });
                for (std::size_t k = i + 1; k < n; ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                // FIX: value_to_str(work[0])
                f.label = "Swap root " + value_to_str(work[0])
                          + " with [" + std::to_string(i) + "]";
                frames.push_back(std::move(f));
            }
            std::swap(work[0], work[i]);
            {
                SortFrame<T> f;
                f.values = work;
                for (std::size_t k = i; k < n; ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                // FIX: value_to_str(work[i])
                f.label = value_to_str(work[i])
                          + " placed at [" + std::to_string(i) + "]";
                frames.push_back(std::move(f));
            }
            sift_down_frames(work, 0, i, frames, "Re-heap");
        }

        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < n; ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));

        m_list = work;
        SortVisualiser::run(frames, "Heap Sort");
    }

    void report_memory() const {
        std::size_t rec = static_cast<std::size_t>(
            std::log2(m_list.size() > 1 ? (double)m_list.size() : 2.0))
            * 3 * sizeof(std::size_t);
        std::cout << "Heap sort memory: "
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

    static void sift_down(std::vector<T>& v,
                           std::size_t node, std::size_t heap_size) {
        while (true) {
            std::size_t largest = node;
            std::size_t l = 2 * node + 1, r = 2 * node + 2;
            if (l < heap_size && v[l] > v[largest]) largest = l;
            if (r < heap_size && v[r] > v[largest]) largest = r;
            if (largest == node) break;
            std::swap(v[node], v[largest]);
            node = largest;
        }
    }

    static void sift_down_frames(std::vector<T>& v,
                                  std::size_t node, std::size_t heap_size,
                                  std::vector<SortFrame<T>>& frames,
                                  const std::string& phase) {
        while (true) {
            std::size_t largest = node;
            std::size_t lc = 2 * node + 1, rc = 2 * node + 2;
            if (lc < heap_size && v[lc] > v[largest]) largest = lc;
            if (rc < heap_size && v[rc] > v[largest]) largest = rc;

            {
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ node, HighlightRole::Compare });
                if (lc < heap_size) f.highlights.push_back({ lc, HighlightRole::Compare });
                if (rc < heap_size) f.highlights.push_back({ rc, HighlightRole::Compare });
                for (std::size_t k = heap_size; k < v.size(); ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                // FIX: value_to_str(v[node])
                f.label = phase + ": node [" + std::to_string(node)
                          + "] = " + value_to_str(v[node]);
                frames.push_back(std::move(f));
            }
            if (largest == node) break;
            std::swap(v[node], v[largest]);
            {
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ node,    HighlightRole::Swap });
                f.highlights.push_back({ largest, HighlightRole::Swap });
                for (std::size_t k = heap_size; k < v.size(); ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                f.label = phase + ": swap [" + std::to_string(node)
                          + "] and [" + std::to_string(largest) + "]";
                frames.push_back(std::move(f));
            }
            node = largest;
        }
    }
};
