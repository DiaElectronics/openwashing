#!/bin/bash
xrandr --newmode "848x480p60"   31.50  848 872 952 1056  480 483 493 500 -hsync +vsync
xrandr --addmode HDMI-1 "848x480p60"
xrandr --output HDMI-1 --mode "848x480p60"
cd /home/pi/opi
sleep 10
sudo ./firmware.exe
