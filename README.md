# HTX README

The Hardware Test Executive (HTX) is a suite of test tools for hardware
validation of OpenPOWER system.

The HTX stress tests the system by exercising all or selected hardware
components concurrently to uncover any hardware design flaws and
hardware-hardware or hardware-software interaction issues.

The HTX consists of test programs (a.k.a exercisers) to test processor, memory
and IO subsystem of POWER system.

The HTX runs on PowerKVM guests (Linux distributions like Ubuntu, SuSE and
Redhat), and OPAL-based distributions (like Ubuntu) and on Linux host machine.
HTX supports two types of control interfaces to exercisers, user interactive
and command line.

## Documentation
Please refer `Documentation` dir.

## Building HTX from source tree

Download the source from github.

Edit htx.mk file to point `TOPDIR` to downloaded HTX source.

Also set `HTX_RELEASE` variable to appropriate distro for which you are
building HTX source for. Possible values are:

 * htxubuntu
 * htxrhel72le
 * htxrhel7
 * htxsles12,
 * htxfedora
 * htxfedorale

Build instructions:  
  
In Debian/Ubuntu  
1) install git to download source code  
apt-get install git  
2) download HTX source code  
git clone https://www.github.com/open-power/HTX  
3) install other packages needed to compile HTX  
apt-get install gcc make libncurses5 g++ libdapl-dev  
&#160;&#160;  3a) for Ubuntu 14.04 there is no libdapl-dev package, so have to download and compile dapl separately  
&#160;&#160;  Can download latest dapl version (dapl-2.1.9.tar.gz) from https://www.openfabrics.org/downloads/dapl/  
&#160;&#160;  3b) install following packages needed to compile dapl  
&#160;&#160;  apt-get install libibverbs-dev librdmacm-dev  
&#160;&#160;  3c) cd into dapl directory and "./configure" "make" and then "make install"  
4) back in HTX directory do a "make all" and then after compiling do a "make deb" which will create a htxubuntu.deb package  
  
Note that HTX assumes you are building on a PowerPC distribution.  

Other targets:

 * To clean: make clean
 * To make rpm package: work in progress

## Install
Please refer HTX Users manual at `Documentation/HTX_user_manual.txt`
for details.

## License
Apache License, Version 2.0. For full details see LICENSE.

## Contributing
If you want to add your own test program as part of HTX. Please refer HTX
Programmer's manual at `Documentation/HTX_developers_manual.txt`.
