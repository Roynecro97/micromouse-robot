#ifndef UNITTESTS_STRONGLY_TYPED_UTILS_H
#define UNITTESTS_STRONGLY_TYPED_UTILS_H

#include <misc_utils/strongly_typed.h>

#include <ostream>

namespace micromouse
{
template <StrongType T>
std::ostream &operator<<(std::ostream &os, const T &st)
{
    return os << "StronglyTyped{" << st.get() << "}";
}
}  // namespace micromouse

#endif  // UNITTESTS_STRONGLY_TYPED_UTILS_H
