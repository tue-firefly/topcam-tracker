#ifndef DRONEDETECTOR
#define DRONEDETECTOR

#include <opencv2/imgproc.hpp>
#include "DroneState.h"

class DroneDetector {
    public:
        DroneDetector(unsigned int nrDrones);
			
        DroneState GetState(std::vector<cv::Point2f> leds);
        std::vector<DroneState> FindDrones(cv::Mat frame, int* deltaIntensity);

    private:
        unsigned int nrDrones;	
        std::vector<DroneState> previousStates;

        std::vector< std::vector<cv::Point2f> > PartitionPoints(std::vector<cv::Point2f> points); 
        void UpdateStates(std::vector<DroneState>& states);
};

#endif
