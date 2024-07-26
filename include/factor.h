#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <utility>
#include <stdexcept>
#include "ceres/ceres.h"

namespace factor {

class PesudorangeFactorCostFunctor {
    public:
        PesudorangeFactorCostFunctor(const std::vector<double>& sv_position, double pesudorange)
            : sv_position_(sv_position), pesudorange_(pesudorange) {}

        template <typename T>
        bool operator()(const T* state, T* residual) const {
            const double c = 299792458.0;
            // state[0]: receiver x
            // state[1]: receiver y
            // state[2]: receiver z
            // state[3]: receiver clock bias

            T dx = state[0] - T(sv_position_[0]);
            T dy = state[1] - T(sv_position_[1]);
            T dz = state[2] - T(sv_position_[2]);
            T clock_bias = state[3] * T(c);

            T estimated_pr = ceres::sqrt(dx * dx + dy * dy + dz * dz) + clock_bias;

            residual[0] = estimated_pr - T(pesudorange_);

            return true;
        }

    private:
        std::vector<double> sv_position_;
        double pesudorange_;
};
class DopplerFactorCostFunctor {
    public:
        DopplerFactorCostFunctor(const std::vector<double>& sv_velocity, double doppler, double time_interval)
            : sv_velocity_(sv_velocity), doppler_(doppler), time_interval_(time_interval) {}

        template <typename T>
        bool operator()(const T* const prev_state, const T* const next_state, T* residual) const {
            const double c = 299792458.0;  // speed of light in m/s
            const double L1_frequency = 1575.42e6;  // L1 signal frequency in Hz

            T user_velocity[3];
            for (int i = 0; i < 3; ++i) {
                user_velocity[i] = (next_state[i] - prev_state[i]) / (T(time_interval_) + next_state[3] - prev_state[3]);
            }

            T relative_velocity[3];
            for (int i = 0; i < 3; ++i) {
                relative_velocity[i] = user_velocity[i] - T(sv_velocity_[i]);
            }

            T relative_speed = ceres::sqrt(relative_velocity[0] * relative_velocity[0] +
                                        relative_velocity[1] * relative_velocity[1] +
                                        relative_velocity[2] * relative_velocity[2]);

            // Convert Doppler shift to velocity
            T doppler_velocity = T(doppler_) * T(c) / T(L1_frequency);

            residual[0] = relative_speed - doppler_velocity;

            return true;
        }

    private:
        std::vector<double> sv_velocity_;
        double doppler_;
        double time_interval_;
};

};