#ifndef UNITTESTS_VALUE_RANGE_UTILS_H
#define UNITTESTS_VALUE_RANGE_UTILS_H

#include <misc_utils/value_range.h>

#include <ostream>

namespace micromouse
{

std::ostream &operator<<(std::ostream &os, Mode mode)
{
    return os << enum2str(mode);
}

template <typename R, bool C>
std::ostream &operator<<(std::ostream &os, const ConstrainedValue<R, C> &st)
{
    return os << "ConstrainedValue<low = " << R::low << ", high = " << R::high << ", mode = " << R::mode
              << ", cycle = " << std::boolalpha << C << ">{" << st.get() << "}";
}

}  // namespace micromouse

#endif  // UNITTESTS_VALUE_RANGE_UTILS_H
