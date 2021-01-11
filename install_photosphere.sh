#!/bin/bash

# installs photosphere code
# and configures the raspberry pi
# this code can be adapted for general
# linux installs (works on laptops alike)

# update the system
sudo apt-get -y update > /dev/null 2>&1

# install development libraries for libusb
# needed by libptp
sudo apt-get -y install libusb-dev
sudo apt-get -y install lftp

# add lftp settings (kicks session so it doesn't keep trying)
sudo echo "set net:timeout 5" >> /etc/lftp.conf
sudo echo "set net:max-retries 2" >> /etc/lftp.conf
sudo echo "set net:reconnect-interval-base 5" >> /etc/lftp.conf

# create ramdisk for the image stream
# this prevents writing to disk and potential
# corruption of the SD card on a raspberry pi
# [this feature needs to be disabled on a laptop - not required]
ramdisk=`cat /etc/fstab | grep data | wc -l`
if [[ ramdisk != "1" ]];then
	sudo mkdir /var/tmp
	echo "tmpfs /var/tmp tmpfs nodev,nosuid,size=100M 0 0" | sudo tee -a /etc/fstab
	sudo mount -a
fi

# build the libptp library and ptpcam program
# I included this in the github repository
cd ./libptp/

# don't update aclocal / Makefiles
touch aclocal.m4
touch Makefile.am
touch Makefile.in

# configure and make the tools
./configure && make

# system wide install
sudo make install

# run ldconfig to link libraries properly
sudo ldconfig

# setup crontab file to upload pictures every 15 min
echo "*/15 * * * * ~/photosphere/./capture_sphere.sh -n false -u true" >> mycron

#install new cron file
crontab mycron
rm mycron
