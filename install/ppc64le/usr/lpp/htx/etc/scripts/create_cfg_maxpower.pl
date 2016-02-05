#!/usr/bin/perl

# @(#)40	1.6  src/htx/usr/lpp/htx/etc/scripts/create_cfg_maxpower.pl, htxconf, htxubuntu 5/7/15 05:06:22

# This script will create equaliser config files to vary the workload running under HTX
# such that it shakes the system between 0 and max. power configuration.
# 2 types of scenarios will be created:
# 1. Switching: Here the workload pattern will be such that the power consumption for all the
#    cores (together) will be switching instantly between 0 and Pmax.
# 2. Exponential: Here the workload pattern will be such that power consumption of the whole system
#    will increase/decrease to Pmax./0 gradually. This will be achieved by adding/deleting 1 core to
#    the active list of cores (which are having workload running) at regular intervals.
#
# The kind of workload will be such that when the core is in the list of active cores, it will consume
# maximum power. The type of workload whiich will be used for it is as below:
# 50% of threads of a core will be running hxefpu64 with VSX workload.
# 25% of threads of a core will be running hxecpu with fixed point type workload.
# 25% of threads of a core will be running hxemem64 to have cache and memory stress.

################################################################
################### Main Starts here    ################
################################################################

$debug=0;
@pattern;

# Get no. of cores available on the system.
$no_of_cores = `cat /tmp/htx_syscfg | grep "Number of Cores" | awk '{print \$5}'`;
chomp($no_of_cores);
#print "no_of_cores: $no_of_cores\n";

# Determine SMT threads per processor
$lcpus = `grep processor /proc/cpuinfo | wc -l`;
$pcpus = `ls -l /proc/device-tree/cpus | grep ^d | awk '(\$NF ~ /POWER/)' | wc -l`;
$smt_threads = int ($lcpus / $pcpus);
if ($smt_threads == 0) {
	$smt_threads = 1;
}

# Create cfg file for switching test case
$cfg_file="/usr/lpp/htx/htx_eq_maxpwr_switch.cfg";
unless (open (CFG_FILE, ">$cfg_file")) {
    die ("Open failed for file $cfg_file\n");
}
make_entry(100); # Will create common entries in cfg file.
create_cfg1();    # will create exerciser entries in cfg file.
close(CFG_FILE);

# Create cfg file for exponential inc./dec. test case.
$cfg_file="/usr/lpp/htx/htx_eq_maxpwr_exp.cfg";
unless (open (CFG_FILE, ">$cfg_file")) {
	die ("Open failed for file $cfg_file\n");
}

make_entry(4000); # Will create common entries in cfg file.
create_cfg2();    # will create exerciser entries in cfg file.
close(CFG_FILE);

# Create cfg file for cpu only
$cfg_file="/usr/lpp/htx/htx_eq_cpu.cfg";
unless (open (CFG_FILE, ">$cfg_file")) {
	die ("Open failed for file $cfg_file\n");
}
# Create cfg file for 100/75/50/25 % cpu load
     $cfg_file="/usr/lpp/htx/htx_eq_100_75_50_25_util.cfg";
     unless (open (CFG_FILE, ">$cfg_file")) {
          die ("Open failed for file $cfg_file\n");
     }
      make_entry(300000); # Will create common entries in cfg file.
      create_cfg3();    # will create exerciser entries in cfg file.
      close(CFG_FILE);
# Create cfg file for 100% cpu & 90% memory usage
     $cfg_file="/usr/lpp/htx/htx_eq_cpu_mem_100.cfg";
     unless (open (CFG_FILE, ">$cfg_file")) {
          die ("Open failed for file $cfg_file\n");
     }
      make_entry(300000); # Will create common entries in cfg file.
      create_cfg4();    # will create exerciser entries in cfg file.
      close(CFG_FILE);
# Create cfg file for 50% cpu & 50% memory usage
     $cfg_file="/usr/lpp/htx/htx_eq_cpu_mem_50.cfg";
     unless (open (CFG_FILE, ">$cfg_file")) {
          die ("Open failed for file $cfg_file\n");
     }
      make_entry(300000); # Will create common entries in cfg file.
      create_cfg5();    # will create exerciser entries in cfg file.
      close(CFG_FILE);

