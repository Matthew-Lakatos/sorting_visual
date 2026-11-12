// algorithms/msd_radix_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
//      std::string T now gets a proper character-by-character bucket visualisation.
//      Non-integral, non-string arithmetic types (float/double) get the sort +
//      single sorted frame fallback.
// FIX: std::to_string(sub[i]) in labels — replaced with value_to_str().

export module algorithms.msd_radix_sort;

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
class msd_radix_sort {
public:
    explicit msd_radix_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        if (m_list.empty()) return;
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            std::vector<T> neg, pos;
            for (T v : m_list) {
                if (v < T{}) neg.push_back(-v);
                else         pos.push_back(v);
            }
            T max_p = pos.empty() ? T{1} : *std::max_element(pos.begin(), pos.end());
            T max_n = neg.empty() ? T{1} : *std::max_element(neg.begin(), neg.end());
            msd_radix(pos, count_digits(max_p) - 1);
            msd_radix(neg, count_digits(max_n) - 1);
            std::reverse(neg.begin(), neg.end());
            for (T& v : neg) v = -v;
            m_list = std::move(neg);
            m_list.insert(m_list.end(), pos.begin(), pos.end());
        } else if constexpr (std::is_same_v<T, std::string>) {
            msd_string(m_list, 0);
        } else {
            std::sort(m_list.begin(), m_list.end());
        }
    }

    // FIX: no arithmetic constraint; three dispatch paths:
    //   1. Integral → full bucket-per-digit animation
    //   2. std::string → full bucket-per-character animation (NEW)
    //   3. Everything else → sort + single sorted frame
    void visualise() {
        if (m_list.empty()) return;

        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            visualise_integral();
        } else if constexpr (std::is_same_v<T, std::string>) {
            visualise_string();
        } else {
            std::cout << "[msd_radix_sort] MSD radix requires integers or strings — "
                         "showing sorted result only\n";
            sort();
            std::vector<SortFrame<T>> frames;
            SortFrame<T> f;
            f.values = m_list;
            for (std::size_t k = 0; k < m_list.size(); ++k)
                f.highlights.push_back({ k, HighlightRole::Sorted });
            f.label = "Sorted (MSD radix not applicable — used std::sort)";
            frames.push_back(std::move(f));
            SortVisualiser::run(frames, "MSD Radix Sort (fallback)");
        }
    }

    void report_memory() const {
        int depth = 1;
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
            // count_digits and T{1}/T{10} are only valid for integral types
            T max_val = m_list.empty() ? T{1}
                        : *std::max_element(m_list.begin(), m_list.end());
            depth = count_digits(max_val > T{} ? max_val : T{1});
        } else if constexpr (std::is_same_v<T, std::string>) {
            // Depth ≈ average string length — use longest string as proxy
            if (!m_list.empty()) {
                auto it = std::max_element(m_list.begin(), m_list.end(),
                    [](const T& a, const T& b){ return a.size() < b.size(); });
                depth = static_cast<int>(it->size());
            }
        }
        std::cout << "MSD radix sort memory: "
                  << m_list.capacity() * sizeof(T)
                   + static_cast<std::size_t>(depth) * m_list.size() * sizeof(T)
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

    // ── helpers ───────────────────────────────────────────────────────────────

    static int count_digits(T n) {
        if (n == T{}) return 1;
        int d = 0;
        while (n > T{}) { ++d; n /= T{10}; }
        return d;
    }

    // ── non-recording sort helpers ────────────────────────────────────────────

    static void msd_radix(std::vector<T>& list, int digit_pos) {
        if (list.size() <= 1 || digit_pos < 0) return;
        std::vector<std::vector<T>> buckets(10);
        // Use long long divisor to avoid overflow when T is int
        long long divisor = static_cast<long long>(std::pow(10.0, digit_pos));
        for (T num : list) {
            int d = std::clamp(
                static_cast<int>((static_cast<long long>(num) / divisor) % 10),
                0, 9);
            buckets[d].push_back(num);
        }
        for (auto& b : buckets) msd_radix(b, digit_pos - 1);
        list.clear();
        for (auto& b : buckets) list.insert(list.end(), b.begin(), b.end());
    }

    static void msd_string(std::vector<T>& list, std::size_t char_pos) {
        if (list.size() <= 1) return;
        std::vector<std::vector<T>> buckets(257);
        for (const T& s : list) {
            int idx = (char_pos < s.size())
                      ? static_cast<unsigned char>(s[char_pos]) + 1
                      : 0;
            buckets[idx].push_back(s);
        }
        for (auto& b : buckets) if (b.size() > 1) msd_string(b, char_pos + 1);
        list.clear();
        for (auto& b : buckets) list.insert(list.end(), b.begin(), b.end());
    }

    // ── frame-recording: integral path ───────────────────────────────────────
    // Wrapped in `if constexpr` to prevent instantiation of T{1}/count_digits
    // for non-integral T (e.g. std::string) even though this function is only
    // ever *called* from the is_integral branch of visualise().

    void visualise_integral() {
        if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;

        std::vector<T> neg, pos;
        for (T v : work) {
            if (v < T{}) neg.push_back(-v);
            else         pos.push_back(v);
        }
        T max_p = pos.empty() ? T{1} : *std::max_element(pos.begin(), pos.end());
        T max_n = neg.empty() ? T{1} : *std::max_element(neg.begin(), neg.end());

        msd_radix_frames(pos, count_digits(max_p) - 1, frames, "positives");
        msd_radix_frames(neg, count_digits(max_n) - 1, frames, "negatives (abs)");

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

        SortVisualiser::run(frames, "MSD Radix Sort");
        } // end if constexpr
    }

    static void msd_radix_frames(std::vector<T>& sub, int digit_pos,
                                  std::vector<SortFrame<T>>& frames,
                                  const std::string& part_label) {
        if (sub.size() <= 1 || digit_pos < 0) return;
        long long divisor = static_cast<long long>(std::pow(10.0, digit_pos));
        std::vector<std::vector<T>> buckets(10);

        for (std::size_t i = 0; i < sub.size(); ++i) {
            int d = std::clamp(
                static_cast<int>((static_cast<long long>(sub[i]) / divisor) % 10),
                0, 9);
            buckets[d].push_back(sub[i]);
            SortFrame<T> f;
            f.values = sub;
            f.highlights.push_back({ i, HighlightRole::Bucket });
            // FIX: value_to_str instead of std::to_string
            f.label = part_label + "  digit_pos=" + std::to_string(digit_pos)
                      + "  " + value_to_str(sub[i])
                      + " \u2192 bucket [" + std::to_string(d) + "]";
            frames.push_back(std::move(f));
        }

        for (auto& b : buckets)
            msd_radix_frames(b, digit_pos - 1, frames, part_label);

        sub.clear();
        for (auto& b : buckets) sub.insert(sub.end(), b.begin(), b.end());

        SortFrame<T> f;
        f.values = sub;
        f.label  = part_label + "  after digit_pos=" + std::to_string(digit_pos);
        frames.push_back(std::move(f));
    }

    // ── frame-recording: string path (NEW) ───────────────────────────────────
    // 257 buckets: bucket 0 = exhausted strings (shorter), 1–256 = ASCII chars.
    // Highlights cyan for the element being bucketed on each character position.

    void visualise_string() {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;

        msd_string_frames(work, 0, frames);
        m_list = work;

        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < work.size(); ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));

        SortVisualiser::run(frames, "MSD Radix Sort (strings)");
    }

    // Recursive string MSD with frame recording.
    // `offset` is the index into the full work array for highlight mapping.
    static void msd_string_frames(std::vector<T>& list,
                                   std::size_t char_pos,
                                   std::vector<SortFrame<T>>& frames) {
        if (list.size() <= 1) return;

        std::vector<std::vector<T>> buckets(257);
        for (std::size_t i = 0; i < list.size(); ++i) {
            int idx = (char_pos < list[i].size())
                      ? static_cast<unsigned char>(list[i][char_pos]) + 1
                      : 0;
            buckets[idx].push_back(list[i]);

            SortFrame<T> f;
            f.values = list;
            f.highlights.push_back({ i, HighlightRole::Bucket });
            char shown = (char_pos < list[i].size()) ? list[i][char_pos] : '\0';
            f.label = "char_pos=" + std::to_string(char_pos)
                      + "  \"" + value_to_str(list[i]) + "\""
                      + "  char='"
                      + (shown ? std::string(1, shown) : std::string("(end)"))
                      + "'  bucket [" + std::to_string(idx) + "]";
            frames.push_back(std::move(f));
        }

        for (auto& b : buckets)
            if (b.size() > 1)
                msd_string_frames(b, char_pos + 1, frames);

        list.clear();
        for (auto& b : buckets) list.insert(list.end(), b.begin(), b.end());

        SortFrame<T> f;
        f.values = list;
        f.label  = "Reassembled after char_pos=" + std::to_string(char_pos);
        frames.push_back(std::move(f));
    }
};
