// algorithms/insertion_sort.cppm
// FIX: visualise() had `requires std::is_arithmetic_v<T>` — removed.
// FIX: std::to_string(key) replaced with value_to_str(key).

export module algorithms.insertion_sort;

import visualiser.sfml_visualiser;

import <vector>;
import <algorithm>;
import <string>;
import <concepts>;
import <iostream>;

export template <typename T>
    requires std::totally_ordered<T>
class insertion_sort {
public:
    explicit insertion_sort(std::vector<T> list) : m_list(std::move(list)) {}

    void sort() {
        for (std::size_t i = 1; i < m_list.size(); ++i) {
            T key = m_list[i];
            int j = static_cast<int>(i) - 1;
            while (j >= 0 && m_list[static_cast<std::size_t>(j)] > key) {
                m_list[static_cast<std::size_t>(j + 1)] = m_list[static_cast<std::size_t>(j)];
                --j;
            }
            m_list[static_cast<std::size_t>(j + 1)] = key;
        }
    }

    // FIX: no arithmetic constraint
    void visualise() {
        std::vector<SortFrame<T>> frames;
        std::vector<T> work = m_list;
        const std::size_t n = work.size();

        for (std::size_t i = 1; i < n; ++i) {
            T key = work[i];
            int j = static_cast<int>(i) - 1;

            // Pick-up frame
            {
                SortFrame<T> f;
                f.values = work;
                f.highlights.push_back({ i, HighlightRole::Compare });
                for (std::size_t k = 0; k < i; ++k)
                    f.highlights.push_back({ k, HighlightRole::Sorted });
                f.label = "Picking up [" + std::to_string(i)
                          + "] = " + value_to_str(key);
                frames.push_back(std::move(f));
            }

            while (j >= 0 && work[static_cast<std::size_t>(j)] > key) {
                work[static_cast<std::size_t>(j + 1)] = work[static_cast<std::size_t>(j)];
                SortFrame<T> f;
                f.values = work;
                f.highlights.push_back({ static_cast<std::size_t>(j),     HighlightRole::Swap });
                f.highlights.push_back({ static_cast<std::size_t>(j + 1), HighlightRole::Swap });
                for (std::size_t k = 0; k < i; ++k)
                    if (static_cast<int>(k) != j && static_cast<int>(k) != j + 1)
                        f.highlights.push_back({ k, HighlightRole::Sorted });
                f.label = "Shifting [" + std::to_string(j) + "] right";
                frames.push_back(std::move(f));
                --j;
            }
            std::size_t pos = static_cast<std::size_t>(j + 1);
            work[pos] = key;

            SortFrame<T> f;
            f.values = work;
            for (std::size_t k = 0; k <= i; ++k)
                f.highlights.push_back({ k, HighlightRole::Sorted });
            f.label = "Inserted " + value_to_str(key)
                      + " at position " + std::to_string(pos);
            frames.push_back(std::move(f));
        }

        SortFrame<T> ff;
        ff.values = work;
        for (std::size_t k = 0; k < n; ++k)
            ff.highlights.push_back({ k, HighlightRole::Sorted });
        ff.label = "Sorted!";
        frames.push_back(std::move(ff));

        m_list = work;
        SortVisualiser::run(frames, "Insertion Sort");
    }

    void report_memory() const {
        std::cout << "Insertion sort memory: " << m_list.capacity() * sizeof(T) << " B\n";
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
