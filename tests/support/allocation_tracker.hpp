#ifndef YUCA_TEST_ALLOCATION_TRACKER_HPP
#define YUCA_TEST_ALLOCATION_TRACKER_HPP

#include <cstddef>

// Only link the implementation into dedicated diagnostic executables. It replaces
// their global new/delete; it must never be linked into the distributed library.
namespace yuca_test {
    struct AllocationSnapshot {
        std::size_t allocations;
        std::size_t allocated_bytes;
        std::size_t live_bytes;
        std::size_t peak_bytes;
    };

    AllocationSnapshot allocationSnapshot() noexcept;
    void resetAllocationPeak() noexcept;
    // One-shot: fail the allocation after this many successful allocation calls.
    // Automatically disarms before throwing, so failure reporting can allocate.
    void failAllocationAfter(std::size_t successful_calls) noexcept;
    void disableAllocationFailure() noexcept;
}

#endif
