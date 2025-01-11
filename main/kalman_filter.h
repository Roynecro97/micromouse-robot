#ifndef MAIN_KALMAN_FILTER_H
#define MAIN_KALMAN_FILTER_H

#include "distance_sensor.h"
#include "motion_model.h"
#include "position.h"

#include <Eigen/Dense>

#include <cmath>
#include <vector>

namespace micromouse
{

using JacobianT = Eigen::Matrix<float, pos_dimension, DistanceSensors::sensor_count>;
using PosCov = Eigen::Matrix<float, pos_dimension, pos_dimension>;
using MeasurementCov = Eigen::Matrix<float, DistanceSensors::sensor_count, DistanceSensors::sensor_count>;

class KalmanFilter
{
public:
    /**
     * @brief Kalman filter update.
     * @see https://en.wikipedia.org/wiki/Kalman_filter
     *
     * @param pos Current position.
     * @param motion_jacobian The jacobian of the position with respect to the position vector.
     * @param sensors_error Error between real sensor measurement and predicted sensor measurment.
     * @param sensors_jacobian The jacobian of the sensor measurement prediction with respect to the position vector.
     * @param encoder_cov The covariance value for the encoders (how much do we believe them)
     * @param sensor_cov The covariance value for the distance sensors (how much do we believe them)
     *
     * @return updated position.
     */
    Position operator()(
        const Position &pos,
        const PosJacobian &motion_jacobian,
        const DistanceSensors::Measurements &sensors_error,
        const DistanceSensors::Jacobian &sensors_jacobian
    ) noexcept;

private:
    // Covariance
    PosCov P = PosCov::Identity();

    // Process and measurement noise
    static const PosCov Q;
    static const MeasurementCov R;
};

}  // namespace micromouse

#endif  // MAIN_KALMAN_FILTER_H
