#include <iostream>
#include <iomanip>
#include <math.h>

#include <cstdio>
#include <ctime>

#include "FrameObserver.h"
#include "ApiController.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

namespace AVT {
namespace VmbAPI {
namespace Examples {

boost::asio::io_service io_service;
double exposure = 2000;

double oldPsi;
double oldTime;

//
// We pass the camera that will deliver the frames to the constructor
//
// Parameters:
//  [in]    pCamera             The camera the frame was queued at
//
FrameObserver::FrameObserver( CameraPtr pCamera, const std::string& ip, const std::string& port)
    :   IFrameObserver( pCamera )
    ,   udp(io_service, ip, port)
{
}

FrameObserver::DroneLocation FrameObserver::FindDrone(Mat frame) {
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

        std::cout << "x: " << physicalCenter.x << ", y: " << physicalCenter.y << "psi: " << (psi / M_PI * 180) << "\n";
        // According to conventions
        return {
            0,
            -physicalCenter.y,
            physicalCenter.x,
            psi
        };
    }
    else {
        if(contours.size() > 4) {
            return {
                -100,
                0,0,0
            };
        } 
        else {
            return {
                100,
                0,0,0
            };
        }
    }
}

//
// This is our callback routine that will be executed on every received frame.
// Triggered by the API.
//
// Parameters:
//  [in]    pFrame          The frame returned from the API
//
void FrameObserver::FrameReceived( const FramePtr pFrame )
{
    std::cout << "Frame\n";
    if(! SP_ISNULL( pFrame ) )
    {

        VmbUint32_t nWidth = 0;
        VmbUint32_t nHeight = 0;
        VmbUchar_t *pImage = NULL;

        pFrame->GetWidth( nWidth );
        pFrame->GetHeight( nHeight );
        pFrame->GetImage( pImage );

        int size = nWidth * nHeight;
        if (size > 0) {
            Mat frame;
            frame.create(nHeight, nWidth, CV_8UC1);
            memcpy(frame.data, pImage, nWidth * nHeight);
            FrameObserver::DroneLocation loc = FindDrone(frame);

            if (loc.deltaIntensity == 0) {
                boost::array<double, 3> data;
                data[0] = loc.x;
                data[1] = loc.y;
                data[2] = loc.psi;
                udp.send(data);
            }
            // Could not detect the drone, tweak intensity and hope the next frame will be better
            else {
                exposure += loc.deltaIntensity;
                if (exposure < 1000) {
                    exposure = 1000;
                }
                else if (exposure > 10000) {
                    exposure = 10000;
                }
                std::cout << "Exposure: " << exposure << "\n";
                ApiController api;
                VmbErrorType res = api.SetExposureTimeAbs(m_pCamera, exposure);
                if(VmbErrorSuccess != res) {
                    std::cout << "=========== Error setting exposure ===========\n";
                }
            }

            cv::namedWindow("Image", CV_WINDOW_NORMAL);
            cv::setWindowProperty("Image", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
            cv::imshow("Image", frame);
            cv::waitKey(10);
        }
    }
    else
    {
        std::cout <<" frame pointer NULL\n";
    }

    m_pCamera->QueueFrame( pFrame );
}

}}} // namespace AVT::VmbAPI::Examples
