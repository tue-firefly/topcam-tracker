#include "DroneDetector.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
#include <limits>
#include <ctime>
#include <algorithm> 

// Maximum difference between two psi measurements, in radians (0.35 radians is about 20 degrees)
#define PSI_MAX_DIFF 0.35
// If detection fails, change the exposure by this amount
#define DELTA_EXPOSURE 100
// Number of leds per drone
#define NR_LEDS 3

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
    std::cout << "New code\n";
    std::vector<double> Distance;
    Distance.resize(3);
    //A left bottom, B right top, C left top, Lower right LED is off
    //Distance[0] = 01 -> AB
    //Distance[1] = 12 -> BC
    //DIstance[2] = 20 -> CA

    std::cout << "Fill in the distances and find the largest one\n";
    float max = 0;
    int maxi = 0;
    for(unsigned int i = 0; i < leds.size(); i++){
	Distance[i] = norm(leds[(i+1)%NR_LEDS]-leds[i]);
        if(Distance[i] > max){
	    max = Distance[i];
	    maxi = i;
	}
    }
    int C = (maxi+NR_LEDS-1)%NR_LEDS;

    std::cout << "Place AB to Distance[0]\n";
    if(maxi != 0){
	float tempDist = Distance[0];
	Distance[0] = Distance[maxi];
	Distance[maxi] = tempDist;
    }
    std::cout << "Place C to final LED position\n";
    if(NR_LEDS-1 != C){
    	Point2f temp = leds[NR_LEDS-1];
    	leds[NR_LEDS-1] = leds[C];
    	leds[C] = temp;
    }
    std::cout << "Place A to 0 and B to 1\n";
    Point2f first = leds[2] - leds[0];
    float tempX = first.x;
    first.x = -first.y;
    first.y = tempX;
    if(first.dot(leds[2]-leds[1]) < 0){
	Point2f temp = leds[0];
	leds[0] = leds[1];
	leds[1] = temp;
    }
    Point2f center;
    center.x = (leds[0].x + leds[1].x)/2;
    center.y = (leds[0].y + leds[1].y)/2;
    Point2f top;
    top.x = (leds[2].x + leds[1].x)/2;
    top.y = (leds[2].y + leds[1].y)/2;
    double direction[2] = {top.x - center.x, top.y - center.y };



    std::cout << "calculate physical center\n";
    Point2f offset(X_OFF, Y_OFF);
    Point2f physicalCenter = center * PIXELS2METERS - offset;

    //Vec4f line;
    //fitLine(leds, line, CV_DIST_L2, 0, 0.01, 0.01);

    //Point2f lineDirection = Point2f(line[0], line[1]);
    std::cout <<"Calculate psi\n";
    double psi;

    psi = atan2(direction[1],direction[0]);
    Point2f dir(cos(psi), sin(psi));

    psi = fmod(psi + (M_PI/2), 2*M_PI);
    
    std::cout <<"Make new state\n";
    DroneState state;
    // According to conventions
    Point2f pos(-physicalCenter.y, physicalCenter.x);
    state.pos = pos;
    state.psi = psi;
    //std::cout << state.pos << std::endl;
    //std::cout << state.psi << std::endl;
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
