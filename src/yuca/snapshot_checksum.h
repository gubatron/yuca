// Portable XXH64, seed 0, for accidental-corruption detection, NOT authentication.
// Algorithm: https://github.com/Cyan4973/xxHash/blob/v0.8.3/doc/xxhash_spec.md
// Small C++11 adaptation of the BSD-2-Clause xxHash algorithm:
// Copyright (c) 2012-2023 Yann Collet. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
#ifndef YUCA_SNAPSHOT_CHECKSUM_H
#define YUCA_SNAPSHOT_CHECKSUM_H
#include <cstddef>
#include <cstdint>

namespace yuca { namespace detail {
    inline std::uint64_t rotate64(std::uint64_t value, unsigned bits) { return (value << bits) | (value >> (64 - bits)); }
    inline std::uint64_t readLittle(const unsigned char *bytes, unsigned count) {
        std::uint64_t result = 0;
        for (unsigned i = 0; i < count; ++i) result |= static_cast<std::uint64_t>(bytes[i]) << (i * 8);
        return result;
    }
    inline std::uint64_t xxRound(std::uint64_t accumulator, std::uint64_t value) {
        return rotate64(accumulator + value * 14029467366897019727ULL, 31) * 11400714785074694791ULL;
    }
    inline std::uint64_t xxMerge(std::uint64_t accumulator, std::uint64_t value) {
        return (accumulator ^ xxRound(0, value)) * 11400714785074694791ULL + 9650029242287828579ULL;
    }
    inline std::uint64_t snapshotChecksum(const unsigned char *bytes, std::size_t size) {
        const std::uint64_t p1 = 11400714785074694791ULL, p2 = 14029467366897019727ULL;
        const std::uint64_t p3 = 1609587929392839161ULL, p4 = 9650029242287828579ULL, p5 = 2870177450012600261ULL;
        std::size_t position = 0;
        std::uint64_t result;
        if (size >= 32) {
            std::uint64_t v1 = p1 + p2, v2 = p2, v3 = 0, v4 = 0ULL - p1;
            do {
                v1 = xxRound(v1, readLittle(bytes + position, 8));
                v2 = xxRound(v2, readLittle(bytes + position + 8, 8));
                v3 = xxRound(v3, readLittle(bytes + position + 16, 8));
                v4 = xxRound(v4, readLittle(bytes + position + 24, 8));
                position += 32;
            } while (size - position >= 32);
            result = rotate64(v1, 1) + rotate64(v2, 7) + rotate64(v3, 12) + rotate64(v4, 18);
            result = xxMerge(xxMerge(xxMerge(xxMerge(result, v1), v2), v3), v4);
        } else result = p5;
        result += size;
        while (size - position >= 8) {
            result = rotate64(result ^ xxRound(0, readLittle(bytes + position, 8)), 27) * p1 + p4;
            position += 8;
        }
        if (size - position >= 4) {
            result = rotate64(result ^ (readLittle(bytes + position, 4) * p1), 23) * p2 + p3;
            position += 4;
        }
        while (position != size) result = rotate64(result ^ (bytes[position++] * p5), 11) * p1;
        result ^= result >> 33; result *= p2; result ^= result >> 29; result *= p3; result ^= result >> 32;
        return result;
    }
}}
#endif
