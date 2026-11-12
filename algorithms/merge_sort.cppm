// algorithms/merge_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
// FIX: std::to_string(lh[i]) / std::to_string(rh[j]) in labels — replaced
//      with value_to_str() so string T compiles.

export module algorithms.merge_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class merge_sort {
public:
    explicit merge_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort(bool stable = false) {
        if (m_list.empty()) return;
        if (stable) stable_sort_impl(m_list, 0, m_list.size() - 1);
        else        unstable_sort_impl(m_list, 0, m_list.size() - 1);
    }

    // FIX: no arithmetic constraint
    void visualise(bool stable = false) {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        if (!work.empty()) {
            if (stable) stable_sort_frames(work, 0, work.size() - 1, frames);
            else        unstable_sort_frames(work, 0, work.size() - 1, frames);
        }
        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < work.size(); ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));
        m_list = work;
        SortVisualiser::run(frames, stable ? "Merge Sort (Stable)" : "Merge Sort (Unstable)");
    }

    void report_memory() const {
        std::cout << "Merge sort memory: "
                  << m_list.capacity() * sizeof(T) * 2 << " B (includes aux)\n";
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

    // ── non-recording sort ────────────────────────────────────────────────────
    static void stable_sort_impl(std::vector<T>& v, std::size_t l, std::size_t r) {
        if (l >= r) return;
        std::size_t m = (l + r) / 2;
        stable_sort_impl(v, l, m);
        stable_sort_impl(v, m + 1, r);
        std::vector<T> lh(v.begin() + l, v.begin() + m + 1);
        std::vector<T> rh(v.begin() + m + 1, v.begin() + r + 1);
        std::size_t i = 0, j = 0, k = l;
        while (i < lh.size() && j < rh.size())
            v[k++] = (lh[i] <= rh[j]) ? lh[i++] : rh[j++];
        while (i < lh.size())  v[k++] = lh[i++];
        while (j < rh.size())  v[k++] = rh[j++];
    }

    static void unstable_sort_impl(std::vector<T>& v, std::size_t l, std::size_t r) {
        if (l >= r) return;
        std::size_t m = (l + r) / 2;
        unstable_sort_impl(v, l, m);
        unstable_sort_impl(v, m + 1, r);
        std::vector<T> tmp;
        std::size_t i = l, j = m + 1;
        while (i <= m && j <= r)
            tmp.push_back(v[i] < v[j] ? v[i++] : v[j++]);
        while (i <= m)  tmp.push_back(v[i++]);
        while (j <= r)  tmp.push_back(v[j++]);
        std::copy(tmp.begin(), tmp.end(), v.begin() + l);
    }

    // ── frame-recording merge ─────────────────────────────────────────────────
    static void merge_frames(std::vector<T>& v,
                              std::size_t l, std::size_t m, std::size_t r,
                              std::vector<SortFrame<T>>& frames,
                              bool stable) {
        std::vector<T> lh(v.begin() + l, v.begin() + m + 1);
        std::vector<T> rh(v.begin() + m + 1, v.begin() + r + 1);
        std::size_t i = 0, j = 0, k = l;

        while (i < lh.size() && j < rh.size()) {
            {
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ l + i,     HighlightRole::Compare });
                f.highlights.push_back({ m + 1 + j, HighlightRole::Compare });
                // FIX: value_to_str instead of std::to_string
                f.label = "Merging [" + std::to_string(l) + ".."
                          + std::to_string(r) + "]  "
                          + value_to_str(lh[i]) + " vs " + value_to_str(rh[j]);
                frames.push_back(std::move(f));
            }
            bool take_left = stable ? (lh[i] <= rh[j]) : (lh[i] < rh[j]);
            v[k++] = take_left ? lh[i++] : rh[j++];
            {
                SortFrame<T> f;
                f.values = v;
                f.highlights.push_back({ k - 1, HighlightRole::Swap });
                f.label = "Placed " + value_to_str(v[k - 1])
                          + " at [" + std::to_string(k - 1) + "]";
                frames.push_back(std::move(f));
            }
        }
        while (i < lh.size()) v[k++] = lh[i++];
        while (j < rh.size()) v[k++] = rh[j++];

        SortFrame<T> f;
        f.values = v;
        for (std::size_t x = l; x <= r; ++x)
            f.highlights.push_back({ x, HighlightRole::Sorted });
        f.label = "Merged [" + std::to_string(l) + ".." + std::to_string(r) + "]";
        frames.push_back(std::move(f));
    }

    static void stable_sort_frames(std::vector<T>& v,
                                    std::size_t l, std::size_t r,
                                    std::vector<SortFrame<T>>& frames) {
        if (l >= r) return;
        std::size_t m = (l + r) / 2;
        stable_sort_frames(v, l, m, frames);
        stable_sort_frames(v, m + 1, r, frames);
        merge_frames(v, l, m, r, frames, true);
    }

    static void unstable_sort_frames(std::vector<T>& v,
                                      std::size_t l, std::size_t r,
                                      std::vector<SortFrame<T>>& frames) {
        if (l >= r) return;
        std::size_t m = (l + r) / 2;
        unstable_sort_frames(v, l, m, frames);
        unstable_sort_frames(v, m + 1, r, frames);
        merge_frames(v, l, m, r, frames, false);
    }
};
