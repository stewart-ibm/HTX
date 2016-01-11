#!/bin/sh

# @(#)50        1.2  src/htx/usr/lpp/htx/etc/scripts/create_my_mdt.sh, htxconf, htxubuntu 12/9/08 10:00:57

if [ "$*" != "" ]
then

case "$1" in
"--help"|"-h")
  echo "Usage:"
  echo "create_my_mdt <dev_name1>:<rule_filename1>  <dev_name2>:<rule_filename2> ...."
  echo
  echo "Note: If only devices type is given (like only sctu), omitting device id, all such devices(sctu0, sctu1 ...)"
  echo "will be picked up. If colon (':') is omitted after dev_name, default rule file is used."
  exit 0
;;
esac

i=0
# While loop for parsing parameters into device array and rulefile array
while [ $# != 0 ] 
do
  # devrule contains one parameter at a time for example <device_name>:<rulefile_name>
  devrules=$1

  # Parsing device name and putting in a array
  dev_name=`echo "$devrules" | awk -F : '{print $1}'`
  devs[i]=$dev_name

  # Parsing rulefile name
  rulefile_name=`echo "$devrules" | awk -F : '{print $2}'` 
  rules[i]=$rulefile_name
  
  i=`expr $i + 1`  
  shift
done

count=0
# While loop for taking devices from devs and put one by on seperated by space eg. <device name1> <device name2> ...
while [ $count -lt ${#devs[*]} ]
do
  # If condition for checking rule as rules.seq for sctu only. 
  # By using 'rules.seq' with sctu user wants to run sctu in a config in which sctu picks up all SMT threads per core.
  # We would be creating sctu stanzas differently (than the default ones in htxconf.awk) for this later.
  if [ "${rules[$count]}" != "rules.seq" ]
  then
    devices="${devices}${devs[$count]} "
  fi
count=`expr $count + 1`
done

# Putting mdt name in a variable
mf=${HTXMDT}mdt.temp


# Creating a temporary mdt and  will have stanza for those devices which is present in devices variable 
  cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk $devices >$mf

# While loop for creating final mdt and replacing rule file name for perticular device
c=0
while [ $c -lt ${#devs[*]} ]
do
  # Switch case for devices which is not present in mdt.bu. 
  # Here it will look into respective mdt for that device and edit rule file name and generate the stanza.	
  case ${devs[$c]} in

  ablink)
    cat ${HTXMDT}mdt.ablink | create_mdt_with_devices.awk ${devs[$c]} | create_mdt_without_devices.awk default >>$mf  	
  ;;

  xyzlink)
   cat ${HTXMDT}mdt.xyzlink | create_mdt_with_devices.awk ${devs[$c]} | create_mdt_without_devices.awk default >> $mf
  ;;
      
  iba)
    cat ${HTXMDT}mdt.hca | create_mdt_with_devices.awk ${devs[$c]} | create_mdt_without_devices.awk default >> $mf
    cat ${HTXMDT}mdt.hca.wrap | create_mdt_with_devices.awk ${devs[$c]} | create_mdt_without_devices.awk default>>$mf
  ;;
  esac
  # If rulefile is given then edit rulefile otherwise it will be the default  
  if [ "${rules[$c]}" != "" ]
  then
  if [ "${rules[$c]}" = "rules.seq" ] 
  then
    if [ "${devs[$c]}" = "sctu" ]
    then
      cat /dev/null/ | create_seq_sctu_stanzas.awk >>$mf
    fi
  fi
# Changing rulefile name for respective device for /reg/ and /emc/ 
  cat $mf | awk -F '/' 'BEGIN{t=0}; /:/ {if(t==1) {t=0}; if(dev ~ /[0-9]$/) {dev=sprintf("%s:",dev)};if( dev == $1) { t=1};d=sprintf("^%s[0-9]*:",dev);if($1 ~ d) {t=1};}(t==1) && (/reg_rules/||/emc_rules/){sub(/[A-Za-z.0-9]*/,rule,$2);print $1"/"$2;next};{print}' rule=${rules[$c]}  dev=${devs[$c]} >$mf
	
  fi
c=`expr $c + 1`
done

# Throw contents on screen and remove temp mdt file
cat $mf
rm $mf

#Else part will create stanza as default
else
opt=""

echo "\nHow do you want to create your own mdt file: -"
echo "With devices of your choice: 1"
echo "Without devices of your choice: 2"
echo "Note: Chosen devices will be selected (if option 1) or dropped (if option 2) from 'mdt.all'"
echo "while creating new mdt file of your choice."
while [ "$opt" = "" ]
do
  echo  "\nEnter 1 or 2: \c"
  read opt
  if [ "$opt" != "1" -a "$opt" != "2" ]
  then
    echo "Invalid choice!"
    opt=""
  fi
done

echo "\nFor device names used by HTX and their availability on this system,"
echo "you can look into 'mdt.all' in '$HTXMDT' directory"

echo "\nEnter device names, seperated by space only: \c"
read devices

echo "\nmdt files already present in '$HTXMDT' directory"
ls ${HTXMDT}

echo "\nEnter new mdt file name: \c"
read my_mdt_file 

echo "\nCreating new mdt file: $my_mdt_file"
if [ $opt = "1" ]
then
  echo "With devices: $devices"
  cat ${HTXMDT}mdt.all | create_mdt_with_devices.awk $devices > ${HTXMDT}$my_mdt_file
fi 

if [ $opt = "2" ]
then
  echo "Without devices: $devices"
  cat ${HTXMDT}mdt.all | create_mdt_without_devices.awk $devices > ${HTXMDT}$my_mdt_file
fi

fi
