#ifndef MAIN_MOTION_MODEL_H
#define MAIN_MOTION_MODEL_H

#include <maze_solver/direction.h>
#include <misc_utils/angle.h>
#include <misc_utils/physical_size.h>
#include <misc_utils/strongly_typed.h>

#include "position.h"

#include <Eigen/Dense>

#include <chrono>

namespace micromouse
{

static constexpr auto pos_dimension = 3;
using PosJacobian = Eigen::Matrix<float, pos_dimension, pos_dimension>;
using seconds = std::chrono::duration<float>;
static constexpr auto distance_between_wheels = unit_cast<meters>(10.1_cm);

/**
 * @brief Calculate new estimated position based on current position, motors velocity and time frame.
 * @see https://www.cs.columbia.edu/~allen/F17/NOTES/icckinematics.pdf
 *
 * @param current The current position.
 * @param vl Velocity of left motor.
 * @param vr Velocity of right motor.
 * @param dtime Time difference between current position and new position.
 * @return The new position.
 */
Position update_pos(const Position &current, meters_per_second vl, meters_per_second vr, const seconds &dt) noexcept;

template <typename Rep, typename Ratio>
auto update_pos(
    const Position &current,
    meters_per_second vl,
    meters_per_second vr,
    const std::chrono::duration<Rep, Ratio> &dt
) noexcept
{
    return update_pos(current, vl, vr, duration_cast<seconds>(dt));
}

/**
 * @brief Calculate the jacobian of the position model with respect to theta.
 *
 * @param pos The current position.
 * @param vl Velocity of left motor.
 * @param vr Velocity of right motor.
 * @param dtime Time difference between calculations.
 * @return The jacobian matrix of the position.
 */
PosJacobian pos_jacobian(const Position &pos, meters_per_second vl, meters_per_second vr, const seconds &dt) noexcept;

template <typename Rep, typename Ratio>
auto pos_jacobian(
    const Position &pos,
    meters_per_second vl,
    meters_per_second vr,
    const std::chrono::duration<Rep, Ratio> &dt
) noexcept
{
    return pos_jacobian(pos, vl, vr, duration_cast<seconds>(dt));
}

}  // namespace micromouse

#endif  // MAIN_MOTION_MODEL_H
