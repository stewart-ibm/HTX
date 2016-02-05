#!/bin/awk -f

# @(#)68	1.7  src/htx/usr/lpp/htx/etc/scripts/parse_equaliser_configuration_file.awk, htxconf, htxubuntu 5/7/15 05:06:19 


# This file is meant to print the mdt stanzas for equaliser to standard output
# Its output can be redirected to a file to make a suitable mdt file for equaliser.

# Begin: Executed once before processing input

    BEGIN {

# If any argument given then print correct usage, else parse the default config file.
	if(ARGV[2] != "") {
	    { print "Correct Usage : ./parse_equaliser_configuration_file.awk <suitable cfg file name> > <suitable mdt file name>" }
	    exit
	}
	else {
	    cmd=sprintf("basename %s",ARGV[1]);
	    CONFIG_FILE_NAME=snarf(cmd);
	}

	{HTX_HOME=ENVIRON["HOME"]}
	{DEVICE_NAME=""}
	{MDT_FILE_PATH="/usr/lpp/htx/mdt"}

# Create "default" stanza
	printf("default:\n");
	HE_name(""); 
	adapt_desc(""); 
	device_desc(""); 
	string_stanza("reg_rules","","reg rules"); 
	string_stanza("emc_rules","","emc rules"); 
	dma_chan(0); 
	idle_time(0); 
	intrpt_lev(0) 
	load_seq(32768); 
	max_run_tm(7200); 
	port("0"); 
	priority(19); 
	slot("0"); 
	max_cycles("0"); 
	hft(0); 
	cont_on_err("YES"); 
	halt_level("1"); 
	start_halted("n"); 
	dup_device("n"); 
	log_vpd("y"); 
	create_tlbie_rules=0;

# Enable equaliser flag 
    equaliser_flag("1");

# cfg file used corresponding to current mdt
    cfg_file(CONFIG_FILE_NAME);

# Equaliser Debug flag , default disabled
    equaliser_debug_flag("0");

	{EQ_RULE_FILE_NAME_CACHE="default.cache.eq"}
	{EQ_RULE_FILE_NAME_FPU="default.fpu.eq"}

# Here we are checking the default pagesize it can either 4k or 64k page 
#if default_pagesize is 64k we choose "default.mem.eq.64k" else we choose "default.mem.eq.4k"
	default_pagesize = snarf("getconf PAGESIZE");
		if(default_pagesize == 65536) {
		{EQ_RULE_FILE_NAME_MEM="default.mem.eq.64k"}
	}else{
		{EQ_RULE_FILE_NAME_MEM="default.mem.eq.4k"}
	}

	if(CONFIG_FILE_NAME == "eq_th_trans_switch.cfg") {
		{EQ_RULE_FILE_NAME_CPU="cpu.eq_th_trans_switch"}
	} 
	else {
		{EQ_RULE_FILE_NAME_CPU="default.cpu.eq"}
  	}
	if (CONFIG_FILE_NAME == "htx_eq_cpu_mem_100.cfg") { 
		{EQ_RULE_FILE_NAME_MEM="mem.eq.90"}
	}
	else if (CONFIG_FILE_NAME == "htx_eq_cpu_mem_50.cfg") { 
		{EQ_RULE_FILE_NAME_MEM="mem.eq.50"}
	}
    }

# Rule 1 : Skip all lines that :
#          a) Begin with '#' :comment
#          b) Have more than 1 field: non blank lines
#          c) Dont match "timeQuantum" string as its not data for this script to interpret.

    ($1 !~ "#") && ($1 !~ "time_quantum") && ($1 !~ "startup_time_delay")&& (NF > 1) && ($1 !~ "log_duration" ) { 

# Get the device name from 1st column of config file
	DEVICE_NAME=$1 
# Get the equaliser control flag . 
	EQ_ENABLE=$3

	shared_processor_mode=snarf("cat /tmp/htx_syscfg | grep 'shared_processor_mode' | awk -F: '{print $2}'")
# If cache, create a cache stanza with the relevant device name & rule file for equaliser
	if(DEVICE_NAME ~ /cache+/ && shared_processor_mode == " no") {
    mkstanza("hxecache","","Processor Cache",DEVICE_NAME,"hxecache",EQ_RULE_FILE_NAME_CACHE,EQ_RULE_FILE_NAME_CACHE,EQ_ENABLE);
	    cont_on_err("NO");
	}
# If mem, create a mem stanza with the relevant device name & rule file for equaliser
	else if(DEVICE_NAME ~ /mem+/){
	    mkstanza("hxemem64","64bit","memory",DEVICE_NAME,"hxemem64",EQ_RULE_FILE_NAME_MEM,EQ_RULE_FILE_NAME_MEM,EQ_ENABLE);
	    cont_on_err("NO");
	}
# If fpu, create a fpu stanza with the relevant device name & rule file for equaliser
	else if(DEVICE_NAME ~ /fpu+/){
	    mkstanza("hxefpu64", "core", "floating_point", DEVICE_NAME, "hxefpu64", EQ_RULE_FILE_NAME_FPU,EQ_RULE_FILE_NAME_FPU,EQ_ENABLE);
	    cont_on_err("NO");
	}
	else if(DEVICE_NAME ~ /cpu+/){
		mkstanza("hxecpu", "processor","processor" , DEVICE_NAME, "cpu", EQ_RULE_FILE_NAME_CPU,EQ_RULE_FILE_NAME_CPU,EQ_ENABLE);
		cont_on_err("NO");
	}

}

