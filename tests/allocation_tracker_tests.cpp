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
    const bool non_null = empty != nullptr;
    ::operator delete(empty);
    if (!non_null) {
        return 1;
    }
    yuca_test::failAllocationAfter(1);
    void *allowed = ::operator new(3);
    bool rejected = false;
    try { ::operator delete(::operator new(5)); }
    catch (const std::bad_alloc &) { rejected = true; }
    // The failure must be one-shot; reporting/recovery allocations must work.
    void *recovery = ::operator new(7);
    ::operator delete(allowed);
    ::operator delete(recovery);
    yuca_test::failAllocationAfter(0);
    void *nothrow = ::operator new(1, std::nothrow);
    const bool nothrow_rejected = nothrow == nullptr;
    ::operator delete(nothrow);
    yuca_test::disableAllocationFailure();
    return rejected && nothrow_rejected && yuca_test::allocationSnapshot().live_bytes == before.live_bytes ? 0 : 1;
}
