#ifndef DRONE_STATE
#define DRONE_STATE

#include <opencv2/core.hpp> 

struct DroneState {
    unsigned int id;
    cv::Point2f pos;
    double psi;
};

#endif
