#include "support/allocation_tracker.hpp"
#include <cstdint>
#include <new>

int main() {
    const auto before = yuca_test::allocationSnapshot();
    yuca_test::resetAllocationPeak();
    void *first = ::operator new(37);
    void *second = ::operator new[](19);
    const auto during = yuca_test::allocationSnapshot();
    const bool valid = during.allocations - before.allocations == 2 &&
        during.allocated_bytes - before.allocated_bytes == 56 &&
        during.live_bytes - before.live_bytes == 56 && during.peak_bytes >= during.live_bytes &&
        reinterpret_cast<std::uintptr_t>(first) % alignof(std::max_align_t) == 0;
    ::operator delete(first);
    ::operator delete[](second);
    if (!valid || yuca_test::allocationSnapshot().live_bytes != before.live_bytes) {
        return 1;
    }
    void *empty = ::operator new(0);
    ::operator delete(empty);
    return empty == nullptr ? 1 : 0;
}
