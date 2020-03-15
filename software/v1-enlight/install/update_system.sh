#!/bin/bash

echo "@xset s off" >> /etc/xdg/lxsession/LXDE-pi/autostart
echo "@xset -dpms" >> /etc/xdg/lxsession/LXDE-pi/autostart
echo "@xset s noblank" >> /etc/xdg/lxsession/LXDE-pi/autostart
echo "@/home/pi/run.sh" >> /etc/xdg/lxsession/LXDE-pi/autostart

echo "hdmi_group=2" >> /boot/config.txt
echo "hdmi_mode=14" >> /boot/config.txt
