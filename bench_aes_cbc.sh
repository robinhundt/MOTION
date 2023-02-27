#!/bin/bash

id=$1


for in_blocks in {1,2,8,16,32,64};do
  if [[ $id == 0 ]];then
    heaptrack="$HOME/heaptrack/build/bin/heaptrack -o heaptrack_MOTION_$in_blocks"
  fi
   $heaptrack ./build-release/bin/aescbc --my-id $id --parties 0,130.83.125.166,7745 1,130.83.125.167,7744 --data-bytes $(($in_blocks * 128))
done