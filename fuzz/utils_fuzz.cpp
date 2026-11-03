#include "yuca/utils.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace {
    // Deliberately simple O(n*m) oracle, independent of the one-column implementation.
    std::size_t distance(const std::string &left, const std::string &right) {
        std::vector<std::vector<std::size_t>> grid(left.size() + 1,
                                                  std::vector<std::size_t>(right.size() + 1));
        for (std::size_t i = 0; i <= left.size(); ++i) {
            grid[i][0] = i;
        }
        for (std::size_t j = 0; j <= right.size(); ++j) {
            grid[0][j] = j;
        }
        for (std::size_t i = 1; i <= left.size(); ++i) {
            for (std::size_t j = 1; j <= right.size(); ++j) {
                grid[i][j] = std::min({grid[i - 1][j] + 1, grid[i][j - 1] + 1,
                    grid[i - 1][j - 1] + (left[i - 1] == right[j - 1] ? 0 : 1)});
            }
        }
        return grid.back().back();
    }

    void check(bool valid) {
        if (!valid) {
            std::abort();
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t *data, std::size_t size) {
    if (size == 0 || size > 512) {
        return 0;
    }
    const std::size_t length = std::min<std::size_t>(size - 1, 128);
    std::size_t bound = 0;
    for (std::size_t i = 0; i < std::min(size, sizeof(bound)); ++i) {
        bound = (bound << 8) | data[i];
    }
    check(yuca::utils::maxRand(bound) <= bound);
    const std::string input(reinterpret_cast<const char *>(data + 1), length);
    const std::size_t cut = data[0] % (length + 1);
    const std::string left = input.substr(0, cut);
    const std::string right = input.substr(cut);
    const auto expected = distance(left, right);
    check(yuca::utils::levenshteinDistance(left, right) == expected);
    check(yuca::utils::levenshteinDistance(right, left) == expected);

    const auto words = yuca::utils::split(input);
    for (const auto &word : words.getStdVector()) {
        check(!word.empty());
    }
    yuca::utils::List<int> list;
    std::vector<int> reference;
    for (std::size_t i = 1; i < std::min<std::size_t>(size, 64); ++i) {
        list.add(data[i]);
        reference.push_back(data[i]);
    }
    const unsigned long start = data[0] % 3 == 0
        ? std::numeric_limits<unsigned long>::max() : data[0];
    const unsigned long requested = data[0] % 2 == 0
        ? std::numeric_limits<unsigned long>::max() : data[size - 1];
    const auto slice = list.subList(start, requested);
    const unsigned long count = start >= reference.size()
        ? 0 : std::min<unsigned long>(requested, reference.size() - start);
    check(slice.size() == count);
    for (unsigned long i = 0; i < count; ++i) {
        check(slice.get(i) == reference[start + i]);
    }
    list.addAll(list);
    check(list.size() == reference.size() * 2);
    list.removeAll(list);
    check(list.isEmpty());
    return 0;
}
