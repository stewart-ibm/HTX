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

Run `make all` to build.

Note that HTX assumes you are building on a PowerPC distribution.

Other targets:

 * To make deb package: make deb
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
