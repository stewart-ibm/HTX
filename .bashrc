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

# User specific aliases and functions

# Setup term
  export TERM=vt100

# Source global definitions
  if [ -f /etc/bashrc ]; then
      . /etc/bashrc
  fi

# Environment setup
        export LESS="-CdeiM"

# Term setup
  function ttyis
  {     echo $* > /.ttytype
        TERM="$*"
	export TERM
  }
  
# Directory commands
  alias ksh="bash"
  alias mdt="cd /usr/lpp/htx/mdt"
  alias reg="cd /usr/lpp/htx/rules/reg"
  alias emc="cd /usr/lpp/htx/rules/emc"
  alias tvi=/bos/k/bin/vi
  alias em="emacs =100x38+0+0"
  alias e="xe"
  alias li="/bin/li -v"
  alias liv="/bin/li -v vmm*"
  alias ll="/bin/li -lv"
  alias llv="/bin/li -lv vmm*"
  alias llx="/bin/li -lv xix*"

# Build commands
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

  case $- in
  *i*)  # Options for interactive shells

      myid=`id | sed -n -e 's/).*$//' -e 's/^.*(//p'`
      if [ $myid = root ]
      then    typeset -x PSCH='#'
      else    typeset -x PSCH='%'
      fi

      typeset PSPF="[htx@$HOST] "

# See if tty name starts with 'tty':
      case `tty` in
      *tty* | *pts*)  typeset PS1='${PSPF}${PWD}$PSCH ';;
      *)              typeset PS1='${PSPF} [1m${PWD}$PSCH [0m ';;
      esac

  ;;    # end -- options for interactive shells
  esac

  function xe
  {    SAVDISP=$DISPLAY
       DISPLAY=""; export DISPLAY
       efile=`tpath $* 2>/dev/null`
       if [ -z "$efile" ]
       then /usr/bin/e $*
       else /usr/bin/e $efile
       fi
       DISPLAY=$SAVDISP; export DISPLAY
  }

  function ez
  {    if [ $# -ne 0 ]
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
  {     printlist $* | print -nb -plot
  }

  function prt3800
  {     pr -l60 -f -n $* | /bin/print rpe
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
