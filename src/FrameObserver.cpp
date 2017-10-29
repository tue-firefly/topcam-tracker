#include <iostream>
#include <iomanip>
#include <math.h>

#include <cstdio>
#include <ctime>

#include "FrameObserver.h"
#include "ApiController.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <memory>
#include <boost/asio.hpp>

using namespace cv;

namespace AVT {
namespace VmbAPI {
namespace Examples {


//
// We pass the camera that will deliver the frames to the constructor
//
// Parameters:
//  [in]    pCamera             The camera the frame was queued at
//
FrameObserver::FrameObserver( CameraPtr pCamera, unsigned int nrDrones, const std::string& ip, const std::string& port)
    :   IFrameObserver( pCamera )
    ,   udp(io_service, ip, port)
    ,   detector(nrDrones)
{
    exposure = 6000;
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
            int deltaExposure = 0;
            std::vector<DroneDetector::DroneState> states = detector.FindDrones(frame, &deltaExposure);

            for(unsigned int i = 0; i < states.size(); i++) {
                std::vector<boost::asio::const_buffer> buffers;
                unsigned int id = states[i].id;
                double x = states[i].pos.x;
                double y = states[i].pos.y;
                double psi = states[i].psi;
                buffers.push_back(boost::asio::buffer(&id, sizeof(id)));
                buffers.push_back(boost::asio::buffer(&x, sizeof(x)));
                buffers.push_back(boost::asio::buffer(&y, sizeof(y)));
                buffers.push_back(boost::asio::buffer(&psi, sizeof(psi)));
                udp.send(buffers);
                
            }
            // Could not detect the drone, tweak intensity and hope the next frame will be better
            if(deltaExposure != 0) {
                exposure += deltaExposure;
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
