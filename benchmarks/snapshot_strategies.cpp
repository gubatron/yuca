// Encoding experiment, not a .yuca file writer. Both encodings reconstruct the
// same strings; corpus construction and validation are outside the timers.
#include "corpus.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>

namespace {
    using Bytes = std::vector<unsigned char>;
    using Records = std::vector<std::vector<std::string>>;
    void integer(Bytes &out, std::uint64_t value) {
        for (unsigned i = 0; i < 8; ++i) { out.push_back(static_cast<unsigned char>(value)); value >>= 8; }
    }
    void text(Bytes &out, const std::string &value) {
        integer(out, value.size()); out.insert(out.end(), value.begin(), value.end());
    }
    struct Reader {
        explicit Reader(const Bytes &input) : bytes(input) {}
        const Bytes &bytes; std::size_t offset = 0;
        std::uint64_t integer() {
            if (bytes.size() - offset < 8) throw std::runtime_error("truncated experiment");
            std::uint64_t value = 0;
            for (unsigned i = 0; i < 8; ++i) value |= static_cast<std::uint64_t>(bytes[offset++]) << (i * 8);
            return value;
        }
        std::string text() {
            const auto count = integer();
            if (count > bytes.size() - offset) throw std::runtime_error("bad length");
            std::string value(reinterpret_cast<const char *>(bytes.data() + offset), static_cast<std::size_t>(count));
            offset += static_cast<std::size_t>(count); return value;
        }
    };
    Bytes encode(const Records &records, bool dictionary) {
        Bytes out;
        std::map<std::string, std::uint64_t> ids;
        if (dictionary) {
            for (const auto &record : records) for (const auto &value : record) ids.emplace(value, ids.size());
            integer(out, ids.size());
            std::vector<const std::string *> values(ids.size());
            for (const auto &entry : ids) values[static_cast<std::size_t>(entry.second)] = &entry.first;
            for (const auto *value : values) text(out, *value);
        }
        integer(out, records.size());
        for (const auto &record : records) {
            integer(out, record.size());
            for (const auto &value : record) {
                if (dictionary) integer(out, ids.at(value)); else text(out, value);
            }
        }
        return out;
    }
    Records decode(const Bytes &bytes, bool dictionary) {
        Reader reader{bytes};
        std::vector<std::string> values;
        if (dictionary) {
            const auto count = reader.integer();
            for (std::uint64_t i = 0; i < count; ++i) values.push_back(reader.text());
        }
        Records records(static_cast<std::size_t>(reader.integer()));
        for (auto &record : records) {
            const auto count = reader.integer();
            for (std::uint64_t i = 0; i < count; ++i) {
                record.push_back(dictionary ? values.at(static_cast<std::size_t>(reader.integer())) : reader.text());
            }
        }
        if (reader.offset != bytes.size()) throw std::runtime_error("trailing bytes");
        return records;
    }
}

int main(int argc, char **argv) {
    try {
        const std::string argument = argc > 1 ? argv[1] : "10000";
        std::size_t used = 0;
        const auto parsed = std::stoul(argument, &used);
        if (used != argument.size() || parsed == 0 || parsed > 1000000) throw std::invalid_argument("documents must be 1..1000000");
        const auto count = static_cast<unsigned>(parsed);
        Records records;
        for (const auto &doc : yuca_bench::makeCorpus(count)) {
            std::vector<std::string> record;
            for (const auto &group : doc.getGroups()) {
                const auto keys = doc.getGroupSPKeys(group);
                for (const auto &key : keys.getStdSet()) {
                    record.push_back(group); record.push_back(key->getString());
                }
            }
            record.push_back("title"); record.push_back(doc.stringProperty("title"));
            records.push_back(std::move(record));
        }
        std::cout << "strategy,documents,sample,bytes,encode_ms,decode_ms\n";
        for (unsigned sample = 0; sample < 8; ++sample) for (unsigned pass = 0; pass < 2; ++pass) {
            const bool dictionary = (sample + pass) % 2 != 0;
            const auto start = std::chrono::steady_clock::now();
            const auto bytes = encode(records, dictionary);
            const auto encoded = std::chrono::steady_clock::now();
            const auto restored = decode(bytes, dictionary);
            const auto decoded = std::chrono::steady_clock::now();
            if (records != restored) throw std::runtime_error("experiment round trip failed");
            if (sample != 0) std::cout << (dictionary ? "dictionary" : "inline") << ',' << count << ',' << sample << ',' << bytes.size() << ','
                << std::chrono::duration<double, std::milli>(encoded - start).count() << ','
                << std::chrono::duration<double, std::milli>(decoded - encoded).count() << '\n';
        }
    } catch (const std::exception &error) { std::cerr << error.what() << '\n'; return 1; }
}
