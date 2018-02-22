#!/bin/bash -e

VIMBA_SDK_URL="https://www.alliedvision.com/fileadmin/content/software/software/Vimba/Vimba_v2.1.3_Linux.tgz"
VIMBA_SDK_PATH="Vimba_v2.1.3_Linux.tgz"
VIMBA_SDK_INSTALL_PATH=/opt

echo "============================================"
echo "Firefly topcam-tracker installation script"
echo "============================================"

# Detect distribution
if [ -f "/etc/arch-release" ]; then
	DISTRO="arch"
	echo "Detected distribution: Arch Linux"
elif [ -f "/etc/lsb-release" ]; then
	source /etc/lsb-release
	if [ "$DISTRIB_RELEASE" = "16.04" ]; then
		DISTRO="xenial"
		echo "Detected distribution: Xenial Xerus"
	else
		echo "Unsupported distribution!"
		exit 1
	fi
else
	echo "Unable to detect distribution: /etc/lsb-release not found!"
	exit 1	
fi

# Download SDK
if [ -f "$VIMBA_SDK_PATH" ]; then
	echo "Using existing Vimba SDK $VIMBA_SDK_PATH"
else
	echo "Downloading Vimba SDK"
	curl -O "$VIMBA_SDK_URL"
fi

# Install SDK
echo "Installing Vimba SDK to $VIMBA_SDK_INSTALL_PATH. You may be asked to enter your password now"
sudo tar -xf "$VIMBA_SDK_PATH" -C  "$VIMBA_SDK_INSTALL_PATH"
echo "Installing GigE Transport Layer"
# TODO Support x86 some day
sudo "$VIMBA_SDK_INSTALL_PATH/Vimba_2_1/VimbaGigETL/Install.sh"
echo "Creating a .env file with environment variables necessary for the transport layer."
echo "Either call 'source .env' every time before running the tracker, or put the contents in your bashrc"
DOTENV_PATH="$(dirname $0)/.env"
echo "export GENICAM_GENTL64_PATH=$VIMBA_SDK_INSTALL_PATH/Vimba_2_1/VimbaGigETL/CTI/x86_64bit" > "$DOTENV_PATH"
echo "export VIMBASDK_DIR=$VIMBA_SDK_INSTALL_PATH/Vimba_2_1" >> "$DOTENV_PATH"
echo "Created $(readlink -e $DOTENV_PATH)"

# Install dependencies
if [ "$DISTRO" = "arch" ]; then
	PACKAGES="opencv hdf5 boost gcc make pkg-config cowsay python"
	sudo pacman -Sy --noconfirm $PACKAGES || echo "Failed to install packages ($PACKAGES). Please install them manually"
elif [ "$DISTRO" = "xenial" ] || [ "$DISTRO" = "zesty"  ]; then
	sudo add-apt-repository -y ppa:timsc/opencv-3.3
	sudo apt-get update
	sudo apt-get install -y libopencv-dev libboost-all-dev build-essential g++ cowsay python3
fi

cowsay "Installation finished!" || echo "Installation finished!"
