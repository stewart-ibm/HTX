# IBM_PROLOG_BEGIN_TAG
# 
# Copyright 2003,2016 IBM International Business Machines Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# 		 http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# IBM_PROLOG_END_TAG
# Check whether HTX is already running on system
  [ "`ps -ef | grep hxssup | grep -v grep`" ] && {
     echo "HTX is already running..."
     exit 1
  }

# Export variables
  export USERNAME=htx
  export HOST=$(/bin/hostname -s)
  export PS1='
($?) $USERNAME @ $HOST: $PWD
# '
  export TERM=vt100
  export HOME=/usr/lpp/htx

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

  [ -f /usr/lpp/htx/etc/version ] || export CMVC_RELEASE=$(rpm -qi $(rpm -qa 2>/dev/null | grep ^htx) 2>/dev/null | grep Name | head -n1 | awk '{print $3}')
  [ -f /usr/lpp/htx/etc/version ] && export CMVC_RELEASE=`cat /usr/lpp/htx/etc/version | cut -d- -f1`

# Limit the stack to 8MB, if lesser as it's not done in some distros.
  ulimit -s 8192
  
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

# Set core file limit to be unlimited.
  ulimit -c unlimited

# Set core files to use pid.
  echo 1 >/proc/sys/kernel/core_uses_pid
  
  set -o vi

# HTXNoise is a repository for HTX screen activity during login and runsup commands.
# This is an attempt to capture errors that may otherwise be lost
  export HTXNoise=/tmp/HTXScreenOutput
  >$HTXNoise

# Print IBM Copyright message before doing anything.
  echo "
##############################################################################

                    Licensed Materials - Property of IBM
          (C) Copyright IBM Corp. 2010, 2013,  All rights reserved.

         US Government Users Restricted Rights - Use, duplication or
       disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

############################################################################## " | tee -a $HTXNoise

  export BASH_ENV=~/.bashrc
  [ -f ~/.bashrc ] && . ~/.bashrc
# Write to /proc/sys/kernel/sem to allow more semop operations.This will overwrite default values
# [ -f /proc/sys/kernel/sem ] && echo "550 64000 500 1024" >/proc/sys/kernel/sem

#htx Notice

echo "
###############################################################################

  HTX is the valuable proprietary product of the IBM Corporation. HTX can not
  be distributed external to the IBM Corporation without first negotiating a
  Confidentiality Disclosure Agreement (CDA) with specific consideration for
  HTX and, if applies, conforms to all export regulations and guidelines.

  Contact  Sameer Mulla (sameer.mulla@in.ibm.com) for external distribution requests

###############################################################################

" | tee -a $HTXNoise
sleep 5

# Execute HTX setup scripts 
  [ -f ~/.htxrc ] && . ~/.htxrc
  [ -f /.alias ] && . /.alias

  [ -d ~/.terminfo ] || ln -sf /usr/share/terminfo ~/.terminfo
#  [ -f /proc/sys/kernel/sem ] && echo "250 32000 32 1024" >/proc/sys/kernel/sem
  [ -f /usr/lpp/htx/etc/scripts/htxtmpwatch ] && cp /usr/lpp/htx/etc/scripts/htxtmpwatch /etc/cron.d/htxtmpwatch 2>/dev/null
