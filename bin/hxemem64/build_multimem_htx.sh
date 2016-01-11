#!/bin/sh
#--------------------------------------------------
#Setup the ECG file
#--------------------------------------------------
#Use base mdt file that htx builds
basefile=/usr/lpp/htx/mdt/mdt.all

#Use temp mdt file with collapsed paragraphs 
tempfile=/tmp/ecgpara.bu

#Collapse the paragraphs of mdt file 
awk '/./ { printf "%s\\", $0 } /^$/ { printf "\\\n" } END { printf "\\\n" } ' $* ${basefile} > ${tempfile}

#Start building the custom file with the same default settings
grep "default:" ${tempfile} > /tmp/mdtpara.smp

#Pull the base mem stanza and update for the machine config
grep "mem:" ${tempfile} > /tmp/mempara.stanza

#Change the stanza to point to memmfg, append mem para to the custom file
sed s/maxmem/memmfg/g /tmp/mempara.stanza >> /tmp/mdtpara.smp

#Uncollapse the stanza file
awk ' { gsub("\\\\", "\n", $0); printf "%s", $0 }' $* /tmp/mdtpara.smp > /tmp/mdt.smp1

#Move tmp file to mdt.smp
mv /tmp/mdt.smp1 /tmp/mdtpara.smp

#--------------------------------------------------
#Setup the RULES file to use all memory
#--------------------------------------------------
#Use base maxmem rules file
baserules=/usr/lpp/htx/rules/reg/hxemem64/maxmem

#Determine maximum memory
pages_avail=` cat /proc/meminfo | grep MemFree | awk '{print $2}'`
mem_avail=`expr $pages_avail \* 1024`

#Find out how many processors are available
proc_count=`cat /proc/cpuinfo | grep processor | wc -l`

cpu_limit=$proc_count

#Set 256MB segments
reg_seg_size=`expr 1024 \* 1024 \* 256`

seg_size=`expr $mem_avail / $cpu_limit`

if [[ $seg_size -gt $reg_seg_size ]]; then
seg_size=$reg_seg_size
fi

#Make the segment size multiple of 8
seg_size=$(((seg_size/8)*8))

#Determine number of segments: memory / seg size and num cpus
num_segs=`expr $mem_avail / $seg_size`
segs_cpu=`expr $num_segs / $cpu_limit`

#Minimum segments per CPU should be 1
if [[ $segs_cpu -eq 0 ]] ; then
segs_cpu=1
fi

sed -e '/./{H;$!d;}' -e 'x;/mem_0x00/!d;' $baserules > /tmp/rulefile
echo num_seg_4k = $segs_cpu >> /tmp/rulefile
echo seg_size_4k = $seg_size >> /tmp/rulefile

sed -e '/./{H;$!d;}' -e 'x;/mem_0xFF/!d;' $baserules >> /tmp/rulefile
echo num_seg_4k = $segs_cpu >> /tmp/rulefile
echo seg_size_4k = $seg_size >> /tmp/rulefile

sed s/"max_mem = .*"/"max_mem = no"/g /tmp/rulefile > /tmp/tmprules
sed s/"num_oper = .*"/"num_oper = 9999"/g /tmp/tmprules > /tmp/tmprules1

sed '/./,$!d' /tmp/tmprules1 > /tmp/tmprules

#Copy file into HTX directory
cp  /tmp/tmprules /usr/lpp/htx/rules/reg/hxemem64/memecw

#Copy custom mdt file into htx directory
cp /tmp/mdtpara.smp /usr/lpp/htx/mdt/mdt.mem

#Create a soft link for the file in ecg directory for 'stx' run
ln -sf /usr/lpp/htx/mdt/mdt.mem /usr/lpp/htx/ecg/ecg.mem

