# @(#)55    1.2  src/htx/usr/lpp/htx/bin/hxecom/hxecom.runcleanup.sh, exer_com, htxubuntu 3/6/01 15:28:07

#get rid of any hxecom processes left over
data=`ps -ef | grep hxecom | awk '!/[runcleanup grep]/ {print $2}'`
for item in $data
do
    echo "Removing process " $item
    kill -9 $item  > /dev/null 2>&1
done

#clean up the shared memory keys and semaphores
data1=`ipcs | grep bbb | awk '{print $2}'`
for item1 in $data1
do
#	    echo "Removing hxecom key " $item1
		ipcrm -s $item1 > /dev/null 2>&1
		ipcrm -m $item1 > /dev/null 2>&1
done
#get rid of tty semaphores..
data1=`ipcs | grep eea | awk '{print $2}'`
for item1 in $data1
do
#	    echo "Removing tty key " $item1
		ipcrm -s $item1 > /dev/null 2>&1
done
