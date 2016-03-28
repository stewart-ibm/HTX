#!/bin/bash

# Functions to print message in proper format

# This function prints message to both terminal and HTXNoise
# Takes 1 or 2 arguments:
#	1 argument : Just the string to be printed with date.
#	2 arguments: 1st arg is the string. 2nd arg is return code.
print_htx_log()
{
	if [[ -n "$1" && -n "$2" ]]
	then
		if [[ $2 -eq 0 ]]
		then
			echo "[`date`]: $1. return=$2" | tee -a $HTXNoise
		else
			echo "[`date`]: [ERROR($2)] $1 failed. return=$2" | tee -a $HTXNoise
		fi
	else
		echo "[`date`]: $1" | tee -a $HTXNoise
	fi
}


# Same as above function but printf to HTXNoise _only_
print_htx_log_only()
{
	if [[ -n "$1" && -n "$2" ]]
	then
		if [[ $2 -eq 0 ]]
		then
			echo "[`date`]: $1. return=$2" >> $HTXNoise
		else
			echo "[`date`]: [ERROR($2)] $1 failed. return=$2" >> $HTXNoise
		fi
	else
		echo "[`date`]: $1" >> $HTXNoise
	fi
}
	
function xe 
{
     SAVDISP=$DISPLAY
     DISPLAY=""; export DISPLAY
     efile=`tpath $* 2>/dev/null`
     if [ -z "$efile" ]
     then /usr/bin/e $*
     else /usr/bin/e $efile
     fi
     DISPLAY=$SAVDISP; export DISPLAY
}

