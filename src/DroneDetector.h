#ifndef DRONEDETECTOR
#define DRONEDETECTOR

#include <opencv2/imgproc.hpp>

class DroneDetector {
    public:
        struct DroneState {
            unsigned int id;
            cv::Point2f pos;
            double psi;
        };

        DroneDetector(unsigned int nr_drones);
			
        DroneState GetState(std::vector<cv::Point2f> leds);
        std::vector<DroneState> FindDrones(cv::Mat frame, int* deltaIntensity);

    private:
        unsigned int nr_drones;	
        std::vector<DroneState> previousStates;

        std::vector< std::vector<cv::Point2f> > PartitionPoints(std::vector<cv::Point2f> points); 
        void UpdateStates(std::vector<DroneState>& states);

};

#endif
