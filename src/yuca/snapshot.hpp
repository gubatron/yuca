#ifndef YUCA_SNAPSHOT_HPP
#define YUCA_SNAPSHOT_HPP

#include <cstddef>
#include <stdexcept>

namespace yuca {
    /** Bounds for untrusted snapshots; decoded nodes can use more RAM than bytes on disk. */
    struct SnapshotLimits {
        std::size_t max_bytes = 1024ULL * 1024 * 1024;
        std::size_t max_documents = 1000000;
        std::size_t max_keys = 16000000;
        std::size_t max_properties = 16000000;
        std::size_t max_string_bytes = 16 * 1024 * 1024;
        // Retained text, including group-name duplication in each key and reverse index.
        std::size_t max_decoded_string_bytes = 1024ULL * 1024 * 1024;
    };

    /** Invalid/incompatible snapshot or file I/O failure. bad_alloc is not translated. */
    class SnapshotError : public std::runtime_error {
    public:
        explicit SnapshotError(const char *message) : std::runtime_error(message) {}
    };
}
#endif
