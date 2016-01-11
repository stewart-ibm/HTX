num_nodes=$(ls -d /sys/devices/system/node/node[0-9]* 2>/dev/null | wc -l ) 
printf "num_nodes=%d\n" $num_nodes

for node in $(ls -d /sys/devices/system/node/node[0-9]* 2>/dev/null | sort -n -t : -k 1.30)
do
        #echo "node "$node
        if [ -d $node ]; then
				node_num=${node##*/node/node}
				#cpus_in_node=$(ls -d $node/cpu[0-9]* 2>/dev/null | wc -l )
				cpus_in_node=$(find $node/ -name cpu[0-9]*  | awk ' { printf("grep -H 1 %s/online\n",$1) } ' | sh | wc -l )
                printf "node=%d,cpus_in_node=%d,cpus" ${node##*/node/node} $cpus_in_node
        fi
		str_len=$((${#node}+5))
       # for cpu in $(ls -d $node/cpu[0-9]* 2>/dev/null | sort -n -t : -k 1.$str_len )
	for cpu in $(find $node/ -name cpu[0-9]*  | awk ' { printf("grep -H 1 %s/online\n",$1) } ' | sh | cut -d / -f 7 | awk -F 'cpu' ' {print $2} ' )
        do
        #        if [ -d $cpu ]; then
        #                #echo "cpu "$cpu
        #               printf ":%d" ${cpu##*cpu}
        #      fi
	 	echo -n ":$cpu"
        done
        printf "\n"
done
