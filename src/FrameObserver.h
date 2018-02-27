#ifndef AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER
#define AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER

#include "VimbaCPP/Include/VimbaCPP.h"

#include "UDPClient.h"
#include "DroneDetector.h"
#include <opencv2/imgproc.hpp>

using namespace cv;


class FrameObserver : virtual public AVT::VmbAPI::IFrameObserver
{
public:
    //
    // We pass the camera that will deliver the frames to the constructor
    //
    // Parameters:
    //  [in]    pCamera             The camera the frame was queued at
    //
    FrameObserver(AVT::VmbAPI::CameraPtr pCamera, unsigned int nrDrones, const std::string& ip, const std::string& port);
    
    //
    // This is our callback routine that will be executed on every received frame.
    // Triggered by the API.
    //
    // Parameters:
    //  [in]    pFrame          The frame returned from the API
    //
    virtual void FrameReceived( const AVT::VmbAPI::FramePtr pFrame );

private:
    UDPClient                   udp;
    DroneDetector		detector;
    boost::asio::io_service 	io_service;
    double 			exposure;
};

#endif
