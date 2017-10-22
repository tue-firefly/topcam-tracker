#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE DroneDetector
#include <boost/test/unit_test.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "../src/DroneDetector.h"

using namespace cv;

BOOST_AUTO_TEST_CASE(detect_drone)
{
    Mat image = imread("drone.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector;
    DroneDetector::DroneLocation loc = detector.FindDrone(grey);
    BOOST_CHECK(loc.deltaIntensity == 0);
}


BOOST_AUTO_TEST_CASE(too_many)
{
    Mat image = imread("too_many.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector;
    DroneDetector::DroneLocation loc = detector.FindDrone(grey);
    BOOST_CHECK(loc.deltaIntensity == -100);
}

BOOST_AUTO_TEST_CASE(too_few)
{
    Mat image = imread("too_few.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat grey;
    image.convertTo(grey, CV_8UC1);
    DroneDetector detector;
    DroneDetector::DroneLocation loc = detector.FindDrone(grey);
    BOOST_CHECK(loc.deltaIntensity == 100);
}
