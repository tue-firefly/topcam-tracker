#include <sstream>
#include <iostream>

#include "ApiController.h"

namespace AVT {
namespace VmbAPI {
namespace Examples {

#define NUM_FRAMES 3
#define DEFAULT_EXPOSURE 2500
#define PIXEL_FORMAT VmbPixelFormatMono8

ApiController::ApiController()
    // Get a reference to the Vimba singleton
    : m_system ( VimbaSystem::GetInstance() )
{}

ApiController::~ApiController()
{
}

//
// Translates Vimba error codes to readable error messages
//
// Parameters:
//  [in]    eErr        The error code to be converted to string
//
// Returns:
//  A descriptive string representation of the error code
//
std::string ApiController::ErrorCodeToMessage( VmbErrorType eErr ) const
{
    switch( eErr )
    {
        case VmbErrorSuccess:           return "Success.";
        case VmbErrorInternalFault:     return "Unexpected fault in VmbApi or driver.";
        case VmbErrorApiNotStarted:     return "API not started.";
        case VmbErrorNotFound:          return "Not found.";
        case VmbErrorBadHandle:         return "Invalid handle ";
        case VmbErrorDeviceNotOpen:     return "Device not open.";
        case VmbErrorInvalidAccess:     return "Invalid access.";
        case VmbErrorBadParameter:      return "Bad parameter.";
        case VmbErrorStructSize:        return "Wrong DLL version.";
        case VmbErrorMoreData:          return "More data returned than memory provided.";
        case VmbErrorWrongType:         return "Wrong type.";
        case VmbErrorInvalidValue:      return "Invalid value.";
        case VmbErrorTimeout:           return "Timeout.";
        case VmbErrorOther:             return "TL error.";
        case VmbErrorResources:         return "Resource not available.";
        case VmbErrorInvalidCall:       return "Invalid call.";
        case VmbErrorNoTL:              return "TL not loaded.";
        case VmbErrorNotImplemented:    return "Not implemented.";
        case VmbErrorNotSupported:      return "Not supported.";
        default:                        return "Unknown";
    }
}

//
// Starts the Vimba API and loads all transport layers
//
// Returns:
//  An API status code
//
VmbErrorType ApiController::StartUp()
{
    return m_system.Startup();
}

//
// Shuts down the API
//
void ApiController::ShutDown()
{
    // Release Vimba
    m_system.Shutdown();
}

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
VmbErrorType ApiController::StartContinuousImageAcquisition( const std::string& cameraID, unsigned int nrDrones, const std::string& ip, const std::string& port )
{
    // Open the desired camera by its ID
    VmbErrorType res = m_system.OpenCameraByID( cameraID.c_str(), VmbAccessModeFull, m_pCamera );
    if ( VmbErrorSuccess == res )
    {
        // Set the GeV packet size to the highest possible value
        // (In this example we do not test whether this cam actually is a GigE cam)
        FeaturePtr pCommandFeature;
        if ( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ))
        {
            if ( VmbErrorSuccess == pCommandFeature->RunCommand() )
            {
                bool bIsCommandDone = false;
                do
                {
                    if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
                    {
                        break;
                    }
                } while ( false == bIsCommandDone );
            }
        }

        if ( VmbErrorSuccess == res )
        {
            // set camera so that transform algorithms will never fail
            res = PrepareCamera();
            if ( VmbErrorSuccess == res )
            {
		std::cout << "port: " << port << "\n";
                // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
                m_pFrameObserver = new FrameObserver(m_pCamera, nrDrones, ip, port);
                // Start streaming
                res = m_pCamera->StartContinuousImageAcquisition( NUM_FRAMES, IFrameObserverPtr( m_pFrameObserver ));
            }
        }
        if ( VmbErrorSuccess != res )
        {
            // If anything fails after opening the camera we close it
            m_pCamera->Close();
        }
    }

    return res;
}

/**setting a feature to maximum value that is a multiple of 2*/
VmbErrorType SetIntFeatureValueModulo2( const CameraPtr &pCamera, const char* const& Name )
{
    VmbErrorType    result;
    FeaturePtr      feature;
    VmbInt64_t      value_min,value_max;
    result = SP_ACCESS( pCamera )->GetFeatureByName( Name, feature );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    result = SP_ACCESS( feature )->GetRange( value_min, value_max );
    if( VmbErrorSuccess != result )
    {
        return result;
    }
    value_max =( value_max>>1 )<<1;
    result = SP_ACCESS( feature )->SetValue ( value_max );
    return result;
}
/**set the exposure time of the camera*/
VmbErrorType ApiController::SetExposureTimeAbs(const CameraPtr &pCamera, float exposure) {
    VmbErrorType    result;
    FeaturePtr      feature;
    result = SP_ACCESS(pCamera)->GetFeatureByName("ExposureTimeAbs", feature);
    if(VmbErrorSuccess != result) {
        return result;
    }
    result = SP_ACCESS(feature)->SetValue(exposure);
    return result;
}

/**set the pixel format of the camera*/
VmbErrorType ApiController::SetPixelFormat(const CameraPtr &pCamera, VmbPixelFormatType format) {
    VmbErrorType    result;
    FeaturePtr      feature;
    result = SP_ACCESS(pCamera)->GetFeatureByName("PixelFormat", feature);
    if(VmbErrorSuccess != result) {
        return result;
    }
    result = SP_ACCESS(feature)->SetValue(format);
    return result;
}

/**prepare camera so that we can load the delivered image in OpenCV, and set a default exposure*/
VmbErrorType ApiController::PrepareCamera()
{
    
    VmbErrorType result;

    result = SetExposureTimeAbs(m_pCamera, DEFAULT_EXPOSURE);
    if( VmbErrorSuccess != result) {
        return result;
    }
    result = SetPixelFormat(m_pCamera, PIXEL_FORMAT);
    if( VmbErrorSuccess != result) {
        return result;
    }

    return result;
}

//
// Calls the API convenience function to stop image acquisition
// Closes the camera
//
// Returns:
//  An API status code
//
VmbErrorType ApiController::StopContinuousImageAcquisition()
{
    // Stop streaming
    m_pCamera->StopContinuousImageAcquisition();

    // Close camera
    return  m_pCamera->Close();
}

//
// Gets all cameras known to Vimba
//
// Returns:
//  A vector of camera shared pointers
//
CameraPtrVector ApiController::GetCameraList() const
{
    CameraPtrVector cameras;
    // Get all known cameras
    if ( VmbErrorSuccess == m_system.GetCameras( cameras ))
    {
        // And return them
        return cameras;
    }
    return CameraPtrVector();
}
}}} // namespace AVT::VmbAPI::Examples
