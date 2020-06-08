echo "installing gui"
sudo apt-get -y install libxfce4util-common
sudo apt-get autoremove
sudo apt-get -y install xorg lightdm xfce4 tango-icon-theme gnome-icon-theme

echo "installing libs"
sudo apt install -y libsdl-image1.2-dev libevent-dev libsdl1.2-dev
sudo apt install -y redis-server libssl-dev libjansson-dev libsdl-image1.2-dev libevent-dev libsdl1.2-dev libcurl4-openssl-dev libreadline-dev

echo "installing wiring pi"
git clone https://github.com/zhaolei/WiringOP.git -b h3
cd WiringOP
chmod +x ./build
sudo ./build
