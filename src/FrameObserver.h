#ifndef AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER
#define AVT_VMBAPI_EXAMPLES_FRAMEOBSERVER

#include "VimbaCPP/Include/VimbaCPP.h"

#include "UDPClient.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {


class FrameObserver : virtual public IFrameObserver
{
public:
    //
    // We pass the camera that will deliver the frames to the constructor
    //
    // Parameters:
    //  [in]    pCamera             The camera the frame was queued at
    //
    FrameObserver(CameraPtr pCamera, const std::string& ip, const std::string& port);
    
    //
    // This is our callback routine that will be executed on every received frame.
    // Triggered by the API.
    //
    // Parameters:
    //  [in]    pFrame          The frame returned from the API
    //
    virtual void FrameReceived( const FramePtr pFrame );

private:
    UDPClient                   udp;
};

}}} // namespace AVT::VmbAPI::Examples

#endif
