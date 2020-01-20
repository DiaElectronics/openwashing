echo "autologin"
sudo cp -f lightdm.conf /etc/lightdm/lightdm.conf
sudo cp -f firmware.desktop /etc/xdg/autostart/firmware.desktop
sudo echo "pi ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

echo "video_mode"
xrandr --newmode "848x480p60"   31.50  848 872 952 1056  480 483 493 500 -hsync +vsync
xrandr --addmode HDMI-1 "848x480p60"
xrandr --output HDMI-1 --mode "848x480p60"
