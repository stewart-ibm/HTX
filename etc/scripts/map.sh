#!/bin/sh
# @(#)12        1.1  src/htx/usr/lpp/htx/etc/scripts/map.sh, libmisc, htxubuntu 8/24/06 17:38:30

rm /tmp/pmap.txt 2> /dev/null
num_proc=$(cat /proc/cpuinfo | grep processor |wc -l)
loop=1
p_num=0
while [ "$loop" -le "$num_proc" ]; do
        string="cat /proc/cpuinfo | grep processor | awk '(NR==$loop) { print \$3 }'"
        eval output='$('$string')'
        echo "Processor"$p_num" "$output >> /tmp/pmap.txt
        loop=$(($loop+1))
        p_num=$(($p_num+1))
done
