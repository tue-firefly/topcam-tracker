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
        
	DroneLocation GetLocation(std::vector< std::vector<cv::Point> > contours);
        DroneLocation FindDrones(cv::Mat frame);
};

#endif
