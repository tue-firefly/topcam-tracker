# topcam-tracker
Captures and processes a live camera feed to locate drones

## Building
topcam-tracker runs on Linux-based platforms. Run the provided `install.sh` script to install all necessary dependencies.
Supported distributions are:
- Arch linux
- Ubuntu Xenial Xerus (16.04 LTS)
- Ubuntu Artful Aardvark (17.10)
`install.sh` assumes you have both `curl` and `sudo` installed, install these first in the unlikely case you don't have them already.

## Testing
`dockerfiles/` contains a simple Dockerfile for each of the supported distributions, that will run `ìnstall.sh` and build the application in a container. Travis CI is set up to build these containers automatically after each commit. Before pushing you should run these tests yourself, i.e. run `docker build -f dockerfiles/<file> .` for each of the Dockerfiles and verify that they build succesfully.
