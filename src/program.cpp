#include <string>
#include <stdlib.h>
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

    if(argc < 4) {
        std::cout << "Example usage: tracker 3 192.168.0.100 6000\n";
        return 1;
    }
    long n = strtol(argv[1], NULL, 10);
    if (n <= 0 || n > 10) {
        std::cout << "Number of drones should be between 1 and 10\n";
        return 1;
    }
    unsigned int nrDrones = n;
    std::cout << "Searching for " << nrDrones << " drones\n";
    std::string ip(argv[2]);
    std::string port(argv[3]);

    ApiController apiController;
    
    // Startup Vimba
    err = apiController.StartUp();        
    if ( VmbErrorSuccess == err )
    {

        AVT::VmbAPI::CameraPtrVector cameras = apiController.GetCameraList();
        if( cameras.empty())
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
                err = apiController.StartContinuousImageAcquisition( strCameraID, nrDrones, ip, port);
    
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
