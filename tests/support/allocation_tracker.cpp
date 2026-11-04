#include "allocation_tracker.hpp"
#include <atomic>
#include <cstdlib>
#include <limits>
#include <new>

namespace {
    struct alignas(std::max_align_t) Header { std::size_t bytes; };
    std::atomic<std::size_t> allocations{0};
    std::atomic<std::size_t> allocated_bytes{0};
    std::atomic<std::size_t> live_bytes{0};
    std::atomic<std::size_t> peak_bytes{0};
    std::atomic<std::size_t> failure_countdown{std::numeric_limits<std::size_t>::max()};

    void *allocate(std::size_t bytes) {
        auto remaining = failure_countdown.load(std::memory_order_relaxed);
        while (remaining != std::numeric_limits<std::size_t>::max()) {
            const auto next = remaining == 0 ? std::numeric_limits<std::size_t>::max() : remaining - 1;
            if (failure_countdown.compare_exchange_weak(remaining, next, std::memory_order_relaxed)) {
                if (remaining == 0) {
                    throw std::bad_alloc();
                }
                break;
            }
        }
        if (bytes > std::numeric_limits<std::size_t>::max() - sizeof(Header)) {
            throw std::bad_alloc();
        }
        void *storage = nullptr;
        while (!(storage = std::malloc(sizeof(Header) + bytes))) {
            const auto handler = std::get_new_handler();
            if (!handler) {
                throw std::bad_alloc();
            }
            handler();
        }
        auto *header = ::new(storage) Header{bytes};
        allocations.fetch_add(1, std::memory_order_relaxed);
        allocated_bytes.fetch_add(bytes, std::memory_order_relaxed);
        const auto live = live_bytes.fetch_add(bytes, std::memory_order_relaxed) + bytes;
        auto peak = peak_bytes.load(std::memory_order_relaxed);
        while (live > peak && !peak_bytes.compare_exchange_weak(peak, live, std::memory_order_relaxed)) {}
        return header + 1;
    }

    void release(void *pointer) noexcept {
        if (pointer) {
            auto *header = static_cast<Header *>(pointer) - 1;
            live_bytes.fetch_sub(header->bytes, std::memory_order_relaxed);
            std::free(header);
        }
    }
}

void *operator new(std::size_t bytes) { return allocate(bytes); }
void *operator new[](std::size_t bytes) { return allocate(bytes); }
void operator delete(void *pointer) noexcept { release(pointer); }
void operator delete[](void *pointer) noexcept { release(pointer); }
void operator delete(void *pointer, std::size_t) noexcept { release(pointer); }
void operator delete[](void *pointer, std::size_t) noexcept { release(pointer); }
void *operator new(std::size_t bytes, const std::nothrow_t &) noexcept {
    try { return allocate(bytes); } catch (...) { return nullptr; }
}
void *operator new[](std::size_t bytes, const std::nothrow_t &) noexcept {
    try { return allocate(bytes); } catch (...) { return nullptr; }
}
void operator delete(void *pointer, const std::nothrow_t &) noexcept { release(pointer); }
void operator delete[](void *pointer, const std::nothrow_t &) noexcept { release(pointer); }

namespace yuca_test {
    AllocationSnapshot allocationSnapshot() noexcept {
        return {allocations.load(std::memory_order_relaxed), allocated_bytes.load(std::memory_order_relaxed),
                live_bytes.load(std::memory_order_relaxed), peak_bytes.load(std::memory_order_relaxed)};
    }

    void resetAllocationPeak() noexcept {
        peak_bytes.store(live_bytes.load(std::memory_order_relaxed), std::memory_order_relaxed);
    }

    void failAllocationAfter(std::size_t successful_calls) noexcept {
        failure_countdown.store(successful_calls, std::memory_order_relaxed);
    }

    void disableAllocationFailure() noexcept {
        failure_countdown.store(std::numeric_limits<std::size_t>::max(), std::memory_order_relaxed);
    }
}
