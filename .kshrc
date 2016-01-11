# @(#)42	1.2  src/htx/usr/lpp/htx/.kshrc, htxbringup, htxubuntu 6/4/04 14:08:30
#########################################################################
#       environment setup
        export LESS="-CdeiM"

#       term setup
function ttyis
{       echo $* > /.ttytype
        TERM="$*"
	export TERM
}

#       directory commands
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

#       screen management commands
        alias cls="tput clear"
        alias bye="tput clear; exit"
	alias win="open ksh; tput clear"

#       system management commands
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

#       miscellaneous commands
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

#       see if tty name starts with 'tty':
        case `tty` in
        *tty* | *pts*)  typeset PS1='${PSPF}${PWD}$PSCH ';;
        *)              typeset PS1='${PSPF}[1m${PWD}$PSCH[0m ';;
        esac


  ;;    # end -- options for interactive shells

esac