#################################################
#############   Main ends here  #################
#################################################

#######################################################################
#######     Function to create common entries in cfg file   #######
#######################################################################
sub make_entry() {
	 print CFG_FILE ("# if first letter in the line is \"#\", its taken as a comment\n\n");
	 print CFG_FILE ("#timeQuantum in milliseconds\n");
	 print CFG_FILE ("time_quantum = $_[0]\n\n");
	 print CFG_FILE ("#startup time delay specified in seconds\n");
	 print CFG_FILE ("startup_time_delay = 30\n\n");
	 print CFG_FILE ("# Log equalizer status for the last <log_duration> (in secs) only\n");
	 print CFG_FILE ("log_duration = 60\n\n");
	 print CFG_FILE ("# In utilizationSequence,upto 10 steps are allowed.\n#\n");
	 print CFG_FILE ("#\tdev_name\teq_control\t\tutilization_sequence\t\tutilization_pattern\n");
	 print CFG_FILE ("#\t--------\t----------\t\t--------------------\t\t-------------------\n");
}

##############################################################################
########    Function to create cfg file for switching scenario  ######
##############################################################################
sub create_cfg1() {
    $fpu_inst = $cpu_inst = $mem_inst = 0;
    $fpu_inst = int ( $smt_threads / 2);
    if ($fpu_inst == 0) {
        $fpu_inst = 1;
    } else {
        $cpu_inst = int ($fpu_inst / 2);
        if ($cpu_inst == 0) {
            $cpu_inst = 1;
        } else {
            $mem_inst = $cpu_inst;
        }
        $left = ($smt_threads) % ($fpu_inst + $cpu_inst + $mem_inst);
        while ($left != 0) {
            if ($mem_inst == 0 || $left % 2 == 0) {
                $mem_inst++;
            } else {
                $cpu_inst++;
            }
            $left--;
        }
    }
    for ($i=0; $i < $lcpus;) {
        for ($j=0, $k=$i; $j < $fpu_inst; $j++, $k++) {
            $dev=fpu.$k;
#           print "dev: $dev\n";
            print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[11111111110000000000\]\n");
        }
        for ($j=0; $j < $cpu_inst; $j++, $k++) {
            $dev=cpu.$k;
#           print "dev: $dev\n";
            print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[11111111110000000000\]\n");
        }
        for ($j=0; $j < $mem_inst; $j++, $k++) {
            $dev=mem.$k;
#           print "dev: $dev\n";
            print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[11111111110000000000\]\n");
        }
        $i += $smt_threads;
    }
}

#####################################################################
#####   Function to create cfg file for exponential test case   #####
#####################################################################

sub create_cfg2() {
    create_pattern();
# max. no. of utilization seq. pattern created will be 32. If no. of cores are more than
# 32, we do grouping of cores.

if ( $no_of_cores >= 32) {
        for ($i=0; $i < 32; $i++) {
            @cores_per_set[$i] = int($no_of_cores/32);
        }
    }
    $remaining_cores = $no_of_cores%32;
    for ($i=0; $i < $remaining_cores; $i++) {
        @cores_per_set[$i]++;
    }

    $fpu_inst = $cpu_inst = $mem_inst = 0;
    $fpu_inst = int ( $smt_threads / 2);
    if ($fpu_inst == 0) {
        $fpu_inst = 1;
    } else {
        $cpu_inst = int ($fpu_inst / 2);
        if ($cpu_inst == 0) {
            $cpu_inst = 1;
        } else {
            $mem_inst = $cpu_inst;
        }
        $left = ($smt_threads) % ($fpu_inst + $cpu_inst + $mem_inst);
        while ($left != 0) {
            if ($mem_inst == 0 || $left % 2 == 0) {
                $mem_inst++;
            } else {
                $cpu_inst++;
            }
            $left--;
        }
    }

    $k = 0;
    $l = 0;
    for ($i=0; $i < 32 && $i < $no_of_cores; $i++, $k++) {
        $pattern_current = $pattern[$k];
        for ($j=0; $j < @cores_per_set[$i]; $j++) {
            for ($n=0, $m=$l; $n < $fpu_inst; $n++, $m++) {
                $dev=fpu.$m;
#               print "dev: $dev\n";
                print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[$pattern_current\]\n");
            }
            for ($n=0; $n < $cpu_inst; $n++, $m++) {
                $dev=cpu.$m;
#               print "dev: $dev\n";
                print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[$pattern_current\]\n");
            }
            for ($n=0; $n < $mem_inst; $n++, $m++) {
                $dev=mem.$m;
#               print "dev: $dev\n";
                print CFG_FILE ("\t$dev\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[$pattern_current\]\n");
            }
            $l += $smt_threads;
        }
    }
}

