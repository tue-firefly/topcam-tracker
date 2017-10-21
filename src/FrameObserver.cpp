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

#define IP "192.168.12.102"
#define PORT "6000"
#define INF 99999999


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

            std::vector<std::vector<Point> > contours;
            std::vector<Vec4i> hierarchy;
            Mat thresh;
            
            threshold(frame, thresh, 127, 255, CV_THRESH_BINARY);
            findContours(thresh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
            std::cout << "Found " << contours.size() << " contours\n";
            if (contours.size() %4 == 0) {
		int i,j ;
		std::vector<Point2f> allLeds(contours.size());
                float xSum = 0;
                float ySum = 0;
                for( i = 0; i < contours.size(); i++ ) {
                    float radius;
                    minEnclosingCircle((Mat)contours[i], allLeds[i], radius);
                    circle(frame, allLeds[i], radius, Scalar(255, 0, 0), 2, 8, 0);
                    xSum += allLeds[i].x;
                    ySum += allLeds[i].y;
                }		
		int NROF_DRONES = contours.size()/4;   
		
		int *contourMapping = (int *)calloc(contours.size(),sizeof(int));
		for(i = 0; i < contours.size(); i++){
			contourMapping[i] = INF; 
		}
		int *distance = (int *)malloc(sizeof(int)*contours.size());
		int minDistance[3] = {INF};
		int minDistancePoints[3] = {-1};
		for(i = 0; i < NROF_DRONES; i++){
			int mainpoint = 0;
			for(j = 0; j < contours.size(); j++){
				if(contourMapping[j] == INF){
					mainpoint = j;
					break;
				}
			}
			for(j = 0; j < contours.size(); j++){
				distance[j] = INF;
			}
			contourMapping[mainpoint] = i;
			for(j = 0; j < contours.size(); j++){
				if(j != mainpoint && contourMapping[j] == INF){
					distance[j] = sqrt(pow(allLeds[mainpoint].x-allLeds[j].x,2)+pow(allLeds[mainpoint].y+allLeds[j].y,2));
					if(distance[j] < minDistance[0] || distance[j] < minDistance[1] || distance[j] < minDistance[2]){
						if(minDistance[0] >= minDistance[1] && minDistance[0] >= minDistance[2]){
							minDistance[0] = distance[j];
							minDistancePoints[0] = j;
						}else if(minDistance[1] >= minDistance[0] && minDistance[1] >= minDistance[2]){
							minDistance[1] = distance[j];
							minDistancePoints[1] = j;
						}else{
							minDistance[2] = distance[j];
							minDistancePoints[2] = j;
						}
					}
				}
			}
			//Debug print
			printf("Printing distance list\n");
			for(j = 0; j < contours.size(); j++){
				printf("%d -> %d\n",j,distance[j]);

			}

			for(j = 0; j < 3; j++){
				contourMapping[minDistancePoints[j]] = i;
				printf("%d is mapped to drone %d \n",minDistancePoints[j], i); 
			}
		}


		int dronenr;
		for(dronenr = 0; dronenr < NROF_DRONES; dronenr++){
			printf("drone %d is being found\n",dronenr);
                	std::vector<Point2f> leds(4);
			int lednr = 0;
			for(int i = 0; i < contours.size(); i++){
				if(contourMapping[i] == dronenr){
					leds[lednr] = allLeds[i];
					printf("Led %d is from drone %d \n", i, dronenr);
					lednr++;
				}
			}
                	float xSum = 0;
                	float ySum = 0;
                	for( int i = 0; i < 4; i++ ) {
			    float radius;
			    minEnclosingCircle((Mat)contours[i], leds[i], radius);
			    circle(frame, leds[i], radius, Scalar(255, 0, 0), 2, 8, 0);
			    xSum += leds[i].x;
			    ySum += leds[i].y;
			}
			double xAvg = xSum / contours.size();
			double yAvg = ySum / contours.size();
			Point2f avg(xAvg, yAvg);
	    
			std::vector<Point2f> outerLeds(2);
			double max1 = 0;
			double max2 = 0;
			for(int i = 0; i < leds.size(); i++) {
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

			std::cout << "robot: " << dronenr << ", x: " << physicalCenter.x << ", y: " << physicalCenter.y << "psi: " << (psi / M_PI * 180) << "\n";
			boost::array<double, 3> data;
			// According to conventions
			data[0] = -physicalCenter.y;
			data[1] = physicalCenter.x;
			data[2] = psi;
			udp.send(data);
		}
		free(contourMapping);
		free(distance);
	    } else {
		if(contours.size() % 4 > 1) {
		    // Exposure higher
		    exposure += 100;
		    if (exposure < 1000) exposure = 10000;
		} 	
		else {
		    // Exposure lower
		    exposure -= 100;
		    if (exposure > 10000) exposure = 1000;
		}
		std::cout << "Exposure: " << exposure << "\n";
		ApiController api;
		VmbErrorType res = api.SetExposureTimeAbs(m_pCamera, exposure);
		if(VmbErrorSuccess != res) {
		    std::cout << "=========== Error setting exposure ===========\n";
		}
	    }
		    

	 //rectangle(frame, roi, Scalar(255, 0, 0), 2, 1);
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
