#!/bin/awk -f


# @(#)16        1.21.3.4     src/htx/usr/lpp/htx/etc/scripts/devlist.awk, htxconf, htxubuntu 3/4/14 00:55:00


######################################################################################################
######################################################################################################
#                                                                                                    #
#  awk scripts can be broken down into three main routines                                           #
#                                                                                                    #
# BEGIN         This routine is executed once before any input is read                               #
# MAIN PROGRAM  This routine is executed for each line of input                                      #
# END           This routine is executed once after all input has been read                          #
#                                                                                                    #
#                                                                                                    #
#            devlist.c converted to awk script devlist.awk in September 2004                         #
#                                                                                                    #
######################################################################################################
#############################    Start of the BEGIN Routine    #######################################
######################################################################################################


BEGIN	{
	 # CMVC_RELEASE is exported in /usr/lpp/htx/.bash_profile
         CMVC_RELEASE = snarf("echo $CMVC_RELEASE");
	 atm_index = 0
	 gol_index = 0
	 elp_index = 0
	 knox_index = 0
	 lancer_index = 0
	 scurry_index = 0
	 sanjcto_index = 0
	 lai_index = 0
	 ven_index = 0
	 duv_index = 0
	 kng_index = 0
	 vic_index = 0
	 ohc_index = 0
	 dev_id = ""
	 eth = 0
 	 atm = 0
   	 gol = 0
   	 elp = 0
   	 knox = 0
   	 lancer = 0
   	 scurry = 0
   	 sanjcto = 0
   	 lai = 0
   	 ven = 0
	 duv = 0
	 kng = 0
	 vic = 0
	 ohc = 0
	 line_number = 0
	 found=0
	 count=1
	 kernel_ver = ""
	 system("rm /tmp/locaddr_file 2>/dev/null")
	 system("rm /tmp/irq_file 2> /dev/null")
     	 system("rm /tmp/devlist_warning 2>/dev/null");
     	 
     	 # Making 'kernel_ver' variable 'true' always for 2.6 and higher kernels as 2.4 is no more being used now.
	 # kernel_ver = snarf("uname -r | grep 2.6.")
	 kernel_ver = "true";

	 
# Variable 'count' and array 'IRQ[]','locaddr[]' are used  like a global variable.
# Hence they are available to any function in this script without passing them as a 
# parameter to any function. 

	 if (kernel_ver != ""){
	 	cmd1="ifconfig | awk '($1 ~ \"eth\") { print $1 }' >/tmp/up_cards";
		system(cmd1);
		system("touch /tmp/locaddr_file");
	 	while( getline intface < "/tmp/up_cards" )
		{
			cmd2=sprintf("ethtool -i %s | awk '/bus-info:/ { print $2 }' >>/tmp/locaddr_file",intface);
			system(cmd2)
		}
		close("/tmp/up_cards")
		
		while( getline locaddr[count] < "/tmp/locaddr_file")
		{
			count++;
		}
		close("/tmp/locaddr_file");		
		
	} else {

	 	cmd2 = "ifconfig | grep Interrupt | awk '{ split($1,irq,\":\"); print irq[2] }' > /tmp/irq_file"
	 	system(cmd2)	
	        
		cmd3="ifconfig | grep eth | awk '{ print $1 }' > /tmp/up_cards";
	        system(cmd3)
		
		ethtool_run=0
		use_ethtool=0
	 	system("rm /tmp/ethtool_info 2> /dev/null")
	 	while( getline intface < "/tmp/up_cards" ) 
	 	{
	    		cmd4=sprintf("ethtool -i %s 2>/dev/null | awk '/bus-info:/ { print $2 }' >>/tmp/ethtool_info ",intface)
	    		system(cmd4)
			check_cmd=sprintf("ethtool -i %s 2>/dev/null 1>&2",intface)
			ethtool_run=system(check_cmd)
	 	}
		close("/tmp/up_cards")
		cmd5="cat /tmp/up_cards | wc -l"
		no_upcards=0
		no_interrupts=0
		no_upcards=snarf(cmd5)
		cmd6="ifconfig | grep Interrupt | wc -l";
		no_interrupts=snarf(cmd6)
	        if(no_upcards > 0)
		{
		  if(no_upcards != no_interrupts) { use_ethtool=1; }
		}
	 
	 	while( getline IRQ[count] < "/tmp/irq_file")
	 	{ count++ }
	 	close("/tmp/irq_file")
	}

}

######################################################################################################
##############################     End of the BEGIN Routine    #######################################
######################################################################################################

######################################################################################################
################################    Start of MAIN Program    #########################################
######################################################################################################


####################    The rules below will be executed for kernel 2.6     ##########################


