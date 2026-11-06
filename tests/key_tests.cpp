#include "catch.hpp"
#include "yuca/key.hpp"
#include <limits>
#include <sstream>

TEST_CASE("Key comparison operators preserve numeric ordering and group equality", "[key][coverage]") {
    const long ids[] = {std::numeric_limits<long>::min(), 0, std::numeric_limits<long>::max()};
    for (long left : ids) {
        for (long right : ids) {
            const yuca::Key a(left, ":group");
            const yuca::Key b(right, ":group");
            REQUIRE((a < b) == (left < right));
            REQUIRE((a <= b) == (left <= right));
            REQUIRE((a > b) == (left > right));
            REQUIRE((a >= b) == (left >= right));
            REQUIRE((a == b) == (left == right));
        }
    }
    const yuca::Key original(42, ":one");
    const yuca::Key same_id_other_group(42, ":two");
    REQUIRE_FALSE(original == same_id_other_group);
    // Existing ordering uses ID only; do not assume a hash is collision-safe identity.
    REQUIRE(original <= same_id_other_group);
    REQUIRE(original >= same_id_other_group);
    REQUIRE(original.getId() == 42);
    REQUIRE(original.getGroup() == ":one");
    const yuca::Key empty;
    REQUIRE(empty.getId() == 0);
    REQUIRE(empty.getGroup().empty());
}

namespace {
    class FlushCountingBuffer : public std::stringbuf {
    public:
        int flushes = 0;
        int sync() override { ++flushes; return std::stringbuf::sync(); }
    };
}

TEST_CASE("Key diagnostic streams return and flush the supplied stream", "[key][coverage]") {
    yuca::Key key(-7, ":group");
    FlushCountingBuffer buffer;
    std::ostream stream(&buffer);
    REQUIRE(&(stream << key) == &stream);
    REQUIRE(buffer.str().find("Key(@") == 0);
    REQUIRE(buffer.str().find(", id=-7, group=:group)") != std::string::npos);
    REQUIRE(buffer.flushes == 1);

    yuca::StringKey text("two words", ":title");
    FlushCountingBuffer text_buffer;
    std::ostream text_stream(&text_buffer);
    REQUIRE(&(text_stream << text) == &text_stream);
    REQUIRE(text_buffer.str().find("StringKey(@") == 0);
    REQUIRE(text_buffer.str().find(", str_key=\"two words\", id=" + std::to_string(text.getId()) + ", group=:title)") != std::string::npos);
    REQUIRE(text_buffer.flushes == 1);
    REQUIRE(text.getString() == "two words");
}
