// algorithms/lsd_radix_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — this prevented
//      compilation for std::string even before the runtime branch was hit.
//      Changed to an unconditional function with a compile-time if constexpr
//      branch: integral types get the full SFML frame visualisation; all other
//      types (including string) fall back to sort() + a single sorted frame.
// FIX: Labels used std::to_string(sub[i]) — replaced with value_to_str().
//      (Only reached for integral T, but keeps the pattern consistent.)

export module algorithms.lsd_radix_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <cmath>;
import <type_traits>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class lsd_radix_sort {
public:
    explicit lsd_radix_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            std::vector<T> neg, pos;
            for (T v : m_list) {
                if (v < T{}) neg.push_back(-v);
                else         pos.push_back(v);
            }
            lsd_radix(neg);
            lsd_radix(pos);
            std::reverse(neg.begin(), neg.end());
            for (T& v : neg) v = -v;
            m_list = std::move(neg);
            m_list.insert(m_list.end(), pos.begin(), pos.end());
        } else {
            std::cout << "[lsd_radix_sort] Non-integral type — falling back to std::sort\n";
            std::sort(m_list.begin(), m_list.end());
        }
    }

    // FIX: no longer requires arithmetic — non-integral path shows single sorted frame
    void visualise() {
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            std::vector<SortFrame<T>> frames;
            std::vector<T> work = m_list;

            std::vector<T> neg, pos;
            for (T v : work) {
                if (v < T{}) neg.push_back(-v);
                else         pos.push_back(v);
            }

            lsd_radix_frames(pos, frames, "positives");
            lsd_radix_frames(neg, frames, "negatives (abs)");

            std::reverse(neg.begin(), neg.end());
            for (T& v : neg) v = -v;
            work = std::move(neg);
            work.insert(work.end(), pos.begin(), pos.end());
            m_list = work;

            SortFrame<T> ff;
            ff.values = work;
            for (std::size_t k = 0; k < work.size(); ++k)
                ff.highlights.push_back({ k, HighlightRole::Sorted });
            ff.label = "Sorted!";
            frames.push_back(std::move(ff));

            SortVisualiser::run(frames, "LSD Radix Sort");
        } else {
            // Non-integral (string, float, etc.): sort and show single frame
            std::cout << "[lsd_radix_sort] LSD radix requires integers — "
                         "showing sorted result only\n";
            sort();
            std::vector<SortFrame<T>> frames;
            SortFrame<T> f;
            f.values = m_list;
            for (std::size_t k = 0; k < m_list.size(); ++k)
                f.highlights.push_back({ k, HighlightRole::Sorted });
            f.label = "Sorted (LSD radix not applicable — used std::sort)";
            frames.push_back(std::move(f));
            SortVisualiser::run(frames, "LSD Radix Sort (fallback)");
        }
    }

    void report_memory() const {
        std::cout << "LSD radix sort memory: "
                  << m_list.capacity() * sizeof(T)
                   + 10 * m_list.size() * sizeof(T)
                  << " B\n";
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

    static void lsd_radix(std::vector<T>& list) {
        if (list.empty()) return;
        if constexpr (std::is_integral_v<T>) {
            T max_val = *std::max_element(list.begin(), list.end());
            // Use long long for exp to avoid signed overflow when T is int/short
            for (long long exp = 1;
                 static_cast<long long>(max_val) / exp > 0;
                 exp *= 10)
            {
                std::vector<std::vector<T>> buckets(10);
                for (T n : list)
                    buckets[static_cast<int>((static_cast<long long>(n) / exp) % 10)]
                        .push_back(n);
                list.clear();
                for (auto& b : buckets)
                    list.insert(list.end(), b.begin(), b.end());
            }
        }
    }

    static void lsd_radix_frames(std::vector<T>& sub,
                                  std::vector<SortFrame<T>>& frames,
                                  const std::string& part_label) {
        if (sub.empty()) return;
        if constexpr (!std::is_integral_v<T>) return;

        T max_val = *std::max_element(sub.begin(), sub.end());
        if (max_val == T{}) return;

        int digit = 0;
        // Use long long for exp to avoid signed overflow when T is int/short
        for (long long exp = 1;
             static_cast<long long>(max_val) / exp > 0;
             exp *= 10, ++digit)
        {
            std::vector<std::vector<T>> buckets(10);
            for (std::size_t i = 0; i < sub.size(); ++i) {
                int d = static_cast<int>(
                    (static_cast<long long>(sub[i]) / exp) % 10);
                buckets[d].push_back(sub[i]);
                SortFrame<T> f;
                f.values = sub;
                f.highlights.push_back({ i, HighlightRole::Bucket });
                f.label = part_label + "  digit=" + std::to_string(digit)
                          + "  " + value_to_str(sub[i])
                          + " \u2192 bucket [" + std::to_string(d) + "]";
                frames.push_back(std::move(f));
            }
            sub.clear();
            for (auto& b : buckets)
                sub.insert(sub.end(), b.begin(), b.end());
            SortFrame<T> f;
            f.values = sub;
            f.label  = part_label + "  after digit-" + std::to_string(digit) + " pass";
            frames.push_back(std::move(f));
        }
    }
};