function ez 
{
     if [ $# -ne 0 ]
     then arg1=$1
         [ -f "$arg1.lstZ" ] || { echo "$arg1.lstZ not found."; return [1]; }
     fi
     if [ ! -f "$arg1.lstZ" ]
     then arg1=
          while [ -z "$arg1" ]
          do echo "Enter filename without suffix:"
             read arg1
          done
          [ -f "$arg1.lstZ" ] || { echo "$arg1.lstZ not found."; return [1]; }
     fi
     zcat $arg1.lstZ > /tmp/$arg1.list; chmod 444 /tmp/$arg1.list
     e /tmp/$arg1.list; rm -f /tmp/$arg1.list
}

function pz
{    CCFLAG=
     for i in $*
     do [ "$i" != "-cc" ] || { CCFLAG=-cc; shift; continue; }
        zcat $i.lstZ > /tmp/$i.list
        cd /tmp; printlist $CCFLAG $i.list | print -nb -plot; cd - >/dev/null
        rm -rf /tmp/$i.list
     done
}

function pl
{
    printlist $* | print -nb -plot
}

function prt3800
{
    pr -l60 -f -n $* | /bin/print rpe
}

function t
{
    a=`pwd | grep com`
    b=`pwd | grep R2`
    if [ -z "$a" -a  -z "$b" ]
    then
        c=0;
    else
        if [ -n "$a" ]
        then
              cd com R2
        else
              cd R2 com
        fi
    fi
}

function ttyis
{     echo $* > /.ttytype
      TERM="$*"
      export TERM
}



# Export variables
  export USERNAME=htx
  export HOST=$(/bin/hostname -s)
  export PS1='
($?) $USERNAME @ $HOST: $PWD
# '
  export TERM=vt100
  export HOME=/usr/lpp/htx/
  export HTX_HOME_DIR=$HOME
  export HTX_LOG_DIR=/tmp/
  export LESS="-CdeiM"

  export HTXLPP=$HTX_HOME_DIR
  export HTXPATH=$HTX_HOME_DIR
  export HTXLINUXLEVEL=${HTXLPP}/htxlinuxlevel/
  export HTXPATTERNS=${HTXLPP}/pattern/
  export HTXRULES=${HTXLPP}/rules/
  export HTXREGRULES=${HTXRULES}/reg/
  export HTXECG=${HTXLPP}/ecg/
  export HTXMDT=${HTXLPP}/mdt/
  export HTXBIN=${HTXLPP}/bin/
  export HTXETC=${HTXLPP}/etc/
  export HTXSCREENS=${HTXLPP}/etc/screens/
  export STXSCREENS=${HTXLPP}/etc/screens_stx/
  export HTXMISC=${HTXLPP}/etc/misc/
  export HTXTMP=$HTX_LOG_DIR
  export HTXSETUP=${HTXLPP}/setup/
  export HTXRUNSETUP=${HTXLPP}/runsetup/
  export HTXRUNCLEANUP=${HTXLPP}/runcleanup/
  export HTXCLEANUP=${HTXLPP}/cleanup/
  export HTXPROCESSORS=1
  export HTXSCRIPTS=${HTXLPP}/etc/scripts/
  export HTXSCRIPTS_STX=${HTXLPP}/etc/scripts_stx/
  export HTXNoise=$HTX_LOG_DIR/HTXScreenOutput

# Directory commands
  alias ksh="bash"
  alias mdt="cd $HTXMDT"
  alias reg="cd $HTXREGRULES"
  alias emc="cd $HTXEMCRULES"
  alias rules="cd $HTXRULES"
  alias bin="cd $HTXBIN"
  alias tvi=/bos/k/bin/vi
  alias em="emacs =100x38+0+0"
  alias e="xe"
  alias li="/bin/li -v"
  alias liv="/bin/li -v vmm*"
  alias ll="/bin/li -lv"
  alias llv="/bin/li -lv vmm*"
  alias llx="/bin/li -lv xix*"

# Screen management commands
  alias cls="tput clear"
  alias bye="tput clear; exit"
  alias win="open ksh; tput clear"

# System management commands
  alias kmake=/bos/k/bin/make
  alias lml="lm list=list"
  alias print=/bin/print
  alias pq="/bin/print -q"
  alias pspg="ps -ef | pg"
  alias rb="remsh bdslab"
  alias pnum="rexec bcroom pnum"
  alias cnum="rexec bcroom cnum"
  alias dept="rexec bcroom dept"
  alias man="man -e/bin/pg"
  alias manv3="manv3 -e/bin/pg"
  alias nmake="nmake -u"
  alias s="echo 'sync;sync;sync;sync';sync;sync;sync;sync"

# Miscellaneous commands
  alias de="daemon emacs"
  alias dx="daemon xant"

# HTX commands
  set -o ignoreeof
  set -o vi
  alias logout=". ${HTXSCRIPTS}/htxlogout"
  alias exit=". ${HTXSCRIPTS}/htxlogout"
  alias htx=". runsup"
  alias stx="$HTXSCRIPTS_STX/runstx"
  alias stopstx="$HTXSCRIPTS_STX/stopstx"


# Check if this script is invoked by root/equivalent. 
# If not, then return with error
  if [[ $EUID -ne 0 ]]; then
    print_htx_log "Must be root to run HTX"
    exit 1
  fi


# HTXNoise is a repository for HTX screen activity during login and runsup commands.
# This is an attempt to capture errors that may otherwise be lost
  echo "[`date`]: 1st HTX message." | tee $HTXNoise

# Check whether HTX is already running on system
  [ "`ps -ef | grep hxssup | grep -v grep`" ] && {
     print_htx_log "HTX is already running..." 
     exit 1
  }

  PATH=$HOME
  PATH=$PATH:$HOME/etc/scripts
  PATH=$PATH:$HOME/etc
  PATH=$PATH:$HOME/etc/methods
  PATH=$PATH:$HOME/bin
  PATH=$PATH:/bin
  PATH=$PATH:/sbin
  PATH=$PATH:/usr/bin
  PATH=$PATH:/usr/sbin
  PATH=$PATH:/usr/ucb
  PATH=$PATH:/usr/bin/X11
  PATH=$PATH:/etc
  PATH=$PATH:/test/tools
  PATH=$PATH:/usr/local/bin
  PATH=$PATH:$HOME/test/tools
  PATH=$PATH:.
  export PATH
  print_htx_log "exporting PATH=$PATH"

  os_distribution=`grep ^NAME /etc/os-release | cut -f2 -d= | sed s/\"//g`
  if [ $os_distribution == "Ubuntu" ]
  then
    export HTX_RELEASE=htxubuntu
  else
    export HTX_RELEASE=`rpm -qa | grep ^htx | cut -f1 -d-`
  fi

  export CMVC_RELEASE=$HTX_RELEASE
  print_htx_log "exporting HTX_RELEASE=$HTX_RELEASE"

# All ulimit related updates
# Limit the stack to 8MB, if lesser as it's not done in some distros.
  ulimit -s 8192
  ret_val=$?
  print_htx_log "Setting ulimit -s(stack size) to 8MB" $ret_val
  
# Set core file limit to be unlimited.
  ulimit -c unlimited
  ret_val=$?  
  print_htx_log "Setting ulimit -c(core limit) to 'unlimited'" $ret_val

# Reset virtual mem ulimit, to unlimited.
  ulimit -v unlimited
  ret_val=$?
  print_htx_log "Setting ulimit -v(virtual memory)  to 'unlimited'" $ret_val

# Increase open files limit.
  num_proc=`cat /proc/cpuinfo  | grep -i processor | wc -l`
  open_files_limit=`expr $num_proc \* 4`
  if [[ $num_proc -lt 1024 ]]
  then
      open_files_limit=1024
  fi

  ulimit -n $open_files_limit
  ret_val=$?
  print_htx_log "Setting ulimit -n(# open files) to '$open_files_limit'" $ret_val

# Creating a link for /bin/ksh
  if [ ! -f /bin/ksh ]
  then
    if [ -f /usr/bin/ksh ]
      then
        ln -s /usr/bin/ksh /bin/ksh
      else
        ln -s /bin/bash /bin/ksh
      fi
  fi

# Set core files to use pid.
  echo 1 >/proc/sys/kernel/core_uses_pid
  

# Source global definitions
  [ -f /etc/bashrc ] && . /etc/bashrc

# Write to /proc/sys/kernel/sem to allow more semop operations.This will overwrite default values
# [ -f /proc/sys/kernel/sem ] && echo "550 64000 500 1024" >/proc/sys/kernel/sem
# [ -f /proc/sys/kernel/sem ] && echo "250 32000 32 1024" >/proc/sys/kernel/sem

#sleep 5

# Execute HTX setup scripts 
  [ -f /usr/lpp/htx/etc/scripts/htxtmpwatch ] && cp /usr/lpp/htx/etc/scripts/htxtmpwatch /etc/cron.d/htxtmpwatch 2>/dev/null


  if [[ "$HTXD_STARTUP_MODE" = "ON" ]] ; then
    print_htx_log "HTXD_STARTUP_MODE was '$HTXD_STARTUP_MODE'. Turning it OFF and returning."
    HTXD_STARTUP_MODE=OFF
    return
  fi

  if [ -f ${HTXLPP}.autostart ] && [ -f $HTX_LOG_PATH/mpath_parts ] ; then
    if [ `cat $HTX_LOG_PATH/mpath_parts | wc -l` != 0 ] ; then
    htx_print_log "sleeping 120 seconds for bootme with multipath"
    sleep 120
    fi
  fi

  # To execute post boot commands for bootme
	if [ -f ${HTXLPP}.autostart ] ; then
		BOOT_POST=$(grep "^BOOT_POST:" ${HTXREGRULES}bootme/default | head -n1 | awk '{ print $2 }')
		if [ -f ${BOOT_POST} ];
		then
			print_htx_log "Calling ${BOOT_POST}"
			. ${BOOT_POST}
		fi
	fi


	if [ `ls ${HTXBIN}/hxenvidia_* 2>/dev/null | wc -l` != 0 ] ; then
		if [ `grep "8.0" /usr/local/cuda/version.txt 2>/dev/null | wc -l` == 1 ] ; then	
			ln -sf ${HTXBIN}/hxenvidia_8_0 ${HTXBIN}/hxenvidia
			print_htx_log "making hxenvidia_8_0 as default."
		elif [ `grep "7.5" /usr/local/cuda/version.txt 2>/dev/null | wc -l` == 1 ] ; then
			ln -sf ${HTXBIN}/hxenvidia_7_5 ${HTXBIN}/hxenvidia
			print_htx_log "making hxenvidia_7_5 as default."
		fi
 	fi 
	   
  
  print_htx_log "Collecting LPAR configuration details ..."
  ${HTXBIN}/show_syscfg > ${HTX_LOG_DIR}/htx_syscfg 2>/dev/null
  
  for file in `/bin/ls $HTXSETUP[a-zA-Z]*.setup 2>/dev/null| sort | grep -v mem.setup | grep -v "htx\.setup"`
  do
    print_htx_log "Running $file"
    . $file  | tee -a $HTXNoise
  done

  # Now run mem.setup
  print_htx_log "Running mem.setup"
  . ${HTXSETUP}/mem.setup | tee -a $HTXNoise

  print_htx_log "Calling devconfig"
  ${HTXSCRIPTS}/devconfig | tee -a $HTXNoise
  
  [ ! -f ${HTXMDT}/mdt ] && cp ${HTXMDT}/mdt.all ${HTXMDT}/mdt
  
  cat /tmp/devlist_warning 2>/dev/null

# Work-around for autostartup problems
  if [ -f ${HTXLPP}.autostart ];
  then
     print_htx_log "Autostart is ON. Sleeping for 20 seconds and then calling runsup."
     sleep 20
     ${HTXSCRIPTS}runsup
  fi

# Export ZLIB variables to start testing corsa hardware.
  print_htx_log "exporting ZLIB_DEFLATE_IMPL=1, ZLIB_INFLATE_IMPL=1, ZLIB_CARD=0, ZLIB_INFLATE_THRESHOLD=0"
  export ZLIB_DEFLATE_IMPL=1; export ZLIB_INFLATE_IMPL=1; export ZLIB_CARD=0; export ZLIB_INFLATE_THRESHOLD=0;

# Below logic is to increase cgroup max thread limit.
# If not done, this will cause failure of multiple exercisers
# as the default is 512 only starting ubuntu 16.04. 

print_htx_log "setting proper systemd resource limits..."
A=`echo /sys/fs/cgroup/pids$(awk -F : '/pids/{print $NF}' /proc/$$/cgroup) 2> /dev/null` 
B=$A

# Set pids.max for all levels below us.
while :; do
	test -f $B/pids.max || break
	echo max > $B/pids.max
	if [ $B ==  "/sys/fs/cgroup/pids" ]; then
		break;
	fi
	B=$(dirname $B)
done

# Set pids.max for all levels above us.

for i in $(find $A -name "pids.max" 2> /dev/null); do
	test -f $i || break
	echo max > $i
done
