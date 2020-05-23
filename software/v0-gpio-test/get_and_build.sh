#!/bin/bash

cd /home/pi/SelfServiceWash/gpio_test

if [ -f ./firmware.exe ]; then
  echo "copying firmware..."
  cp firmware.exe firmware.exe_
fi

./build_updated.sh &
