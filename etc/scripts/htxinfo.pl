#!/usr/bin/perl

# @(#)93        1.14.5.2  src/htx/usr/lpp/htx/etc/scripts/htxinfo.pl, htxconf, htxubuntu 9/19/11 03:32:33

$linux_dist = "Unknown Linux distribution";

$kern_ver = `uname -r 2> /dev/null` or $kern_ver = "Unknown kernel version";

$linux_dist = `cat /etc/*-release 2> /dev/null |head -1` or $linux_dist = "Unknown Linux distribution";

$CMVC_RELEASE = `echo \$CMVC_RELEASE`;
if ( $CMVC_RELEASE =~ /htxcab/ || $CMVC_RELEASE =~ /htxltsbml/ || $CMVC_RELEASE =~ /htxcpbw/ ) {
    @arr_htx_ver = `cat /usr/lpp/htx/etc/version`;
} else {
    @arr_htx_ver = `(rpm -qa | grep ^htx) 2> /dev/null | grep htx |cut -d- -f1,2` or $htx_ver = "Unknown HTX Level";
}

$htx_ver=@arr_htx_ver[$#arr_htx_ver];

if( $linux_dist =~ /Fedora Core release 4.9/ )                   { $linux_dist = "FC5";    }
if( $linux_dist =~ /Fedora Core release 6/ )                     { $linux_dist = "FC6";    }
if( $linux_dist =~ /SUSE LINUX Enterprise Server 9/ )            { $linux_dist = "SLES9";  }
if( $linux_dist =~ /SUSE Linux Enterprise Server 10/ )           { $linux_dist = "SLES10"; }
if( $linux_dist =~ /SUSE Linux Enterprise Server 11/ )           { $linux_dist = "SLES11"; }
if( $linux_dist =~ /Red Hat Enterprise Linux AS release 4/ )     { $linux_dist = "RHEL4";  }
if( $linux_dist =~ /Red Hat Enterprise Linux Server release 5/ ) { $linux_dist = "RHEL5";  }
if( $linux_dist =~ /Red Hat Enterprise Linux Server release 6/)  { $linux_dist = "RHEL6";  }

chomp ($kern_ver);
chomp ($htx_ver);
chomp ($linux_dist);

$out = sprintf(" %s    %s    Kernel: %s", $htx_ver, $linux_dist, $kern_ver);

print $out;
