#!/bin/bash
#
#
# @(#)50 1.4  src/htx/usr/lpp/htx/etc/scripts/htx.d, htxd, htxubuntu 11/17/14 06:19:41
################################################################
# htx.d script is to enable htxd service
#
# System startup script for htx deamon
#
### BEGIN INIT INFO
# Provides: htxd
# Required-Start:
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start htx daemon
# Description:       Start htx daemon
### END INIT INFO
#
# chkconfig: 2345 20 80
# description: Runs htxd startup script htxd_run 
# processname: htxd 
#


prog=htxd


start() 
{
	return_code=$(rh_status)
	if [[ ${return_code} = 0 ]] ; then
        	echo "htx daemon has been already running..." 
		exit 1 
	fi
	echo "Starting $prog: ..."
	/usr/lpp/htx/etc/scripts/htxd_run > /tmp/htxd_starup_log 2>&1	
}

stop() 
{
	return_code=$(rh_status)
	if [[ ${return_code} != 0 ]] ; then
		echo "htx daemon was already stopped..."
		exit 0
	fi
	echo "Stopping $prog: ..."
	/usr/lpp/htx/etc/scripts/htxd_shutdown > /tmp/htxd_starup_log 2>&1
}

restart() 
{
	stop
	start
}

reload() 
{
	restart
}

rh_status() 
{
    # run checks to determine if the service is running or use generic status
	pgrep htxd > /dev/null 2>&1
	return_code=$?
	echo ${return_code}
}

rh_status_q() 
{
    rh_status >/dev/null 2>&1
}


### program starts here ###

case "$1" in
    start)
        $1
        ;;
    stop)
        $1
        ;;
    restart)
        $1
        ;;
    reload)
        $1
        ;;
    status)
        return_code=$(rh_status)
	if [[ ${return_code} = 0 ]] ; then
		echo "htx daemon is running"
	else
		echo "htx daemon is stopped"
	fi
        ;;
    *)
        echo $"Usage: $0 {start|stop|status|restart|reload}"
        exit 2
esac

exit 0