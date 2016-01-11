#!/bin/bash 
# This script generates the info about the environment
# on which HTX has been built, it stores the info on
# to the file /usr/lpp/htx/.htxsysinfo, which will
# be compared with the test machine's info
# Depending on the parameter this script will create files 
#
#e.g. "./htxinfo.sh build" will cause it to collect the 
#	   	build environment info and store it in a file
#	 				     while
#	  "./htxinfo" will cause it collect the machine info 
# 	   and compare it with .htxsysinfo         

if [[ -n "$1" ]]
then 
if [ $1 = "build" ]
then
	[ "$SANDBOXBASE" ] && [ "$CONTEXT" ] || {
		echo "Error : Not inside the sandbox";
		exit 1;
	}	 
	if [ "$CONTEXT" = "ppc64_linux_2" ] ;
	then 
 		CONT=ppc_linux_2;
		m_arch=ppc64
	else
 		CONT=$CONTEXT;
# We might be compiling a ppc release in 64 bit machine
		if [ "$CONTEXT" = "ppc_linux_2" ] ;
		then
			m_arch=ppc
		else
			m_arch=`arch`
		fi
	fi	
	gcc_path=$SANDBOXDIR/tools/$CONT/gcc/bin
	libc_path=$SANDBOXDIR/tools/$CONT/gcc/lib/

# if kernel version is defined then get it from home dir
# else pick it from backing build

	if [ -z $KERNEL_VERSION ]
	then
		[ $KERNEL_VERSION ] || { 
			kernel_headers=$SANDBOXDIR/linux/include/linux 
		}
	else
		kernel_headers=$SANDBOXBASE/linux/include/linux 
	fi
	KERNEL_VER=`cat $kernel_headers/version.h | grep UTS_RELEASE | cut -c 21- `
	htx_file=$SANDBOXBASE/src/htx/usr/lpp/htx/.htxsysinfo
fi
else  # on test machine 
	gcc_path=/usr/bin
	libc_path=/lib/
	KE_VER=`cat /proc/sys/kernel/osrelease`
	KERNEL_VER=\"$KE_VER\"	
	htx_file=/tmp/htxmachineinfo
	m_arch=`arch`
fi
# directly piping does not work with gcc

$gcc_path/gcc -v &> /tmp/ginfo

GCCINFO=`grep version /tmp/ginfo | cut -c  13-18`

echo "GCCVER=$GCCINFO" > $htx_file

GLIBCINFO=`strings $libc_path/libc.so.6 | grep "release version" | cut -c 38-42`

echo "GLIBVE=$GLIBCINFO" >> $htx_file

echo "MARCH =$m_arch" >> $htx_file

date_compiled=`date`
echo "DATE  =$date_compiled" >> $htx_file

if [ -n "$CMVC_RELEASE" ] ; then
echo "RELEAS=$CMVC_RELEASE" >> $htx_file
fi

echo "KERNVE=$KERNEL_VER" >> $htx_file

# Now time to check the compiled and current version 

HTX_I_PATH=/usr/lpp/htx/

[ "$1" != "build" ] &&
{
	BGCCV=`cat $HTX_I_PATH/.htxsysinfo | grep GCCV | cut -c 8-`
	BGLIBV=`cat $HTX_I_PATH/.htxsysinfo | grep GLIBVE | cut -c 8-`
	BKERNVE=`cat $HTX_I_PATH/.htxsysinfo | grep KERNVE | cut -c 8-`
	BMARCH=`cat $HTX_I_PATH/.htxsysinfo | grep MARCH | cut -c 8-`

	C_GCCV=`cat $htx_file | grep GCCV | cut -c 8-`
	C_GLIBV=`cat $htx_file | grep GLIBVE | cut -c 8-`
	C_KERNVE=`cat $htx_file | grep KERNVE | cut -c 8-`
	C_MARCH=`cat $htx_file | grep MARCH | cut -c 8-`

	if [ "$BGCCV" != "$C_GCCV" -o "$BGLIBV" != "$C_GLIBV" -o "$BKERNVE" != "$C_KERNVE" -o "$BMARCH" != "$C_MARCH" ] 
	then
	echo "************************************************************"
	echo "*                 WARNING !!!!!!                           *"
	echo "*                                                          *"
	echo "*  There is a mismatch between the release version         *"
	echo "*  of current OS and the version for which HTX is          *"
	echo "*  compiled.Some of the exercisers might not work !!       *"
	echo "*  Please check if u have picked the correct ver.          *"
	echo "*  of htx from                                             *"
	echo "*    http://w3.austin.ibm.com/:/projects/htx/public_html   *"
	echo "*                                                          *"
	echo "   CURRENT OS INFO ::                                      "
	echo "    GCC VERSION = $C_GCCV         "
	echo "   GLIB VERSION = $C_GLIBV        "
	echo " KERNEL VERSION = $C_KERNVE       "
	echo " ARCHITECTURE   = $C_MARCH        "
	echo "************************************************************"
	echo "   HTX RELEASE INFO::          "
	echo "    GCC VERSION = $BGCCV          "
	echo "   GLIB VERSION = $BGLIBV         "
	echo " KERNEL VERSION = $BKERNVE        "
	echo " ARCHITECTURE   = $BMARCH         "
	echo "                                                          "
	echo "************************************************************"
	fi
}
 
