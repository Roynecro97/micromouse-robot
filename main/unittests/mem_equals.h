#ifndef UNITTESTS_MEM_EQUALS_H
#define UNITTESTS_MEM_EQUALS_H

#include <gmock/gmock.h>

#include <cstring>

namespace micromouse::tests
{

using namespace std::string_literals;

MATCHER_P(
    MemoryEquals,
    expected,
    "memory "s + (negation ? "not " : "") + "equals to " + ::testing::PrintToString(expected)
)
{
    static_assert(sizeof(arg) == sizeof(expected));
    return std::memcmp(&arg, &expected, sizeof(arg)) == 0;
}

}  // namespace micromouse::tests

#endif  // UNITTESTS_MEM_EQUALS_H
