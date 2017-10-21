#include <string>
#include <cstring>
#include <iostream>

#include "VimbaCPP/Include/VimbaCPP.h"
#include "ApiController.h"

int main( int argc, char* argv[] )
{
    VmbErrorType err = VmbErrorSuccess;

    std::cout<<"/////////////////////////////////////////////\n";
    std::cout<<"/// Glow LED Ground robot / Drone tracker ///\n";
    std::cout<<"/////////////////////////////////////////////\n\n";


    AVT::VmbAPI::Examples::ApiController apiController;
    
    // Startup Vimba
    err = apiController.StartUp();        
    if ( VmbErrorSuccess == err )
    {

        AVT::VmbAPI::CameraPtrVector cameras = apiController.GetCameraList();
        if( cameras.empty() )
        {
            err = VmbErrorNotFound;
        }
        else
        {
            std::string strCameraID;
            err = cameras[0]->GetID( strCameraID );
            if( VmbErrorSuccess == err )
            {
                std::cout<<"Opening camera with ID: "<< strCameraID <<"\n";
                if(argc < 3) {
			std::cout << "Example usage: DroneTracker 192.168.0.100 6000\n";
			return 1;
		}
		std::string ip(argv[1]);
		std::string port(argv[2]);
                err = apiController.StartContinuousImageAcquisition( strCameraID, ip, port);
    
                if ( VmbErrorSuccess == err )
                {
                    std::cout<< "Press <enter> to stop acquisition...\n" ;
                    getchar();
    
                    apiController.StopContinuousImageAcquisition();
                }
            }
        }

        apiController.ShutDown();
    }

    if ( VmbErrorSuccess == err )
    {
        std::cout<<"\nAcquisition stopped.\n" ;
    }
    else
    {
        std::string strError = apiController.ErrorCodeToMessage( err );
        std::cout<<"\nAn error occurred: " << strError << "\n";
    }
}
