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
    BOOST_CHECK_EQUAL(deltaExposure, 0);
    BOOST_CHECK_EQUAL(states.size(), 1);
}

BOOST_AUTO_TEST_CASE(detect_drones)
{
    Mat image = imread("3drones.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(3);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> state = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK_EQUAL(deltaExposure, 0);
    BOOST_CHECK_EQUAL(state.size(), 3);
    for(int i = 0; i < 3; i++) {
        cout << "Drone " << state[i].id << ": " << state[i].pos << " at angle: " << state[i].psi << "\n";
    }
}

// Checks that if at first 3 drones are detected and the next iteration only 1,
// deltaExposure > 0 and the state has matching id to one closest in the first iteration
BOOST_AUTO_TEST_CASE(detect_drones_partial)
{
    Mat image = imread("3drones.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(3);
    int deltaExposure = 0;
    // Find three drones, set initial drone states
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK_EQUAL(states.size(), 3);
    
    Mat image2 = imread("drone.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey2;
    image2.convertTo(grey2, CV_8UC1);
    std::vector<DroneDetector::DroneState> state = detector.FindDrones(grey2, &deltaExposure);
    BOOST_CHECK_EQUAL(deltaExposure, 100);
    BOOST_CHECK_EQUAL(state.size(), 1);
    bool found = false;
    for(int i = 0; i < 3; i++) {
        if(states[i].id == state[0].id) {
            BOOST_CHECK_EQUAL(states[i].pos, state[0].pos);
            BOOST_CHECK_EQUAL(states[i].psi, state[0].psi);
            found = true;
        }
    }
    BOOST_CHECK(found);

    Mat image3 = imread("2drones.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey3;
    image3.convertTo(grey3, CV_8UC1);
    std::vector<DroneDetector::DroneState> states2 = detector.FindDrones(grey3, &deltaExposure);
    BOOST_CHECK_EQUAL(deltaExposure, 100);
    BOOST_CHECK_EQUAL(state.size(), 1);
    int found2 = 0;
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 2; j++) {
            if(states[i].id == states2[j].id) {
                BOOST_CHECK_EQUAL(states[i].pos, states2[j].pos);
                BOOST_CHECK_EQUAL(states[i].psi, states2[j].psi);
                found2++;
            }
        }
    }
    BOOST_CHECK_EQUAL(found2, 2);
}



BOOST_AUTO_TEST_CASE(too_many)
{
    Mat image = imread("too_many.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(1);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK_EQUAL(deltaExposure, -100);
    BOOST_CHECK_EQUAL(states.size(), 0);
}


BOOST_AUTO_TEST_CASE(too_few)
{
    Mat image = imread("too_few.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector(1);
    int deltaExposure = 0;
    std::vector<DroneDetector::DroneState> states = detector.FindDrones(grey, &deltaExposure);
    BOOST_CHECK_EQUAL(deltaExposure, 100);
    BOOST_CHECK_EQUAL(states.size(), 0);
}
