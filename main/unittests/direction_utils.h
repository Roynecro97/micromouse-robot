#ifndef UNITTESTS_DIRECTION_UTILS_H
#define UNITTESTS_DIRECTION_UTILS_H

#include <maze_solver/direction.h>

#include <ostream>

namespace micromouse
{

std::ostream &operator<<(std::ostream &os, RelativeDirection rd)
{
    return os << enum2str(rd);
}

std::ostream &operator<<(std::ostream &os, Direction d)
{
    return os << enum2str(d);
}

}  // namespace micromouse

#endif  // UNITTESTS_DIRECTION_UTILS_H
