#!/bin/bash


echo "THIS SCRIPT IS FOR RASPBIAN ONLY! DO NOT USE ON OTHER LINUX"
sleep 5

cd ~
git clone https://github.com/OpenRbt/openwashing
cd openwashing/software/v1-enlight
./prepare_pi.sh


mkdir ~/wash
cp firmware.exe ~/wash/firmware.exe
mkdir ~/wash/firmware
cp openwashing/software/v1-enlight/samples/wash ~/wash/firmware
cp install/prepare_pi.sh ~/wash/
cp install/update_system.sh ~/wash/
cd ~/wash
./preapare_pi.sh

