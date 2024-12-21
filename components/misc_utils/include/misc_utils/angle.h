#ifndef MISC_UTILS_ANGLE_H
#define MISC_UTILS_ANGLE_H

#include "typing_utils.h"
#include "value_range.h"

#include <numbers>

namespace micromouse
{

template <
    PartialArithmetic T = float,
    T Low = -std::numbers::pi_v<make_floating_point<T>>,
    T Cycle = 2 * std::numbers::pi_v<make_floating_point<T>>>
using BasicAngleRange = ValueRange<T, Low, Low + Cycle, Mode::RightOpen>;

template <float Low = -std::numbers::pi_v<float>, float Cycle = 2 * std::numbers::pi_v<float>>
using FloatingAngleRange = BasicAngleRange<float, Low, Cycle>;

using AngleRange = FloatingAngleRange<>;

using Angle = ConstrainedValue<AngleRange>;

}  // namespace micromouse

#endif  // MISC_UTILS_ANGLE_H