#####################################################################################
#####   Function to create utilization sequence pattern for config file     #####
#####################################################################################

# There will be 1 utilization sequence pattern for each core if the no. of cores are
# less than or equal to 32. Otherwise, we will create maximum 32 patterns.

sub create_pattern()
{
    $no_of_patterns = $no_of_cores;
    if ($no_of_patterns > 32) {
        $no_of_patterns = 32;
    }
    $pattern_length = 2 * $no_of_patterns;
    $picked=0;

    for ($i=0; $i < $no_of_patterns; $i++) {
        for ($k=0; $k< $pattern_length; $k++) {
            if ($k == $i) {
                $picked = 1;
                for ($j =0; $j < $no_of_patterns; $j++) {
                    @pattern[$i] = @pattern[$i].$picked;
#                   print "j: $j, k: $k, pattern: @pattern[$i]\n";
                }
            }
            $k = $k + $j;
            $j = 0;
            $picked = 0;
            @pattern[$i] = @pattern[$i].$picked;
#           print "k: $k, pattern: @pattern[$i]\n";
        }
#       print "pattern: @pattern[$i]\n";
    }
}
##############################################################################
########    Function to create cfg file for 100/75/50/25 scenario  ######
##############################################################################
sub create_cfg3() {
        $j=0;
   for ($i=0; ($i < $lcpus) && ($lcpus >= 4); $i=$i+4) {
      print CFG_FILE ("\tmem$j\t\t\tY\t\t\t\t\t\[100\]\t\t\t\t\[1111\]\n");
            $j++;
      print CFG_FILE ("\tcpu$j\t\t\tY\t\t\t\t\t\[75\]\t\t\t\t\[1110\]\n");
            $j++;
      print CFG_FILE ("\tcpu$j\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[1100\]\n");
            $j++;
      print CFG_FILE ("\tcpu$j\t\t\tY\t\t\t\t\t\[25\]\t\t\t\t\[1000\]\n");
            $j++;
   }
}
##############################################################################
########    Function to create cfg file for 100% cpu & 90% mem usage  ######
##############################################################################
sub create_cfg4() {
        $i=0;
   print CFG_FILE ("\tmem0\t\t\tY\t\t\t\t\t\[100\]\t\t\t\t\[1111\]\n");
   for ($i=1; ($i < $lcpus); $i=$i+1) {
      print CFG_FILE ("\tcpu$i\t\t\tY\t\t\t\t\t\[100\]\t\t\t\t\[1111\]\n");
   }
}
##############################################################################
########    Function to create cfg file for 50% cpu & 50% mem usage  ######
##############################################################################
sub create_cfg5() {
        $j=0;
   print CFG_FILE ("\tmem0\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[1010\]\n");
	$j++;
   print CFG_FILE ("\tcpu1\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[0101\]\n");
	$j++;
   for ($i=2; ($i < $lcpus) && ($lcpus >= 4); $i=$i+2) {
      print CFG_FILE ("\tcpu$j\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[1010\]\n");
            $j++;
      print CFG_FILE ("\tcpu$j\t\t\tY\t\t\t\t\t\[50\]\t\t\t\t\[0101\]\n");
            $j++;
   }
}
