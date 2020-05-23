#!/bin/bash

sudo apt-get update -y && sudo apt-get upgrade -y

sudo apt install -y libsdl-image1.2-dev libevent-dev libsdl1.2-dev

sudo echo "@xset s off" >> /etc/xdg/lxsession/LXDE-pi/autostart
sudo echo "@xset -dpms" >> /etc/xdg/lxsession/LXDE-pi/autostart
sudo echo "@xset s noblank" >> /etc/xdg/lxsession/LXDE-pi/autostart
sudo echo "@/home/pi/run.sh" >> /etc/xdg/lxsession/LXDE-pi/autostart

sudo echo "@xset s off" >> /home/pi/.config/lxsession/LXDE-pi/autostart
sudo echo "@xset -dpms" >> /home/pi/.config/lxsession/LXDE-pi/autostart
sudo echo "@xset s noblank" >> /home/pi/.config/lxsession/LXDE-pi/autostart
sudo echo "@/home/pi/run.sh" >> /home/pi/.config/lxsession/LXDE-pi/autostart

sudo echo "hdmi_group=2" >> /boot/config.txt
sudo echo "hdmi_mode=14" >> /boot/config.txt


echo "cd /home/pi/SelfServiceWash/gpio_test" >> /home/pi/run.sh
echo "./get_and_build.sh &" >> /home/pi/run.sh
echo "sleep 1" >> /home/pi/run.sh
echo "./firmware.exe_" >> /home/pi/run.sh

cd /home/pi/SelfServiceWash/gpio_test
./build.sh
read -p "Enter Card Code : " card_code
echo "$card_code" >> id.txt
chmod 777 /home/pi/run.sh