##  Also look out for actual bigbend device id , since on some platform (gpul), only device id is present 

(kernel_ver != "" ) && (/ATM network controller:/ || /Network controller:/ || /VGA compatible controller:/ || /Ethernet controller:/ || /1014:00a1/ || /USB Controller:/ || /RAID bus controller:/) && (found==0) { 
	 
	  	if(/ATM network controller:/) { atm=1 }
	  	
		if(/Ethernet controller:/) { eth=1 }
            
                if(/IBM Obsidian chipset/) { ven=1 }

                if(/IBM Unknown device 0339/) { ven=1 }
	  	
		loc_str=""
		id=$1
	 	sub(/\./,":",id)

# For 2.6 lspci gives domain info also. But in RHEL4 if domian is 0000 it does not show it. 
# See Bugzilla 14540.  So we need to make a check that whether domain info is there or not in
# lspci output.  Hopefully this bug will be fixed in later RHEL4 releases. So then this extra 
# code  can be removed.

		no_elements=split(id,str_id,":")
		if(no_elements == 4)
		{
			loc_str=sprintf("%s", str_id[1])
	  	 	domain_str=sprintf("%s",loc_str) 
			cnt=2      
		}
		if(no_elements == 3)
		{
			loc_str=sprintf("0000:%s",$1)
			cnt=1
		}
	  	
		hex=sprintf("%s",str_id[cnt++])
	  	dec=hex_to_dec(hex) 
	  	 if(dec<10) {  bus_str=sprintf("0%s",dec) }
	  	 else {
	  	 	if(dec<100) {  bus_str=sprintf("0%s",dec) }
	  	 	else { bus_str=sprintf("%s",dec) }
	  	      }
	  	
	  
	  	hex=sprintf("%s",str_id[cnt++])
	  	dec=hex_to_dec(hex)
	  	if(dec<10) {  dev_str=sprintf("0%s",dec) }
	  	else {  dev_str=sprintf("%s",dec) }
	  	
		hex=sprintf("%s",str_id[cnt++])
	  	fn_str=hex_to_dec(hex)
	  	
		if(/1014:00a1/) { dev_id="1014:00a1" }
	  	split($0,device,":")

		if(no_elements==4)
		{
			temp_str=sprintf("%s:%s:%s:",device[1],device[2],device[3])
		}
		if(no_elements==3)
		{
			temp_str=sprintf("%s:%s:",device[1],device[2])
		}
	  	dev_name=$0
	  	sub(temp_str,"",dev_name)

## Supported goliad cards are 82545EM & 82545GM 
## 8086:1027 is used as sometimes it is shown in place of 82545GM (defect 476928) 
## Supported duval cards are of type 82546EB & 82546GB.
## Supported elpaso card is of type 82571EB
## Supported knox card is shown(lspci) as "Intel Corporation Unknown device 10c7 (rev 01)"
		
		if(/82545/ || /8086:1027/) { gol=1 }
	  	if(/82557/) { scurry=1 }
	  	if(/4022/ || /Network controller:/) { 
				flag=find_scsi(loc_str);
				if(flag == 0)
				{
				sanjcto=1 
				}
		}	
	     	if(/82546/) { duv=1 }
	     	if(/82571/) { elp=1 }
	     	if(/10c7/) { knox=1 }
	     	if(/10df:e220/ || /Ethernet controller: Emulex Corporation OneConnect NIC/) { lancer=1 }
	  	if(/USB/) { ohc=1 }
	  	if(/S2io/) { kng=1 }
	  	if(/1a48/) { vic=1 }
	  	if(/VGA compatible controller:/ && /Matrox Graphics/) { lai=1 }
        if(/G550/) { lai=0 }
		found=1
	  	line_number=NR
		line_number += 2
	  	next
	       }
	  
(kernel_ver != "") && (found==1) && (ven==1) && /Subsystem:/ { 
                 if( (!/IBM Unknown device 02cd/) && (!/IBM Unknown device 02ce/)) {
                 ven=0; next } 
}

