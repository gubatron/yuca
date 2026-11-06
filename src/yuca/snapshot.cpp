#include "snapshot_detail.h"
#include "snapshot_checksum.h"
#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <limits>
#include <vector>
#include <utility>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#endif

namespace yuca {
namespace {
    static_assert(CHAR_BIT == 8 && sizeof(long) <= 8 && sizeof(int) <= 8 && sizeof(std::size_t) <= 8,
                  "snapshot requires 8-bit bytes and at most 64-bit integers");
    using Bytes = std::vector<unsigned char>;
    const unsigned char magic[] = {'Y', 'U', 'C', 'A', 'I', 'D', 'X', 0};
    const std::size_t header_size = 40;

    void require(bool condition, const char *message) {
        if (!condition) throw SnapshotError(message);
    }

    std::uint64_t checksum(const unsigned char *data, std::size_t size) {
        return detail::snapshotChecksum(data, size);
    }

    std::uint64_t hashProbe() {
        return static_cast<std::uint64_t>(std::hash<std::string>{}(":yuca snapshot v1\x01"));
    }

    void setInteger(Bytes &out, std::size_t position, std::uint64_t value) {
        for (unsigned i = 0; i < 8; ++i) {
            out[position + i] = static_cast<unsigned char>(value);
            value >>= 8;
        }
    }

    struct Writer {
        Bytes bytes;
        Writer() : bytes(header_size, 0) { bytes.reserve(4096); }
        void integer(std::uint64_t value) {
            const auto position = bytes.size();
            require(position <= bytes.max_size() - 8, "snapshot exceeds address space");
            bytes.resize(position + 8);
            setInteger(bytes, position, value);
        }
        void text(const std::string &value) {
            integer(value.size());
            require(value.size() <= bytes.max_size() - bytes.size(), "snapshot exceeds address space");
            bytes.insert(bytes.end(), value.begin(), value.end());
        }
    };

    struct Reader {
        const unsigned char *data;
        std::size_t size;
        std::size_t position = 0;
        std::size_t decoded_string_bytes = 0;
        const SnapshotLimits &limits;
        Reader(const unsigned char *input, std::size_t length, const SnapshotLimits &bounds) : data(input), size(length), limits(bounds) {}
        std::size_t remaining() const { return size - position; }
        unsigned char byte() {
            require(remaining() != 0, "truncated snapshot");
            return data[position++];
        }
        std::uint64_t integer() {
            require(remaining() >= 8, "truncated snapshot integer");
            std::uint64_t value = 0;
            for (unsigned i = 0; i < 8; ++i) value |= static_cast<std::uint64_t>(data[position++]) << (i * 8);
            return value;
        }
        std::int64_t signedInteger() {
            const auto value = integer();
            const auto maximum = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());
            return value <= maximum ? static_cast<std::int64_t>(value) :
                -1 - static_cast<std::int64_t>(std::numeric_limits<std::uint64_t>::max() - value);
        }
        template<class T> T nativeInteger() {
            const auto value = signedInteger();
            require(value >= std::numeric_limits<T>::min() && value <= std::numeric_limits<T>::max(), "snapshot integer does not fit this platform");
            return static_cast<T>(value);
        }
        std::size_t count(std::size_t maximum, std::size_t minimum_bytes) {
            const auto value = integer();
            require(value <= maximum && value <= remaining() / minimum_bytes, "snapshot count exceeds bounds");
            return static_cast<std::size_t>(value);
        }
        std::string text() {
            const auto length = count(limits.max_string_bytes, 1);
            chargeStrings(length);
            std::string result(reinterpret_cast<const char *>(data + position), length);
            position += length;
            return result;
        }
        void chargeStrings(std::size_t bytes) {
            require(bytes <= limits.max_decoded_string_bytes - decoded_string_bytes, "snapshot decoded string budget exceeded");
            decoded_string_bytes += bytes;
        }
    };

