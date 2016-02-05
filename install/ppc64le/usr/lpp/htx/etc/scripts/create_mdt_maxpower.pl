#!/usr/bin/perl

# @(#)39	1.6  src/htx/usr/lpp/htx/etc/scripts/create_mdt_maxpower.pl, htxconf, htxubuntu 5/14/15 04:08:42

# This script is intended to create a mdt with a workload to consume maximum
# power. The type of workload is such that all the cores will consume maximum
# power. Below is the type of workload which will be used:
# 50% of threads of a core will be running hxefpu64 with VSX workload.
# 25% of threads of a core will be running hxecpu with fixed point type workload.
# 25% of threads of a core will be running hxemem64 to have cache and memory stress.

# Note: The rule files associated with each of these workload is not suitable for
# verification purpose. They are only intended to generate max. power consumption.

############################################################
########            Main starts here               #########
############################################################

# Get the no. of cores available on the system
my $no_of_cores = `cat /tmp/htx_syscfg | grep "Number of Cores" | awk '{print \$5}'`;
chomp($no_of_cores);
#printf("no_of_cores: %s\n", $no_of_cores);

# Determine SMT threads per processor
$lcpus = `grep processor /proc/cpuinfo | wc -l`;
$pcpus = `ls -l /proc/device-tree/cpus | grep ^d | awk '(\$NF ~ /POWER/)' | wc -l`;
$smt_threads = int ($lcpus / $pcpus);
if ($smt_threads == 0) {
	$smt_threads = 1;
}

# Get the default stanza
$output = `create_my_mdt default`;
#$output = `cat /usr/lpp/htx/mdt/mdt.all | grep -p "default:"` ;
print $output;

# For each core, find the no. of threads and type of workload accordingly and then create stanza for
# each thread.
$fpu_inst = $cpu_inst = $mem_inst = 0;
$fpu_inst = int ($smt_threads / 2);
if ($fpu_inst == 0) {
    $fpu_inst = 1;
} else {
    $cpu_inst = int ($fpu_inst / 2);
    if ($cpu_inst == 0) {
        $cpu_inst = 1;
    } else {
        $mem_inst = $cpu_inst;
    }
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


# Changes for feature#961679. Replacing individual instances for fpu and cpu with
# cpu only running with combo rulefile (which covers both fpu n cpu instructions)
#
# Hack : rather than changing dev number calculation logic, just use both combined
# (cpu_inst+fpu_inst) for cpu device creation. 

$cpu_inst = int ($fpu_inst + $cpu_inst);


$k = 0;
for ($i=0; $i < $lcpus;) {
    for ($j=0; $j < $cpu_inst; $j++, $k++) {
        print "cpu$k:\n";
        create_stanza("hxecpu", "core", "fixed_point", "hxefpu64/fpu_cpu_combo.p8", "NO");
    }
    for ($j=0; $j < $mem_inst; $j++, $k++) {
        print "mem$k:\n";
        # Check default pagesize it can either 4k or 64k. If default_pagesize is 64k
        # we choose "default.mem.eq.64k" else we choose "default.mem.eq.4k".
        $default_pagesize = `getconf PAGESIZE`;
        if ($default_pagesize == 65536) {
        	$rule_file_name = "hxemem64/default.mem.eq.64k";
        } else {
        	$rule_file_name = "hxemem64/default.mem.eq.4k";
        }
        create_stanza("hxemem64", "nest", "memory", $rule_file_name, "NO");
    }
    $i += $smt_threads;
}

####################################################
##############     Main ends here   ############
####################################################

##################################################################
#####   Function to create exerciser stanza in mdt file    #######
##################################################################
sub create_stanza()
{
    $HE_name_len = 8 + 7 + 3 + length($_[0]) + 2 - 50;
    printf("\tHE_name = \"%s\" %${HE_name_len}s *Hardware Exerciser name, 14 char\n", $_[0], " ");
    $adapt_desc_len = 8 + 10 + 3 + length($_[1]) + 2 - 50;
    printf("\tadapt_desc = \"%s\" %${adapt_desc_len}s *adapter description, 11 char max.\n", $_[1], " ");
    $device_desc_len = 8 + 11 + 3 + length($_[2]) + 2 - 50;
    printf("\tdevice_desc = \"%s\" %${device_desc_len}s *device description, 15 char max.\n", $_[2], " ");
    $reg_rules_len = 8 + 9 + 3 + length($_[3]) + 2 - 50;
    printf("\treg_rules = \"%s\" %${reg_rules_len}s *reg rule file\n", $_[3], " ");
    printf("\temc_rules = \"%s\" %${reg_rules_len}s *emc rule file\n", $_[3], " ");
    if ($_[4] eq "NO") {
        $cont_on_err_len = 8 + 11 + 3 + length($_[4]) + 2 -50;
        printf("\tcont_on_err = \"%s\" %${cont_on_err_len}s *continue on error (YES/NO)\n", $_[4], " ");
    }
    print "\n";
}
