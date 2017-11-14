#ifndef DRONE_STATE
#define DRONE_STATE

#include <opencv2/core.hpp> 

// The soccer field is 8 meters wide on the Y axis, and the camera resolution is 1920x1080    
#define PIXELS2METERS (8.0/1080.0)
// Correspond to the coordinates of the center of the field, as seen by the camera
#define X_OFF 7.0
#define Y_OFF 3.8

struct DroneState {
    unsigned int id;
    cv::Point2f pos;
    double psi;
};

#endif
