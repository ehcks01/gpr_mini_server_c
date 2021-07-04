#!/bin/bash
#arr_stop_target_list=$(ps -ef |grep "gpr_mini" |grep -v "grep" |awk '{print $2}')
#for target_stop in ${arr_stop_target_list};do
#        kill -9 $target_stop
#        echo kill process $target_stop
#done

path=$(dirname $(realpath $0))
sudo service hostapd restart
echo hospad restart
sleep 5
echo gpr server start
sudo $path/$1