# End: Executed once before exit
    END {
# flush stdout
	print "\n" 
    }

############################################################################## 
#####################     FUNCTION DEFINITIONS     ########################### 
############################################################################## 
function deleted(dev) { 
    if (flag[dev] == "-d") return 1 
} 

function string_stanza(a,b,c) { 
    len=(length(a) + length(b) + 8 + 3 + 2) - 60 
    if (create_tlbie_rules) {
	printf("\t%s = \"%s\" %" len "s * %s\n",a,b,"",c) >> tlbie_mdtfile;
    }
    else { 
	printf("\t%s = \"%s\" %" len "s * %s\n",a,b,"",c);
    }
} 
function number_stanza(a,b,c) { 
    len=(length(a) + length(b) + 8 + 3) - 60 
    if (create_tlbie_rules) {
	printf("\t%s = %s %" len "s * %s\n",a,b,"",c) >> tlbie_mdtfile;
    }
    else {
	printf("\t%s = %s %" len "s * %s\n",a,b,"",c);
    }
}
function HE_name(x) { 
    string_stanza("HE_name",x,"Hardware Exerciser name, 14 char");
} 
function max_cycles(x) { 
    string_stanza("max_cycles",x,"max cycles");
} 
function adapt_desc(x) {
    gsub(" ","_",x); 
    string_stanza("adapt_desc",x,"adapter description, 11 char max.");
} 
function cont_on_err(x) { 
    string_stanza("cont_on_err",x,"continue on error (YES/NO)");
}
function device_desc(x) {
    gsub(" ","_",x); 
    string_stanza("device_desc",x,"device description, 15 char max.");
}
function halt_level(x) { 
    string_stanza("halt_level",x,"level <= which HE halts");
}
function start_halted(x) {
    string_stanza("start_halted",x,"exerciser halted at startup");
}
function dup_device(x) {
    string_stanza("dup_device",x,"duplicate the device ");
}
function log_vpd(x) {
    string_stanza("log_vpd",x,"Show detailed error log");
}
function port(x) {
    string_stanza("port",x,"port number");
}
function slot(x) {
    string_stanza("slot",x,"slot number");
}
function dma_chan(x) {
    number_stanza("dma_chan",x,"DMA channel number");
}
function hft(x) {
    number_stanza("hft",x,"hft number");
}
function idle_time(x) {
    number_stanza("idle_time",x,"idle time (secs)");
}
function intrpt_lev(x) {
    number_stanza("intrpt_lev",x,"interrupt level");
}
function load_seq(x) {
    number_stanza("load_seq",x,"load sequence (1 - 65535)");
}
function max_run_tm(x) {
    number_stanza("max_run_tm",x,"max run time (secs)");
}
function priority(x) {
    number_stanza("priority",x,"priority (1=highest to 19=lowest)");
}
function snarf(cmd) { 
    snarf_input=""; 
    cmd | getline snarf_input; close(cmd); return snarf_input; 
}
function equaliser_flag(x) {
    string_stanza("equaliser_flag",x,"Equaliser flag enabled for supervisor");
} 
function cfg_file(x) {
    string_stanza("cfg_file",x,"Corresponding cfg file for this mdt");
}
function equaliser_debug_flag(x) {
    string_stanza("equaliser_debug_flag",x,"Equaliser Debug Flag for supervisor");
} 

# use mkstanza() to generate the basic stanza, arguments are: 
#     hename,                           # "hxecd" 
#     adapt_desc,                       # "scsi" 
#     device_desc,                      # "cdrom" 
#     device_name,                      # "rcd0" 
#     rulesfile_directory,              # "cd" 
#     default_reg_rules,                # "cdrom.ibm" 
#     default_emc_rules                 # "cdrom.ibm" 
# 

function mkstanza(hxe,a,d,dev,rfdir,reg,emc,eqsupport) { 
# for 4.1, rules file directory is same as exerciser name 
    rfdir = hxe 

# make device name entry
    if (create_tlbie_rules) {
	printf("\n") >> tlbie_mdtfile;
    }
    else {
	printf("\n")
    }

    if (create_tlbie_rules) { 
	printf("%s:\n",dev)>> tlbie_mdtfile;
    }
    else {
	printf("%s:\n",dev);
    }

# make exerciser entry 
    if(he[dev]) { 
	HE_name(he[dev]); 
# for 3.2, rules file directory is exerciser name minus leading "hxe" 
	rfdir = he[dev]; 
	sub(/^hxe/,"",rfdir); 
# for 4.1, rules file directory is same as exerciser name 
	rfdir = he[dev]; 
    }
    else { 
	if (hxe) HE_name(hxe); 
    }

# make adapter and device description entries 
    if (a) adapt_desc(a) 
    if (d) device_desc(d) 

# make rules file entries 
    if(rf[dev]) { 
	string_stanza("reg_rules",sprintf("%s/%s",rfdir,rf[dev]),"reg"); 
	string_stanza("emc_rules",sprintf("%s/%s",rfdir,rf[dev]),"emc"); 
    }
    else { 
	if(reg) 
	    string_stanza("reg_rules",sprintf("%s/%s",rfdir,reg),"reg"); 
	if(emc) 
	    string_stanza("emc_rules",sprintf("%s/%s",rfdir,emc),"emc"); 
    }
# make start_halted entry 
    if(flag[dev] == "-h") 
	string_stanza("start_halted","y","exerciser halted at startup"); 
}

############################################################################## 
##################     END OF FUNCTION DEFINITIONS     ####################### 
############################################################################## 

