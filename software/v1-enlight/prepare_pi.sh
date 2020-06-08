#!/bin/bash

sudo apt-get update -y && sudo apt-get upgrade -y

sudo apt install -y libssl-dev libjansson-dev libsdl-image1.2-dev libevent-dev
sudo apt install -y libsdl1.2-dev libcurl4-openssl-dev krb5-config
sudo apt install -y libidn2-0-dev libssh2-1-dev  libreadline-dev
sudo apt install -y librtmp-dev libssh-dev libnghttp2-dev  libpsl-dev libldap2-dev

cd 3rd/lua53/src && make linux
