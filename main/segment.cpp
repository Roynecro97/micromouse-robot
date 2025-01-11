#include "segment.h"

#include <limits>

namespace micromouse::segment
{

float Segment::intersection_distance(const Segment &other) const noexcept
{
    const auto intersect = m_line.intersection(other.m_line);
    if (contains(intersect) && other.contains(intersect))
    {
        return (m_p1 - intersect).norm();
    }

    return std::numeric_limits<float>::infinity();
}

}  // namespace micromouse::segment
