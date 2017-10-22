#ifndef DRONEDETECTOR
#define DRONEDETECTOR

#include <opencv2/imgproc.hpp>

class DroneDetector {
    public:
        struct DroneLocation {
            int deltaIntensity;
            double x;
            double y;
            double psi;
        };
        
        DroneLocation FindDrone(cv::Mat frame);
};

#endif
