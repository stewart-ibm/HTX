#!/bin/bash

no_of_instances=`ls -d /sys/devices/system/node/node[0-9]* 2>/dev/null | sort -n -t : -k 1.30 | wc -l`
for node in $(ls -d /sys/devices/system/node/node[0-9]* 2>/dev/null | sort -n -t : -k 1.30)
do
	if [ -d $node ]; then
		cpulist=" "
		cpulist=`cat $node/cpulist`
		if [ -z "$cpulist" ];
		then
			no_of_instances=`expr $no_of_instances - 1`
		fi
	fi
done

echo "num_nodes=$no_of_instances" 

for node in $(ls -d /sys/devices/system/node/node[0-9]* 2>/dev/null | sort -n -t : -k 1.30)
do
	#printf "node=$node\n"
	if [ -d $node ]; then
		node_num=${node##*/node/node}
		cpulist=" "
		cpulist=`cat $node/cpulist`
#               printf "$cpulist,"
        	if [ -n "$cpulist" ];
        	then
            		i=0
            		k=0
            		cpus_in_node=0
            		for cpus in $(cat $node/cpulist | awk -F"," '{for(i=1;i<=NF;i++) print $i }')
            		do
            			#echo "cpu:$cpus"
            			j=0
            			for cpu_range in $(echo $cpus | awk -F"-" '{for(i=1;i<=NF;i++) print $i}')
            			do
            				#echo "cpus:$cpu_range"
            				cpu_no[$j]=$cpu_range
            				#echo "cpu_ran[$j]:${cpu_ran[$j]}"
            				j=`expr $j + 1`
            			done
            			if [ $j == 1 ];
            			then
            				no_cpus=$j
            			else
            				no_cpus=`expr ${cpu_no[1]} - ${cpu_no[0]} + 1`
            			fi
            			
            			for ((a=0; a < $no_cpus; a++))
            			do
            				lcpu[$k]=${cpu_no[0]}
            				cpu_no[0]=`expr ${cpu_no[0]} + 1`
            				#echo "lcpu:${lcpu[$k]}"
            				k=`expr $k + 1`
					cpus_in_node=`expr $cpus_in_node + 1`
            			done
            		done
            		mem_avail=`cat /sys/devices/system/node/node$node_num/meminfo | grep MemTotal | awk '{print $4}'`
            		mem_free=`cat /sys/devices/system/node/node$node_num/meminfo | grep MemFree | awk '{print $4}'`
			Hugepagesize=`cat /proc/meminfo | grep Hugepagesize | awk '{print $2}'`
			if [[ -z "$Hugepagesize" ]];
			then
				Hugepagesize=0
			fi
			HugePages_Total=`cat /sys/devices/system/node/node$node_num/meminfo | grep HugePages_Total | awk '{print $4}'`
			if [[ -z "$HugePages_Total" ]];
			then
				HugePages_Total=0
			fi
			HugePages_Free=`cat /sys/devices/system/node/node$node_num/meminfo | grep HugePages_Free | awk '{print $4}'`
			if [[ -z "$HugePages_Free" ]];
			then
				HugePages_Free=0
			fi
			printf "node_num=$node_num,mem_avail=$mem_avail,mem_free=$mem_free,cpus_in_node=$cpus_in_node,Hugepagesize=$Hugepagesize,HugePages_Total=$HugePages_Total,HugePages_Free=$HugePages_Free,cpus,"
            		for ((a=0; a < $k; a++))
            		do
            			printf ":%d" ${lcpu[$a]}
            		done
            		printf "\n" 
            		i=`expr $i + 1`
		fi
	fi
done
sync
