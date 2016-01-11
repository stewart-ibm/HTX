#!/bin/awk -f

# @(#)02        1.2  src/htx/usr/lpp/htx/etc/scripts/create_mdt_with_devices.awk, htxconf, htxubuntu 12/10/08 04:14:44

 BEGIN {
      t=0
      devices[0] = "default"
      for(count=1; count<ARGC; count++) {
          devices[count] = ARGV[count]
      }
    # Set ARGC to 1 to avoid awk interpreting passed device name arguments as files to workon.
      ARGC = 1
  }

  ($1 ~ /:/) {
      if(t==1) {t=0}
      for(i=0; i<count; i++) {
      dev="";
      dev_class="";
           if(devices[i] ~ /[0-9]$/) { 
             dev=sprintf("%s:",devices[i]);
             if(dev == $1) {print; t=1;break}
           } else {
             dev_class=sprintf("^%s[0-9]*:",devices[i]);
             if($1 ~ dev_class) {print; t=1;break}
           }
      }
      if(t==1) {next}
  }
(t==1) {print}
