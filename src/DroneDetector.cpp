#include "DroneDetector.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
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


double oldPsi;
double oldTime;

using namespace cv;
using namespace std;

DroneDetector::DroneDetector(unsigned int nr_drones) {
    this->nr_drones = nr_drones;
}

vector< vector<Point2f> > DroneDetector::PartitionPoints(vector<Point2f> points) {
    vector< vector<Point2f> > partitioned(1);
    partitioned[0] = (points);
    return partitioned;    
}

DroneDetector::DroneLocation DroneDetector::GetLocation(vector<Point2f> leds) {
    float xSum = 0;
    float ySum = 0;

    // Calculate weighted average of leds
    for(unsigned int i = 0; i < leds.size(); i++ ) {
        xSum += leds[i].x;
        ySum += leds[i].y;
    }
    double xAvg = xSum / leds.size();
    double yAvg = ySum / leds.size();
    Point2f avg(xAvg, yAvg);

    // Find the two outermost LEDs 
    vector<Point2f> outerLeds(2);
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
    double time = clock() / (double) CLOCKS_PER_SEC;
    if (time < oldTime + 1 && (abs(psi-oldPsi) > PSI_MAX_DIFF || abs(psi-oldPsi) < (2*M_PI - PSI_MAX_DIFF))) {
        // If within 1 second the difference in angle is more than PSI_MAX_DIFF, 
        // this measurement is invalid
        psi = oldPsi;
    }
    else {
        oldPsi = psi;
        oldTime = time;
    }

    DroneLocation loc;
    loc.deltaIntensity = 0;
    // According to conventions
    loc.x = -physicalCenter.y;
    loc.y = physicalCenter.x;
    loc.psi = psi;
    
    return loc; 
}

vector<DroneDetector::DroneLocation> DroneDetector::FindDrones(Mat frame, int* deltaIntensity) {
    // Find contours in frame
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat thresh;
    
    threshold(frame, thresh, 100, 255, CV_THRESH_BINARY);
    findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    cout << "Found " << contours.size() << " contours\n";

    if (contours.size() == nr_drones * 4) {
        // Turn contours into points 
        vector<Point2f> leds(contours.size());
        for(unsigned int i = 0; i < contours.size(); i++) {
            float radius;
            minEnclosingCircle(contours[i], leds[i], radius);
        } 

        vector<vector<Point2f> > partitioned = PartitionPoints(leds);
        vector<DroneLocation> locations;
        locations.reserve(nr_drones);
        for(unsigned int i = 0; i < partitioned.size(); i++) {
            locations.push_back(GetLocation(partitioned[i]));
        }
        return locations;

    }
    else {
        if(contours.size() > nr_drones * 4) {
            *deltaIntensity = -DELTA_EXPOSURE;
        } 
        else {
            *deltaIntensity = DELTA_EXPOSURE; 
        }
        return vector<DroneDetector::DroneLocation>();
    }
}
