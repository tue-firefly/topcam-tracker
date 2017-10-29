#ifndef AVT_VMBAPI_EXAMPLES_APICONTROLLER
#define AVT_VMBAPI_EXAMPLES_APICONTROLLER

#include <string>

#include "VimbaCPP/Include/VimbaCPP.h"

#include "FrameObserver.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {

class ApiController
{
  public:
    ApiController();
    ~ApiController();

    //
    // Starts the Vimba API and loads all transport layers
    //
    // Returns:
    //  An API status code
    //
    VmbErrorType        StartUp();    
    
    //
    // Shuts down the API
    //
    void                ShutDown();
    
    //
    // Opens the given camera
    // Sets the maximum possible Ethernet packet size
    // Adjusts the image format
    // Sets up the observer that will be notified on every incoming frame
    // Calls the API convenience function to start image acquisition
    // Closes the camera in case of failure
    //
    // Parameters:
    //  [in]    Config      A configuration struct including the camera ID and other settings
    //
    // Returns:
    //  An API status code
    //
    VmbErrorType        StartContinuousImageAcquisition( const std::string &pCamera, unsigned int nrDrones, const std::string& ip, const std::string& port );    
    
    //
    // Calls the API convenience function to stop image acquisition
    // Closes the camera
    //
    // Returns:
    //  An API status code
    //
    VmbErrorType        StopContinuousImageAcquisition();

    //
    // Gets all cameras known to Vimba
    //
    // Returns:
    //  A vector of camera shared pointers
    //
    CameraPtrVector     GetCameraList() const;
    
    //
    // Translates Vimba error codes to readable error messages
    //
    // Parameters:
    //  [in]    eErr        The error code to be converted to string
    //
    // Returns:
    //  A descriptive string representation of the error code
    //
    std::string         ErrorCodeToMessage( VmbErrorType eErr ) const;

    //
    // Sets the exposure time of the given camera
    //
    // Parameters:
    // [in]   &pCamera    Pointer to the camera to set the exposure
    // [in]   exposure    The exposure to apply
    //
    // Returns:
    //  An API status code
    //
    VmbErrorType        SetExposureTimeAbs(const CameraPtr &pCamera, float exposure);

  private:
    VmbErrorType        PrepareCamera();
    VmbErrorType        SetPixelFormat(const CameraPtr &pCamera, VmbPixelFormatType format);
    VimbaSystem &       m_system;                   // A reference to our Vimba singleton
    CameraPtr           m_pCamera;                  // The currently streaming camera
    FrameObserver*      m_pFrameObserver;           // Every camera has its own frame observer
};

}}} // namespace AVT::VmbAPI::Examples

#endif
