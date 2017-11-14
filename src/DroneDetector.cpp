#include "DroneDetector.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
#include <limits>
#include <ctime>
#include <algorithm> 

// The soccer field is 8 meters wide on the Y axis, and the camera resolution is 1920x1080    
#define PIXELS2METERS (8.0/1080.0)
// Correspond to the coordinates of the center of the field, as seen by the camera
#define X_OFF 7.0
#define Y_OFF 3.8
// Maximum difference between two psi measurements, in radians (0.35 radians is about 20 degrees)
#define PSI_MAX_DIFF 0.35
// If detection fails, change the exposure by this amount
#define DELTA_EXPOSURE 100
// Number of leds per drone
#define NR_LEDS 4

using namespace cv;
using namespace std;

DroneDetector::DroneDetector(unsigned int nrDrones) {
    this->nrDrones = nrDrones;
}

vector< vector<Point2f> > DroneDetector::PartitionPoints(vector<Point2f> points) {
    if(points.size() == 0) {
        vector<vector<Point2f> > empty(0);
	return empty;
    }
    Mat labels, centers;
    int nrClusters = points.size() / NR_LEDS;
    kmeans(points, nrClusters, labels, 
        TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0),
        3, KMEANS_PP_CENTERS, centers);
    vector<vector<Point2f> > partitioned(nrClusters);
    for(unsigned int i = 0; i < points.size(); i++) {
        int cluster = labels.at<int>(i);
        partitioned[cluster].push_back(points[i]);
    }
    return partitioned;    
}

DroneState DroneDetector::GetState(vector<Point2f> leds) {
    float xSum = 0;
    float ySum = 0;

    // Calculate average of leds
    for(unsigned int i = 0; i < leds.size(); i++ ) {
        xSum += leds[i].x;
        ySum += leds[i].y;
    }
    double xAvg = xSum / leds.size();
    double yAvg = ySum / leds.size();
    Point2f avg(xAvg, yAvg);

    // Find the two outermost LEDs 
    std::vector<Point2f> outerLeds(2);
    double max1 = 0;
    double max2 = 0;
    for(unsigned int i = 0; i < leds.size(); i++) {
        double max = norm(Mat(avg), Mat(leds[i]));
        if (max > max1 && max1 <= max2) {
            outerLeds[0] = leds[i];
            max1 = max;
        } 
        else if (max > max2 && max2 < max1) {
            outerLeds[1] = leds[i];
            max2 = max;
        }
    }

    Point2f center = (outerLeds[0] + outerLeds[1]) / 2;
    Point2f direction = avg - center;

    Point2f offset(X_OFF, Y_OFF);
    Point2f physicalCenter = center * PIXELS2METERS - offset;

    Vec4f line;
    fitLine(leds, line, CV_DIST_L2, 0, 0.01, 0.01);

    Point2f lineDirection = Point2f(line[0], line[1]);
    double psi;
    if(direction.dot(lineDirection) > 0)
        psi = atan2(line[1], line[0]);
    else
        psi = atan2(-line[1], -line[0]);

    Point2f dir(cos(psi), sin(psi));

    psi = fmod(psi + (M_PI/2), 2*M_PI);

    DroneState state;
    // According to conventions
    Point2f pos(-physicalCenter.y, physicalCenter.x);
    state.pos = pos;
    state.psi = psi;
    
    return state; 
}

void DroneDetector::UpdateStates(std::vector<DroneState>& states) {
    // First run there are no previous states
    if(previousStates.size() == 0) {
        if(states.size() != nrDrones) {
            // Not all drones are detected yet, wait for exposure tuning
            return;
        }
        for(unsigned int i = 0; i < states.size(); i++) {
            states[i].id = i;
        }
        previousStates = states;
        return;
    }

    /*
    *  There must always be at least as many previous states as there are states now.
    *  If this condition is broken, it's impossible to assign each state an ID,
    *  and previousStates[minIndex] with minIndex = -1 will result in a memory error.
    */
    assert(states.size() <= previousStates.size());

    // Replace any old states with new ones if found
    for(unsigned int i = 0; i < states.size(); i++) {
        int minIndex = -1;
        float minDistance = std::numeric_limits<float>::max();
        for(unsigned int j = 0; j < previousStates.size(); j++) {
            float distance = norm(Mat(states[i].pos), Mat(previousStates[j].pos));
            if(distance < minDistance) {
                minIndex = j;
                minDistance = distance;
            }    
        }
        states[i].id = previousStates[minIndex].id;
        // Delete the state to avoid assigning it twice
        previousStates.erase(previousStates.begin() + minIndex);
    }

    // Keep the old states, insert the new ones (duplicate have already been removed)
    previousStates.insert(previousStates.end(), states.begin(), states.end());
}

vector<DroneState> DroneDetector::FindDrones(Mat frame, int* deltaExposure) {
    // Find contours in frame
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat thresh;
    
    threshold(frame, thresh, 100, 255, CV_THRESH_BINARY);
    if(thresh.empty()){
        *deltaExposure = DELTA_EXPOSURE;
        return vector<DroneState>();
    }
    findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    if (contours.size() > NR_LEDS * nrDrones) {
        *deltaExposure = -DELTA_EXPOSURE;
        return vector<DroneState>();
    }
    else if (contours.size() < NR_LEDS * nrDrones) {
        *deltaExposure = DELTA_EXPOSURE;
    }
    else {
        *deltaExposure = 0;
    }

    if (contours.size() % NR_LEDS == 0) {
        // Turn contours into points 
        vector<Point2f> leds(contours.size());
        for(unsigned int i = 0; i < contours.size(); i++) {
            float radius;
            minEnclosingCircle(contours[i], leds[i], radius);
        } 

        vector<vector<Point2f> > partitioned = PartitionPoints(leds);
        vector<DroneState> states;
        states.reserve(partitioned.size());
        for(unsigned int i = 0; i < partitioned.size(); i++) {
            states.push_back(GetState(partitioned[i]));
        }
        UpdateStates(states);
        return states;

    }
    else {
        return vector<DroneState>();
    }
}
