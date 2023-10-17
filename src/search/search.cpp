#include "search.h"
#include <algorithm>
#include <map>
#include <numeric>
#include <cmath>
#include <string>

template <class UnaryPredicate>
std::vector<std::string_view> SplitTextWithPredicate(const std::string_view text, const UnaryPredicate& predicate) {
    std::vector<std::string_view> parts;

    size_t left_border = 0;
    while (left_border < text.size()) {
        if (!predicate(text[left_border])) {
            ++left_border;
            continue;
        }

        size_t right_border = left_border + 1;
        while (right_border < text.size() && predicate(text[right_border])) {
            ++right_border;
        }

        size_t len = right_border - left_border;
        parts.push_back(text.substr(left_border, len));

        left_border = right_border;
    }

    return parts;
}

struct CompareCaseInsensitive : public std::binary_function<std::string_view, std::string_view, bool> {
    bool operator()(const std::string_view lhs, const std::string_view rhs) const {
        for (size_t i = 0; i < std::min(lhs.size(), rhs.size()); i++) {
            if (std::tolower(lhs[i]) != std::tolower(rhs[i])) {
                return std::tolower(lhs[i]) < std::tolower(rhs[i]);
            }
        }
        return lhs.size() < rhs.size();
    }
};

class Row {
public:
    explicit Row(std::string_view row) : row_(row) {
        words_ = SplitTextWithPredicate(row, [](const char sym) { return std::isalpha(sym); });
        for (const std::string_view word : words_) {
            ++words_counter_[word];
        }
    }

    long double CalculateTf(std::string_view word) const {
        if (!words_counter_.contains(word)) {
            return 0;
        }
        return static_cast<long double>(words_counter_.at(word)) / static_cast<long double>(words_.size());
    }

    std::map<std::string_view, bool, CompareCaseInsensitive> GetUniqueWords() const {
        std::map<std::string_view, bool, CompareCaseInsensitive> unique_words;
        for (const std::string_view word : words_) {
            unique_words[word] = true;
        }

        return unique_words;
    }

    std::string_view GetText() const {
        return row_;
    }

    bool IsEmpty() const {
        return words_.empty();
    }

private:
    std::string_view row_;
    std::vector<std::string_view> words_;
    std::map<std::string_view, size_t, CompareCaseInsensitive> words_counter_;
};

class Text {
public:
    explicit Text(std::string_view text) {
        std::vector<std::string_view> partition =
            SplitTextWithPredicate(text, [](const char sym) { return sym != '\n'; });
        for (const std::string_view row_str : partition) {
            Row row = Row(row_str);
            if (!row.IsEmpty()) {
                UpdateCounter(row.GetUniqueWords());
                rows_.emplace_back(row);
            }
        }
    }

    void UpdateCounter(const std::map<std::string_view, bool, CompareCaseInsensitive> new_words) {
        for (const auto [word, _] : new_words) {
            ++words_counter_[word];
        }
    }

    std::vector<std::string_view> GetMostRelevant(const std::string_view query, const size_t results_count) const {
        if (rows_.empty()) {
            return {};
        }

        std::vector<std::string_view> query_words =
            SplitTextWithPredicate(query, [](const char sym) { return std::isalpha(sym); });

        std::sort(query_words.begin(), query_words.end());
        query_words.erase(unique(query_words.begin(), query_words.end()), query_words.end());

        std::vector<long double> row_metric(rows_.size());
        for (const std::string_view query_word : query_words) {
            if (!IsInText(query_word)) {
                continue;
            }
            for (size_t i = 0; i < rows_.size(); i++) {
                row_metric[i] += rows_[i].CalculateTf(query_word) * CalculateIdf(query_word);
            }
        }

        std::vector<size_t> rows_order(rows_.size());
        std::iota(rows_order.begin(), rows_order.end(), 0);
        std::stable_sort(rows_order.begin(), rows_order.end(), [&row_metric](const size_t lhs, const size_t rhs) {
            return row_metric[lhs] > row_metric[rhs];
        });

        std::vector<std::string_view> answer;
        for (size_t i = 0; i < std::min(results_count, rows_.size()); i++) {
            if (row_metric[rows_order[i]] == 0) {
                break;
            }
            answer.push_back(rows_[rows_order[i]].GetText());
        }

        return answer;
    }

    bool IsInText(const std::string_view word) const {
        return words_counter_.contains(word);
    }

private:
    std::vector<Row> rows_;
    std::map<std::string_view, size_t, CompareCaseInsensitive> words_counter_;

    long double CalculateIdf(const std::string_view word) const {
        return std::log(static_cast<long double>(rows_.size()) / static_cast<long double>(words_counter_.at(word)));
    }
};

std::vector<std::string_view> Search(std::string_view text, std::string_view query, size_t results_count) {
    return Text(text).GetMostRelevant(query, results_count);
}