    // Both callers have checked/read at least header_size bytes. File loading
    // validates this fixed header before allocating the potentially large buffer.
    std::uint64_t readHeader(Reader &reader, std::size_t full_size) {
        require(std::memcmp(reader.data, magic, 8) == 0, "invalid snapshot magic");
        reader.position = 8;
        require(reader.integer() == 1, "unsupported snapshot version or flags");
        require(reader.integer() == full_size - header_size, "snapshot payload length mismatch");
        const auto expected_checksum = reader.integer();
        require(reader.integer() == hashProbe(), "incompatible snapshot string hash ABI");
        return expected_checksum;
    }

    void validatePath(const std::string &path) {
        require(!path.empty() && path.find('\0') == std::string::npos, "invalid snapshot path");
    }

    class TemporaryFile {
    public:
        std::string path;
        int descriptor = -1;
        explicit TemporaryFile(const std::string &destination) {
            std::string pattern = destination + ".tmp.XXXXXX";
            std::vector<char> name(pattern.begin(), pattern.end());
            name.push_back(0);
#ifdef _WIN32
            require(_mktemp_s(name.data(), name.size()) == 0, "cannot choose snapshot temporary file");
            const auto error = _sopen_s(&descriptor, name.data(), _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY,
                                        _SH_DENYRW, _S_IREAD | _S_IWRITE);
            require(error == 0, "cannot create snapshot temporary file");
#else
            descriptor = mkstemp(name.data());
#endif
            require(descriptor >= 0, "cannot create snapshot temporary file");
            // Even a path-allocation failure must close/unlink the new file.
            try { path.assign(name.data()); }
            catch (...) { closeFile(); std::remove(name.data()); throw; }
        }
        TemporaryFile(const TemporaryFile &) = delete;
        TemporaryFile &operator=(const TemporaryFile &) = delete;
        ~TemporaryFile() { closeFile(); if (!path.empty()) std::remove(path.c_str()); }
        int closeFile() noexcept {
            if (descriptor < 0) return 0;
            const int current = descriptor;
            descriptor = -1;
#ifdef _WIN32
            return _close(current);
#else
            return ::close(current);
#endif
        }
        void publish(const Bytes &bytes, const std::string &destination) {
            std::size_t position = 0;
            while (position != bytes.size()) {
                const auto count = std::min<std::size_t>(bytes.size() - position, 1024 * 1024);
#ifdef _WIN32
                const auto written = _write(descriptor, bytes.data() + position, static_cast<unsigned>(count));
#else
                const auto written = ::write(descriptor, bytes.data() + position, count);
#endif
                if (written < 0 && errno == EINTR) continue;
                require(written > 0, "cannot write snapshot");
                position += static_cast<std::size_t>(written);
            }
#ifdef _WIN32
            require(_commit(descriptor) == 0, "cannot sync snapshot");
#else
            require(fsync(descriptor) == 0, "cannot sync snapshot");
#endif
            require(closeFile() == 0, "cannot close snapshot");
#ifdef _WIN32
            require(MoveFileExA(path.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0, "cannot replace snapshot");
#else
            require(std::rename(path.c_str(), destination.c_str()) == 0, "cannot replace snapshot");
#endif
            path.clear();
        }
    };
}

// A narrow private adapter borrows collection storage to avoid getter copies.
// Decode builds an unpublished index, so it needn't pay per-document rollback
// snapshot costs. Any failure destroys the whole local result.
class SnapshotCodec {
    template<class T> static std::uint64_t propertyInteger(T value) { return static_cast<std::uint64_t>(value); }
    static std::uint64_t propertyInteger(char value) { return static_cast<unsigned char>(value); }
    template<class T> static void writeProperties(Writer &writer, const utils::Map<std::string, T> &properties) {
        writer.integer(properties.size());
        for (const auto &entry : properties.getStdMap()) {
            writer.text(entry.first);
            writer.integer(propertyInteger(entry.second));
        }
    }
    static void writeProperties(Writer &writer, const utils::Map<std::string, std::string> &properties) {
        writer.integer(properties.size());
        for (const auto &entry : properties.getStdMap()) { writer.text(entry.first); writer.text(entry.second); }
    }
    template<class T> static T propertyValue(Reader &reader, T *) { return reader.nativeInteger<T>(); }
    static bool propertyValue(Reader &reader, bool *) {
        const auto value = reader.integer(); require(value <= 1, "invalid snapshot bool"); return value != 0;
    }
    static char propertyValue(Reader &reader, char *) {
        const auto value = reader.integer();
        require(value <= 255, "invalid snapshot byte");
        const auto bits = static_cast<unsigned char>(value);
        char result; std::memcpy(&result, &bits, 1); return result;
    }
    static std::string propertyValue(Reader &reader, std::string *) { return reader.text(); }
    template<class T> static void readProperties(Reader &reader, utils::Map<std::string, T> &properties, std::size_t &total) {
        const auto count = reader.count(reader.limits.max_properties - total, 16);
        total += count;
        for (std::size_t i = 0; i < count; ++i) {
            auto name = reader.text();
            auto value = propertyValue(reader, static_cast<T *>(nullptr));
            require(properties.getStdMap().emplace(std::move(name), std::move(value)).second, "duplicate snapshot property");
        }
    }
public:
    static Bytes encode(const Indexer &index) {
        Writer writer;
        writer.text(index.implicit_group);
        writer.integer(index.docPtrCache.size());
        std::vector<const StringKey *> keys; // Reuse sorting storage across all groups/documents.
        for (const auto &entry : index.docPtrCache.getStdMap()) {
            const auto &document = *entry.second;
            require(entry.first == document.id, "inconsistent document cache");
            writer.integer(static_cast<std::uint64_t>(document.id));
            writer.integer(document.group_2_keyset_map.size());
            for (const auto &group : document.group_2_keyset_map.getStdMap()) {
                writer.text(group.first);
                writer.integer(group.second.size());
                keys.clear();
                keys.reserve(group.second.size());
                for (const auto &key : group.second.getStdSet()) {
                    require(key && key->group == group.first, "inconsistent document key");
                    require(key->id == static_cast<long>(std::hash<std::string>{}(group.first + key->str_key)),
                            "inconsistent snapshot key hash");
                    keys.push_back(key.get());
                }
                std::sort(keys.begin(), keys.end(), [](const StringKey *a, const StringKey *b) { return a->str_key < b->str_key; });
                for (const auto *key : keys) { writer.text(key->str_key); writer.integer(static_cast<std::uint64_t>(key->id)); }
            }
            writeProperties(writer, document.bool_properties);
            writeProperties(writer, document.byte_properties);
            writeProperties(writer, document.int_properties);
            writeProperties(writer, document.long_properties);
            writeProperties(writer, document.string_properties);
        }
        std::copy(magic, magic + 8, writer.bytes.begin());
        writer.bytes[8] = 1; // u32 version=1, u32 reserved flags=0
        setInteger(writer.bytes, 16, writer.bytes.size() - header_size);
        setInteger(writer.bytes, 24, checksum(writer.bytes.data() + header_size, writer.bytes.size() - header_size));
        setInteger(writer.bytes, 32, hashProbe());
        return std::move(writer.bytes);
    }

