#ifndef MAIN_POSITION_H
#define MAIN_POSITION_H

#include <misc_utils/angle.h>
#include <misc_utils/physical_size.h>
#include <misc_utils/strongly_typed.h>

namespace micromouse
{

struct x_tag;
struct y_tag;
using XCoord = StronglyTyped<meters, x_tag>;
using YCoord = StronglyTyped<meters, y_tag>;

struct Position
{
    XCoord x;
    YCoord y;
    Angle theta;  // Clockwise rotation from x-axis in radians.
};

}  // namespace micromouse

#endif  // MAIN_POSITION_H
