#include "DroneDetector.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
#include <ctime>

double oldPsi;
double oldTime;

using namespace cv;

<<<<<<< Updated upstream
DroneDetector::DroneLocation DroneDetector::FindDrone(Mat frame) {
    std::vector<std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    Mat thresh;
    
    threshold(frame, thresh, 100, 255, CV_THRESH_BINARY);
    findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
    std::cout << "Found " << contours.size() << " contours\n";
    if (contours.size() == 4) {
        std::vector<Point2f> leds(contours.size());
        float xSum = 0;
        float ySum = 0;

        // Calculate weighted average of contours
        for(unsigned int i = 0; i < contours.size(); i++ ) {
            float radius;
            minEnclosingCircle((Mat)contours[i], leds[i], radius);
            circle(frame, leds[i], radius, Scalar(255, 0, 0), 2, 8, 0);
            xSum += leds[i].x;
            ySum += leds[i].y;
=======
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
>>>>>>> Stashed changes
        }
        double xAvg = xSum / contours.size();
        double yAvg = ySum / contours.size();
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

        double PIXELS2METERS = 8.0/1080.0;
        double X_OFF = 7.0;
        double Y_OFF = 3.8;

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

        float l = 100;

        Point2f dir(cos(psi), sin(psi));
        cv::arrowedLine(frame, center, center + dir * l, Scalar(255, 0, 0));

        // Offset for mounting
        psi = fmod(psi + (M_PI/2), 2*M_PI);
        double time = std::clock() / (double) CLOCKS_PER_SEC;
        if (time < oldTime + 1 && abs(psi-oldPsi) > 0.35 && abs(psi-oldPsi) < 5.9) {
            psi = oldPsi;
            std::cout << "========== PSI INVALID ========\n";
        }
        else {
            oldPsi = psi;
            oldTime = time;
        }

<<<<<<< Updated upstream
        std::cout << "x: " << physicalCenter.x << ", y: " << physicalCenter.y << "psi: " << (psi / M_PI * 180) << "\n";
	DroneLocation loc;
	loc.deltaIntensity = 0;
	loc.x = -physicalCenter.y;
	loc.y = physicalCenter.x;
	loc.psi = psi;
	
        // According to conventions
        return loc; 
=======
vector<DroneState> DroneDetector::FindDrones(Mat frame, int* deltaExposure) {
    // Find contours in frame
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Mat thresh;
    
    threshold(frame, thresh, 100, 255, CV_THRESH_BINARY);
    findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

    if (contours.size() > NR_LEDS * nrDrones) {
        *deltaExposure = -DELTA_EXPOSURE;
        return vector<DroneState>();
    }
    else if (contours.size() < NR_LEDS * nrDrones) {
        *deltaExposure = DELTA_EXPOSURE;
>>>>>>> Stashed changes
    }
    else {
	DroneLocation loc;
        if(contours.size() > 4) {
            loc.deltaIntensity = -100;
        } 
        else {
            loc.deltaIntensity = 100; 
        }
	return loc;
    }
}
