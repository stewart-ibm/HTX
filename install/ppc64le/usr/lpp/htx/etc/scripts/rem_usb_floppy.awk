#!/bin/awk -f

# @(#)04        1.5.4.1  src/htx/usr/lpp/htx/etc/scripts/rem_usb_floppy.awk, htxconf, htxubuntu 9/19/11 03:32:44

# List all the Floppy Disks Connected to USB
# Include <system("lsusb -v | ./flpy_on_usb.awk");> in fine hxeconf.awk
# after the line <system("lspci -v | ./devlist.awk > /tmp/devlist.txt");>
# to remove the ohc from the devlist.txt to which Floppy disk is connected.

# The Begining of the Script i.e. initial setup
BEGIN   {
    fd=0
    serial=0
    bus_tmp=998
    device_tmp=998

    # Getting kernel Version.
    # Making 'kernel' variable 'true' always for 2.6 and higher kernels as 2.4 is no more being used now.
    # kernel=""
    # kernel=snarf("uname -r | grep 2.6.");
    
    kernel = "true"
    
   # printf("kernel=%s\n",kernel)

    # The File from the Input is taken
    src_file="/tmp/devlist.txt"
    # File where the output is saved
    dst_file="/tmp/dev.txt"

    # Taking Initial Backup of the devlist.txt
    system("cp /tmp/devlist.txt /tmp/devlist.txt.bak")
    system("cp /tmp/devlist.txt /tmp/dev.txt")
}

# Store Bus , Device and ID for the present device
($1=="Bus")&&($3=="Device")&&($5=="ID"){
bus_tmp=$2
device_tmp=$4
#printf("tmp=%s,tmp_dev=%s\n",bus_tmp,device_tmp)
}

(/bInterfaceClass/) {
str=""
str2=""
# Check for bInterfaceClass         8 Mass Storage
if ($2==8)  {
	#printf("tmp=%s,bus=%s,tmp=%s,device=%s,hub=%d\n",bus_tmp,bus,device_tmp,device,hub)
	str=sprintf("lsusb -v -s %s:001 | grep iSerial | awk '{print $3}'",bus_tmp,device_tmp)
    #printf("str=%s\n",str)
	str2=snarf(str)
    #printf("str2=%s\n",str2)
	id=str2
    sub(/\./,":",id)
    n=split(id,str_id,":")
    if(n==3) {
        i=0
        bus_dev_fn=sprintf("%s:%s.%s",str_id[1],str_id[2],str_id[3])
    }
    else if(n==4) {
        i=1
        if(str_id[1]=="0000") {
            bus_dev_fn=sprintf("%s:%s.%s",str_id[2],str_id[3],str_id[4])
        }
        else {
            bus_dev_fn=sprintf("%s:%s:%s.%s",str_id[1],str_id[2],str_id[3],str_id[4])
        }
    }
   # printf("\nbus_dev_fn=%s\n",bus_dev_fn)
    if(kernel!="") {
        irq=snarf("lspci -v  | awk '$1==\""bus_dev_fn"\" { n = NR; n+=2; }; (n==NR) && (/IRQ/) { print $NF }'")
    }
   # printf("IRQ=%d",irq)

    hex=sprintf("%s",str_id[i+1])
    dec=hex_to_dec(hex)
    if(dec<10) {  bus_str=sprintf("00%s",dec) }
    else {
        if(dec<100) {  bus_str=sprintf("0%s",dec) }
        else { bus_str=sprintf("%s",dec) }
    }

    hex=sprintf("%s",str_id[i+2])
    dec=hex_to_dec(hex)
    if(dec<10) {  dev_str=sprintf("0%s",dec) }
    else {  dev_str=sprintf("%s",dec) }

    hex=sprintf("%s",str_id[i+3])
    fn_str=hex_to_dec(hex)

    if(kernel!="") {
        temp_str=sprintf("%s:%s:%s:%s",bus_str,dev_str,fn_str,irq)
    }
    else {
        temp_str=sprintf("%s:%s:%s",bus_str,dev_str,fn_str)
    }

	#printf("\n---------%s---------\n",temp_str)

    system("awk '$3 !~ /"temp_str"/ { print $0 }' "src_file" > "dst_file)
    system("cp "dst_file" "src_file)

    fd=0
    serial=0
	}
}

# Function to Convert a Hexadecimal number to a Decimal number
function hex_to_dec(hex_no){
    system(" rm /tmp/num_file 2>/dev/null")
    tmp_no=toupper(hex_no)
    hex_no=tmp_no
    printf("ibase=16\n%s\nquit",hex_no) > "/tmp/num_file"
    close("/tmp/num_file")
    dec_no=snarf("bc -q /tmp/num_file 2>/dev/null")

    return dec_no
}

function snarf(cmd) {
    snarf_input=""
    cmd | getline snarf_input
    close(cmd)
    return snarf_input
}

# The Ending of the Script.
END {
    # Removing unwanted or temporary files used while working.
    system("rm "dst_file)
    system("rm /tmp/devlist.txt.bak")
}