    static Indexer decode(const void *data, std::size_t size, const SnapshotLimits &limits, bool replay) {
        require(data != nullptr && size >= header_size && size <= limits.max_bytes, "snapshot byte length exceeds bounds");
        const auto *bytes = static_cast<const unsigned char *>(data);
        Reader reader(bytes, size, limits);
        const auto expected_checksum = readHeader(reader, size);
        require(expected_checksum == checksum(bytes + header_size, size - header_size), "snapshot checksum mismatch");
        Indexer result(reader.text());
        const auto count = reader.count(limits.max_documents, 56);
        std::size_t keys_total = 0, properties_total = 0;
        long previous_id = 0;
        for (std::size_t i = 0; i < count; ++i) {
            const auto id = reader.nativeInteger<long>();
            require(i == 0 || id > previous_id, "duplicate or unordered snapshot document");
            previous_id = id;
            auto document = std::make_shared<Document>(id);
            const auto groups = reader.count(limits.max_keys - keys_total, 16);
            for (std::size_t g = 0; g < groups; ++g) {
                auto group = reader.text();
                if (!result.reverseIndices.containsKey(group)) reader.chargeStrings(group.size());
                const auto keys = reader.count(limits.max_keys - keys_total, 16);
                require(keys != 0, "empty snapshot group");
                keys_total += keys;
                SPStringKeySet key_set;
                for (std::size_t k = 0; k < keys; ++k) {
                    auto value = reader.text();
                    const auto key_id = reader.nativeInteger<long>();
                    reader.chargeStrings(group.size()); // Every StringKey owns a group copy.
                    auto key = std::make_shared<StringKey>(value, group);
                    require(key->id == key_id, "incompatible snapshot key hash");
                    key_set.add(key);
                }
                require(document->group_2_keyset_map.getStdMap().emplace(std::move(group), std::move(key_set)).second, "duplicate snapshot group");
            }
            readProperties(reader, document->bool_properties, properties_total);
            readProperties(reader, document->byte_properties, properties_total);
            readProperties(reader, document->int_properties, properties_total);
            readProperties(reader, document->long_properties, properties_total);
            readProperties(reader, document->string_properties, properties_total);
            if (replay) result.indexDocument(document);
            else {
                result.docPtrCache.getStdMap().emplace(id, document);
                for (const auto &group : document->group_2_keyset_map.getStdMap()) {
                    auto &indices = result.reverseIndices.getStdMap();
                    auto found = indices.find(group.first);
                    if (found == indices.end()) found = indices.emplace(group.first, std::make_shared<ReverseIndex>()).first;
                    for (const auto &key : group.second.getStdSet()) found->second->putDocument(key, document);
                }
            }
        }
        require(reader.remaining() == 0, "trailing snapshot data");
        return result;
    }
};

std::vector<unsigned char> Indexer::serialize() const { return SnapshotCodec::encode(*this); }

Indexer Indexer::deserialize(const void *bytes, std::size_t size, const SnapshotLimits &limits) {
    return SnapshotCodec::decode(bytes, size, limits, false);
}

namespace detail {
    Indexer deserializeByReplay(const std::vector<unsigned char> &bytes, const SnapshotLimits &limits) {
        return SnapshotCodec::decode(bytes.data(), bytes.size(), limits, true);
    }
}

void Indexer::save(const std::string &path) const {
    validatePath(path);
    const auto bytes = serialize();
    TemporaryFile temporary(path);
    temporary.publish(bytes, path);
}

Indexer Indexer::load(const std::string &path, const SnapshotLimits &limits) {
    validatePath(path);
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    require(static_cast<bool>(file), "cannot open snapshot");
    const auto length = file.tellg();
    require(length >= 0 && static_cast<std::uintmax_t>(length) <= limits.max_bytes &&
            static_cast<std::uintmax_t>(length) <= static_cast<std::uintmax_t>(std::numeric_limits<std::streamsize>::max()), "snapshot file exceeds byte limit");
    require(length >= static_cast<std::streamoff>(header_size), "truncated snapshot file");
    file.seekg(0);
    unsigned char header[header_size] = {};
    file.read(reinterpret_cast<char *>(header), static_cast<std::streamsize>(header_size));
    require(static_cast<bool>(file), "snapshot header read failed");
    Reader preflight(header, header_size, limits);
    readHeader(preflight, static_cast<std::size_t>(length));
    Bytes bytes(static_cast<std::size_t>(length));
    std::copy(header, header + header_size, bytes.begin());
    file.read(reinterpret_cast<char *>(bytes.data() + header_size), static_cast<std::streamsize>(bytes.size() - header_size));
    require(static_cast<bool>(file) && file.peek() == std::char_traits<char>::eof(), "snapshot read failed or file changed");
    return deserialize(bytes, limits);
}
}
