#!/bin/bash

# copy the actual firmare to .hi
sudo apt-get update -y && sudo apt-get upgrade -y
sudo apt install -y libssl-dev libsdl-image1.2-dev libevent-dev libsdl1.2-dev
sudo ./update_system.sh

echo "cd /home/pi/wash" >> /home/pi/run.sh
echo "./firmware.exe" >> /home/pi/run.sh

cd ..
make
mkdir /home/pi/wash
cp firmware.exe /home/pi/wash

cd /home/pi
chmod 777 /home/pi/run.sh
