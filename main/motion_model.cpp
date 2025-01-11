#include "motion_model.h"

#include <cmath>
#include <cstdint>

namespace micromouse
{

Position update_pos(const Position &current, meters_per_second vl, meters_per_second vr, const seconds &dt) noexcept
{
    Position pos;
    if (vr == vl)
    {
        // Driving in straight line singularity - regular kinematic equations
        pos.x = current.x + XCoord{vr * std::cos(current.theta) * dt};
        pos.y = current.y + YCoord{vr * std::sin(current.theta) * dt};
        pos.theta = current.theta;
    }
    else
    {
        static constexpr auto l = distance_between_wheels;
        const auto r = (l / 2) * (vl + vr) / (vl - vr);
        const auto omega = (vl - vr) / l;
        const Angle dtheta{(omega * dt).count()};
        const auto sin_dtheta = std::sin(dtheta);
        const auto cos_dtheta = std::cos(dtheta);
        const auto translated_icc_x = XCoord{r * std::sin(current.theta)};
        const auto translated_icc_y = YCoord{r * std::cos(current.theta)};
        const auto icc_x = current.x - translated_icc_x;
        const auto icc_y = current.y + translated_icc_y;
        pos.x = XCoord{cos_dtheta * translated_icc_x.get()} + XCoord{sin_dtheta * translated_icc_y.get()} + icc_x;
        pos.y = YCoord{sin_dtheta * translated_icc_x.get()} - YCoord{cos_dtheta * translated_icc_y.get()} + icc_y;
        pos.theta = current.theta + dtheta;
    }

    return pos;
}

PosJacobian pos_jacobian(const Position &pos, meters_per_second vl, meters_per_second vr, const seconds &dt) noexcept
{
    PosJacobian jacobian = PosJacobian::Identity();
    const auto v = (vl + vr) / 2;
    jacobian(0, 2) = (-v * sin(pos.theta) * dt).count();
    jacobian(1, 2) = (v * cos(pos.theta) * dt).count();
    return jacobian;
}

}  // namespace micromouse
