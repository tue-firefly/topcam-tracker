# topcam-tracker [![Build Status](https://travis-ci.org/tue-firefly/topcam-tracker.svg?branch=master)](https://travis-ci.org/tue-firefly/topcam-tracker)
Captures and processes a live camera feed to locate drones

## Building
topcam-tracker runs on Linux-based platforms. Run the provided `install.sh` script to install all necessary dependencies.
Supported distributions are:
- Arch linux
- Ubuntu Xenial Xerus (16.04 LTS)
- Ubuntu Artful Aardvark (17.10)

`install.sh` assumes you have both `curl` and `sudo` installed, install these first in the unlikely case you don't have them already.

## Running
Simply run the generated application with sudo while preservering your enviroment variables, passing the ip and port to send the position data to: `sudo -E ./bin/tracker 127.0.0.1 4444`. The data is sent over UDP.

If you receive the error "Not found", first verify that you can open the camera using the provided Vimba Viewer in the SDK.
Then when you have succesfully done so and the application still does not start, please file an issue with steps to reproduce the error (list distribution, `install.sh` console output etc.)

## Testing
`dockerfiles/` contains a simple Dockerfile for each of the supported distributions, that will run `ìnstall.sh` and build the application in a container. Travis CI is set up to build these containers automatically after each commit. Before pushing you should run these tests yourself, i.e. run `docker build -f dockerfiles/<file> .` for each of the Dockerfiles and verify that they build succesfully.