(kernel_ver != "") && (found==1) && (line_number==NR) { 
		if( /IRQ/ ) {
				irq_no=$NF
			 	flag=0
			 	flag=check_locaddr(loc_str)
				if(flag==1)
				{ atm=0; dev_id=""; eth=0; gol=0; scurry=0; duv=0; elp=0; knox=0; lancer=0;sanjcto=0; lai=0; ven=0; kng=0; vic=0 }
			
				if(((atm==1)||(dev_id=="1014:00a1"))&&(flag==0)){
					printf("atm%d - %s:%s:%s:%s - %s\n", atm_index++, bus_str, dev_str,fn_str, irq_no, dev_name)
					dev_id=""
					atm=0
					}
				if(((sanjcto==1)||(dev_id=="4022:02d0"))&&(flag==0)){
					printf("sanjcto%d - %s:%s:%s:%s - %s\n", sanjcto_index++, bus_str, dev_str,fn_str, irq_no, dev_name)
					dev_id=""
					sanjcto=0
					}
				if(((lai==1)||(dev_id=="0525:0233"))&&(flag==0)){
					printf("lai%d - %s:%s:%s:%s - %s\n", lai_index++, bus_str, dev_str,fn_str, irq_no, dev_name)
					dev_id=""
					lai=0
					}
				if(((ven==1)||(dev_id=="1014:02cd"))&&(flag==0)&&(CMVC_RELEASE == "htxltsbml")){
					printf("ven%d - %s:%s:%s:%s - %s\n", ven_index++, bus_str, dev_str,fn_str, irq_no, dev_name)
					dev_id=""
					ven=0
					}
  				if((eth==1) && (flag==0)){
					if(gol==1){
						printf("gol%d - %s:%s:%s:%s - %s\n", gol_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
						gol=0;scurry=0;duv=0 }
					if(scurry==1){
						printf("scurry%d - %s:%s:%s:%s - %s\n", scurry_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
						scurry=0;duv=0 }
					if(duv==1){
						printf("duv%d - %s:%s:%s:%s - %s\n", duv_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
						duv=0 }
					if(elp==1){
					        printf("elp%d - %s:%s:%s:%s - %s\n", elp_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
					        elp=0 }
					if(knox==1){
					        printf("knox%d - %s:%s:%s:%s - %s\n", knox_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
					        knox=0 }
					if(lancer==1){
					        printf("lancer%d - %s:%s:%s.%s:%s - %s\n", lancer_index++, domain_str, bus_str, dev_str, fn_str, irq_no, dev_name)
					        lancer=0 }
					if(kng==1){
						printf("kng%d - %s:%s:%s:%s - %s\n", kng_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
						kng=0 }
					if(vic==1){
						printf("vic%d - %s:%s:%s:%s - %s\n", vic_index++, bus_str, dev_str, fn_str, irq_no, dev_name)
						vic=0 }
					eth=0
			       }
			       if(ohc==1) {
					printf("ohc%d - %s:%s:%s:%s - %s\n", ohc_index++, bus_str, dev_str,fn_str, irq_no, dev_name)
					ohc=0
				}
     		}
										
		found=0
}

############################    End of rules for kernel 2.6     ######################################
#####                           	Defect 416190                                            #####
#####  IRQ check for each card is performed beacuse we do not want to test the card from which   #####
#####  machine is on the network. Modification for Defect 416190                                 #####
#####                                                                       			 #####
####################    The rules below will be executed for kernel 2.4   ############################


(kernel_ver=="") && ($1=="Bus") && ($3=="device") && ($5=="function") {
    	 	 bus_str=$2
	  	 sub(/,/,"",bus_str)
	  	 dev_str=$4
	 	 sub(/,/,"",dev_str)
	 	 fn_str=$6
	 	 sub(/:/,"",fn_str)
	  	 line_number=NR
		 line_number++
	  	 found=1
	  	 next
	 	}

(kernel_ver=="") && (found==1) && (line_number==NR) {
		 split($0,device,":")
		 dev_class=device[1]
		 temp_var=sprintf("%s:",device[1])
		 dev_name=$0
		 sub(temp_var,"",dev_name)
		
##  Look out for actual bigbend device id , since on some platform (gpul), only device id is present 
                
		 if(/1014:00a1/) { dev_id="1014:00a1"; }
		 
## Supported goliad cards are 82545EM & 82545GM 
## 8086:1027 is used as sometimes it is shown in place of 82545GM (defect 476928) 
## Supported duval cards are of type 82546EB & 82546GB.
		 
		 if(/82545/ || /8086:1027/) { gol=1 }
		 if(/82557/) { scurry=1 }
		 if(/4022/ || /Network controller:/) { sanjcto=1 }
	  	 if(/VGA compatible controller:/ && /Matrox Graphics/) { lai=1 }
		 if(/82546/) { duv=1 }
		 if(/USB/) { ohc=1 }				
		 found=2
		 line_number++
		 next
		}

(kernel_ver=="") && (found==2) && (line_number==NR) {
		 if(/IRQ/) {
		   irq_no=$2
		   sub(/\./,"",irq_no)
		   flag=0
		   if(use_ethtool == 0) {
		   	flag=check_IRQ(irq_no)
	           }
		   if(use_ethtool==1) {
			flag=check_IRQ_ethtool(bus_str,dev_str,fn_str)
		  }
		   if(flag==1){
			dev_id=""
			gol=0
			scurry=0
			duv=0
		     }	
		   
		   if( (( dev_class ~ "ATM network controller") || (dev_id == "1014:00a1")) && (flag==0) ){
			printf("atm%d - %s:%s:%s - %s\n", atm_index++, bus_str, dev_str, fn_str,dev_name)
			dev_id="" 
		   } 
		   
		   if( (( dev_class ~ "Network controller") || (dev_id == "4022:02d0")) && (flag==0) ){
			printf("sanjcto%d - %s:%s:%s - %s\n", sanjcto_index++, bus_str, dev_str, fn_str,dev_name)
			dev_id="" 
		   } 
		   
		   if( (( dev_class ~ "VGA compatible controller") || (dev_id == "0525:0233")) && (flag==0) ){
			printf("lai%d - %s:%s:%s - %s\n", lai_index++, bus_str, dev_str, fn_str,dev_name)
			dev_id="" 
		   }
 
		   if( (( dev_class ~ "IBM Unknown device 02cd") || (dev_id == "1014:02cd")) && (flag==0) ){
			printf("ven%d - %s:%s:%s - %s\n", ven_index++, bus_str, dev_str, fn_str,dev_name)
			dev_id="" 
		   }
 
		   if( (( dev_class ~ "IBM Unknown device 02ce") || (dev_id == "1014:02ce")) && (flag==0) ){
			printf("ven%d - %s:%s:%s - %s\n", ven_index++, bus_str, dev_str, fn_str,dev_name)
			dev_id="" 
		   }
 
		   if((dev_class ~ "Ethernet controller") && (flag==0)) {
			if(gol==1) {
		 	  printf("gol%d - %s:%s:%s - %s\n",gol_index++, bus_str, dev_str, fn_str,dev_name)  
		 	  gol=0;scurry=0;duv=0 }
			
			if(scurry==1) {
			  printf("scurry%d - %s:%s:%s - %s\n", scurry_index++, bus_str, dev_str, fn_str, dev_name)
			  scurry=0;duv=0}
			
			if(duv==1) {
			  printf("duv%d - %s:%s:%s - %s\n",duv_index++, bus_str, dev_str, fn_str, dev_name)
			  duv=0
			}
		  }
		  
		  if( dev_class ~ "USB Controller" )  {
			printf("ohc%d - %s:%s:%s - %s\n", ohc_index++, bus_str, dev_str, fn_str,dev_name)
			ohc=0
		 }
		}

		found=0
		dev_id=""
}

#########################    End of MAIN program and rules for kernel 2.4    #########################


######################################################################################################
###################################   Function definitions    ########################################
######################################################################################################

function check_IRQ(no) {
	 value=0
	 for(j=1;j<count;j++)
	 {   	if(no==IRQ[j])
	       	{ 	value=1
	       	 	break
		}
	 }
	 return value
	}


function check_IRQ_ethtool(bus,dev,fn) {
	value=0	 
 	
	if ( ethtool_run != 0 ) 
	{ system("echo \"\nWARNING : UP cards are not excluded.\" >/tmp/devlist_warning");
	  system("echo \"If you are running HTX from telnet session , you might lose connection to test machine.\" >>/tmp/devlist_warning");
	  system("echo \"Try to run HTX from console.\n\" >>/tmp/devlist_warning");
	  ethtool_run = 0
	}

	while ( getline info < "/tmp/ethtool_info" )
	{	
	 	sub(/\./,":",info)
	 	split(info,temparr,":")
	  	hex=sprintf("%s",temparr[1])
	  	bus_no=hex_to_dec(hex) 
	  
	  	hex=sprintf("%s",temparr[2])
	  	dev_no=hex_to_dec(hex)
		
		hex=sprintf("%s",temparr[3])
	  	fn_no=hex_to_dec(hex)
		if ((bus_no==bus) && (dev_no==dev) && (fn_no==fn))
		{ 
			value=1
			break 
		}		
	
	}
	return value
	}
	  	
function check_locaddr(addr) {
	 value=0
	 for(j=1;j<count;j++)
	 {   	if(addr==locaddr[j])
	       	{ 	value=1
	       	 	break
		}
	 }
	 return value
}

function get_IRQ_no(no){
	 cmd=sprintf("hwinfo --netcard | grep -B 1 -s \"%s\" | awk '/IRQ/ { print $2 }' >> /tmp/irq_file",no)
	 system(cmd)
	 }
	 
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

function find_scsi(addr) {
        found=0;
        cmd=sprintf("lsscsi -v 2>/dev/null | grep %s | wc -l",addr);
        found=snarf(cmd);
        return found;
}

