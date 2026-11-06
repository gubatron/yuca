// Private benchmark seam; not installed or part of the public API.
#ifndef YUCA_SNAPSHOT_DETAIL_H
#define YUCA_SNAPSHOT_DETAIL_H
#include "indexer.hpp"
namespace yuca { namespace detail {
    Indexer deserializeByReplay(const std::vector<unsigned char> &bytes,
                                const SnapshotLimits &limits = SnapshotLimits());
}}
#endif
