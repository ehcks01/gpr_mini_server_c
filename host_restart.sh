#!/bin/bash
path=$(dirname $(realpath $0))

sudo service hostapd restart
echo hospad restart
sleep 5
echo gpr server start
sudo $path/$1
