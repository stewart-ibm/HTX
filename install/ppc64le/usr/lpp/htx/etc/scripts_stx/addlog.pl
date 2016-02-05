#!/usr/bin/perl
	
        $CMVC_RELEASE = `echo \$CMVC_RELEASE`;
        if ( $CMVC_RELEASE =~ /htxrhel6/ ) {
        	$fileToChange = "/etc/rsyslog.conf";
        } else {
        	$fileToChange = "/etc/syslog.conf";
        }
        @conf_lines="";
        unless (open (CONF_FILE,"$fileToChange")) {
                die ("Can't open $fileToChange file!\n");
        }

        @conf_lines=<CONF_FILE>;
        close(CONF_FILE);

        unless (open (TMP_OUT,">tmp_out")) {
                die ("Can't open tmp_out file doing tmp_out file!\n");
        }
        
	foreach $line (@conf_lines) {
                if($line =~ /local5/) {
			$skip = "y";
                        if($line =~ /^#/){
				$skip ="";
			}
                }
                if(!$skip){
			print TMP_OUT ("$line");
			
                }
		$skip = "";	
        }
        print TMP_OUT ("local5.*             /tmp/stx.log\n");
	
        close(TMP_OUT);
        $result = `cp tmp_out $fileToChange`;
        $result = `rm tmp_out`;

