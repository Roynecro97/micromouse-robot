#include "kalman_filter.h"

namespace micromouse
{

static const PosCov I = PosCov::Identity();
const PosCov KalmanFilter::Q = 1e-2f * I;  // The covariance of the process noise.
const MeasurementCov KalmanFilter::R = Eigen::DiagonalMatrix<float, DistanceSensors::sensor_count>(
    0.6f,
    1.0f,
    0.6f,
    1.0f,
    0.6f
);  // The covariance of the observation noise.

Position KalmanFilter::operator()(
    const Position &pos,
    const PosJacobian &motion_jacobian,
    const DistanceSensors::Measurements &sensors_error,
    const DistanceSensors::Jacobian &sensors_jacobian
) noexcept
{
    // Predict (a priori) state and covariance estimate
    Eigen::Vector<float, pos_dimension> pos_vec{pos.x->count(), pos.y->count(), pos.theta.get()};
    P = motion_jacobian * P * motion_jacobian.transpose() + Q;

    // Measurement update
    const JacobianT sensors_jacobian_T = sensors_jacobian.transpose();
    const MeasurementCov S = sensors_jacobian * P * sensors_jacobian_T + R;
    const JacobianT K = P * sensors_jacobian_T * S.inverse();

    // Update (a posteriori) state and covariance estimate
    P = (I - K * sensors_jacobian) * P;
    const Eigen::Vector3f updated_pos = pos_vec + K * sensors_error;
    return Position{XCoord{updated_pos.x()}, YCoord{updated_pos.y()}, Angle{updated_pos.z()}};
}

}  // namespace micromouse
