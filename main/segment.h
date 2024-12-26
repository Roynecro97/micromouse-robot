#ifndef MAIN_SEGMENT_H
#define MAIN_SEGMENT_H

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <cmath>

namespace micromouse::inline segment
{

inline constexpr auto epsilon = 1e-6f;

using Line = Eigen::Hyperplane<float, 2>;

class Segment
{
public:
    Segment(const Eigen::Vector2f &p1, const Eigen::Vector2f &p2) noexcept
        : m_p1{p1}
        , m_p2{p2}
        , m_length{(p1 - p2).norm()}
        , m_line(Line::Through(p1, p2))
    {
    }

    const auto &p1() const & noexcept { return m_p1; }
    const auto &p2() const & noexcept { return m_p2; }
    auto length() const noexcept { return m_length; }
    const auto &line() const & noexcept { return m_line; }

    auto contains(const Eigen::Vector2f &p) const noexcept
    {
        return std::abs((m_p1 - p).norm() + (m_p2 - p).norm() - m_length) < epsilon;
    }

    float intersection_distance(const Segment &other) const noexcept;

private:
    Eigen::Vector2f m_p1;
    Eigen::Vector2f m_p2;
    float m_length;
    Line m_line;
};

}  // namespace micromouse::inline segment

#endif  // MAIN_SEGMENT_H
