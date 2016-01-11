#!/usr/bin/perl

# extracting value of daemon_log_wrap

        @profile_lines="";
        unless (open (PROFILE_FILE,"/usr/lpp/htx/.htx_profile")) {
                die ("Can't open .htx_profile file!\n");
        }

        @profile_lines=<PROFILE_FILE>;

        close(PROFILE_FILE);

        $size=100;

#print("\n size=$size");
        foreach $line (@profile_lines) {
                 if($line =~ /daemon_log_wrap/) {
                                if($line =~ /^\*/){
                                        print("star");
                                }
                                else{
                                        @value= split "\"", $line;
                                        $size = int ($value[1]);
                                }
                }
        }

#print("\n size=$size");


unless (open (TMP_OUT,">/etc/logrotate.d/stxround")) {
                die ("Can't open tmp_out file doing tmp_out file!\n");
        }

                print TMP_OUT ("/tmp/stx.log {\n");
                print TMP_OUT ("\t compress\n");
                print TMP_OUT ("\t rotate 10\n");
                print TMP_OUT ("\t notifempty\n");
                print TMP_OUT ("\t size +",$size,"k\n");
                print TMP_OUT ("\t postrotate\n");
                print TMP_OUT ("\t \t /etc/init.d/syslog reload\n");
              #  print TMP_OUT ("\t \t touch /tmp/stx.log\n");
                print TMP_OUT ("\t endscript \n");
                print TMP_OUT ("}\n");


        close(TMP_OUT);

