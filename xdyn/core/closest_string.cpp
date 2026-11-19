/*
 * closest_string.cpp
 */

#include "closest_string.hpp"

#include <algorithm>
#include <climits>

namespace {
size_t levenshtein_distance(const std::string& s, const std::string& t)
{
    const size_t n = s.size() + 1;
    const size_t m = t.size() + 1;
    std::vector<size_t> d(n * m, 0);
    for (size_t i = 1, im = 0; i < m; ++i, ++im) {
        for (size_t j = 1, jn = 0; j < n; ++j, ++jn) {
            if (s[jn] == t[im]) {
                d[(i * n) + j] = d[((i - 1) * n) + (j - 1)];
            } else {
                d[(i * n) + j] = std::min(
                    d[(i - 1) * n + j] + 1,  // Deletion
                    std::min(
                        d[i * n + (j - 1)] + 1,          // Insertion
                        d[(i - 1) * n + (j - 1)] + 1));  // Substitution
            }
        }
    }
    return d[n * m - 1];
}
}  // namespace

std::string closest_string(
    const std::string& string_to_match,
    const std::vector<std::string>& strings_to_test)
{
    if (strings_to_test.empty())
        return "";
    size_t min_idx = 0;
    size_t min_dist = INT_MAX;
    size_t k = 0;
    for (const auto& s : strings_to_test) {
        const size_t levenshtein = levenshtein_distance(string_to_match, s);
        if (levenshtein < min_dist) {
            min_idx = k;
            min_dist = levenshtein;
        }
        k++;
    }
    return strings_to_test[min_idx];
}
