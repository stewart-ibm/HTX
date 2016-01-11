#!/bin/sh

#@(#)00        1.1     src/htx/usr/lpp/htx/etc/scripts/check_disk.sh, exer_hd, htxubuntu 7/8/14 23:49:26

# This script is called by hxehd exerciser before actually starting the
# test to check whether the device on which hxehd is going to run
# is available to be used by it. Thus, avoiding corrupting any disk
# accidentally.

disk_name=`echo $1 | tr -d '[0-9]'`
partition_num=`echo $1 | tr -cd [:digit:]`

#First of all check if disk has partitions and we are trying
#to run hxehd on disk itself not on its partition.
if [ "$partition_num" = "" ]
then
        partition_name=`basename $disk_name`
        num_partitions=`cat /proc/partitions | grep $partition_name[0-9]* | wc -l`
        if [ $num_partitions > 1 ]
        then
#               echo "$1 contains partitions, so can not be used by hxehd"
                exit 1
        fi
fi

check_partition=`parted -mls | grep $disk_name`
if [ "$check_partition" != "" ]
        for type in `parted $disk_name print | grep -E 'prep|boot|lvm|lba|swap|bios_grub|diag|legacy_boot' | awk '{print $1}'`
        do
                if [ "$partition_num"  != "" ]
                then
                        if [ "$type" -eq "$partition_num" ]
                        then
#                               echo "$1 can not be used by hxehd"
                                exit 1
                        fi
                else
#                       echo "$1 can not be used by hxehd"
                        exit 1
                fi
        done
fi

# check if any filesystem is present on the partition using mount command
mkdir /tmp/mnt_check 2>/dev/null
mount $1 /tmp/mnt_check
rc=$?
if test $rc = 0
then
#       echo "$1 has filesystem. So, can not be used by hxehd"
        umount /tmp/mnt_check
        rm -rf /tmp/mnt_check
        exit 1
else
        rm -rf /tmp/mnt_check
fi

exit 0
