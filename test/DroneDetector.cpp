#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE DroneDetector
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "../src/DroneDetector.h"

using namespace cv;
using namespace std;

BOOST_AUTO_TEST_CASE(detect_drone)
{
    Mat image = imread("drone.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(1);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK(deltaExposure == 0);
    BOOST_CHECK(states.size() == 1);
}

BOOST_AUTO_TEST_CASE(detect_drones)
{
    Mat image = imread("drones.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(3);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> state = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK(deltaExposure == 0);
    BOOST_CHECK(state.size() == 3);
    for(int i = 0; i < 3; i++) {
        cout << "Drone " << i << ": " << state[i].pos << " at angle: " << state[i].psi << "\n";
    }
}

BOOST_AUTO_TEST_CASE(too_many)
{
    Mat image = imread("too_many.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(1);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK(deltaExposure == -100);
    BOOST_CHECK(states.size() == 0);
}


BOOST_AUTO_TEST_CASE(too_few)
{
    Mat image = imread("too_few.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(1);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK(deltaExposure == 100);
    BOOST_CHECK(states.size() == 0);
}
