#include <csignal>
#include <cstddef>

// Modern libc can define SIGSTKSZ as a runtime expression. Reproduce that on
// every Unix host so the vendored runner cannot regress to a fixed-size array.
inline std::size_t runtimeSignalStackSize() { return 64 * 1024; }
#undef SIGSTKSZ
#define SIGSTKSZ runtimeSignalStackSize()
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("Catch accepts a runtime-sized alternate signal stack") {
    REQUIRE(runtimeSignalStackSize() >= 64 * 1024);
}
