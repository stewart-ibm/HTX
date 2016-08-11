#!/bin/bash

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

  PATH=$PATH:$HOME
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

  export HTXLPP=$HTX_HOME_DIR
  export HTXPATH=$HTX_HOME_DIR
  export HTXLINUXLEVEL=${HTXLPP}/htxlinuxlevel # Its a file. Last / should not be there.
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
  export HTXNoise=$HTX_LOG_DIR/HTXScreenOutput # Its a file. Last / should not be there

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

