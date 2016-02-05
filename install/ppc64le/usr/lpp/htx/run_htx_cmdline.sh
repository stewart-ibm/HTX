#!/bin/bash
# @(#)18	1.3  src/htx/usr/lpp/htx/run_htx_cmdline.sh, htxconf, htxubuntu 5/7/15 05:06:24

countdown()
{
	countdown=${1:-${test_duration}}
	w=${#countdown}
	while [ $countdown -gt 0 ]
	do
		sleep ${HTX_BUILD_TIMER_REFRESH_TIME} &
		printf "  %${w}d\r" "$countdown"
		countdown=$(( $countdown - ${HTX_BUILD_TIMER_REFRESH_TIME} ))
		wait
	done
} 2>/dev/null

test_htxd()
{
	echo "start the test with mdt file ${test_mdt} for duration ${TEST_DURATION} seconds..." >> ${log_file}
	
	echo "creating mdts: htxcmdline  -createmdt. This may take some time depending on size of system" >> ${log_file}
	htxcmdline  -createmdt >> ${log_file}
	
	# Now start run with selected MDT
	echo "Starting HTX exercisers with ${test_mdt} mdt file at `date`...." >> ${log_file}

	echo "htxcmdline  -run -ecg ${test_mdt}" >> ${log_file}
	htxcmdline  -run -ecg ${test_mdt} >> ${log_file}
	result=$?
	if [[ $result -eq 0 ]] ; then
			echo " command htxcmdline  -run -ecg ${test_mdt} completed successfully " >> ${log_file}
	else
			echo " command htxcmdline  -run -ecg ${test_mdt} failed with error ${result} " >> ${log_file}
			echo "TEST FAIL" >> ${log_file}
			exit ${result}
	fi

	# wait for provided amount of time
	echo "Waiting for ${TEST_DURATION} seconds ....." >> ${log_file}
	echo "remaining time in seconds:" >> ${log_file}
	test_duration=${TEST_DURATION}
	countdown

	# get running status of all the devices
	echo "running query command: htxcmdline  -query -ecg ${test_mdt} " >> ${log_file}
	htxcmdline  -query -ecg ${test_mdt} >> ${log_file}
	result=$?
	if [[ ${result} -eq 0 ]] ; then
			sleep 5
			echo " command htxcmdline  -query -ecg ${test_mdt} completed successfully " >> ${log_file}
	else
			echo " command htxcmdline  -query -ecg ${test_mdt} failed with error ${result} " >> ${log_file}
	fi

	# populate the HTX stats files
	echo "Populating /tmp/htxstats file now ..." >> ${log_file}
	echo "Proceeding with htxcmdline  -getstats -ecg ${test_mdt} " >> ${log_file}
	htxcmdline  -getstats -ecg ${test_mdt} >> ${log_file}
	result=$?
	if [[ ${result} -eq 0 ]] ; then
			echo "Generating stats file" >> ${log_file}
			sleep 5
			echo " command htxcmdline  -sut localhost -getstats -ecg ${test_mdt} completed successfully " >> ${log_file}
	else
			echo " command htxcmdline  -sut localhost -getstats -ecg ${test_mdt} failed with error ${result} " >> ${log_file}
	fi

	# After waiting, now stop the MDT
	echo "Stopping MDT ${test_mdt}" >> ${log_file}
	echo "Proceeding with htxcmdline  -shutdown -ecg ${test_mdt} " >> ${log_file}
	htxcmdline  -shutdown -ecg ${test_mdt} >> ${log_file}
	result=$?
	if [[ ${result} -eq 0 ]] ; then
			echo " command htxcmdline  -shutdown -ecg ${test_mdt} completed successfully " >> ${log_file}
	else
			echo " command htxcmdline  -shutdown  -ecg ${test_mdt} failed with error ${result} " >> ${log_file}
	fi
}
clear

if [[ $# -le 3 ]] ; then
	echo " Too few arguments"
	echo " argument 1: mdt file with full path name"
	echo " argument 2: HTX test duration - in secs"
	echo " argument 3: status check refresh interval - in secs"
	echo " argument 4: log file name with full path"
	exit 1
fi
test_mdt=$1
TEST_DURATION=$2
HTX_BUILD_TIMER_REFRESH_TIME=$3
log_file=$4
echo "starting HTX test with mdt: $1 on `hostname` at `date` for $2 seconds"
test_htxd
if [[ -s /tmp/htxerr ]] ; then
	echo "TEST FAIL" >> ${log_file}
	echo "TEST FAIL: HTX test done at `date`"
	exit 1
else
	echo "TEST PASS" >> ${log_file}
	echo "TEST PASS: HTX test done at `date`"
	exit 0
fi



