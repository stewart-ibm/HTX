#!/bin/awk -f

# @(#)03        1.2  src/htx/usr/lpp/htx/etc/scripts/create_mdt_without_devices.awk, htxconf, htxubuntu 12/10/08 04:13:35

  BEGIN {
      t=1
      for(count=0; count<ARGC-1; count++){
          devices[count] = ARGV[count+1]
      }
    # Set ARGC to 1 to avoid awk interpreting passed device name arguments as files to workon.
      ARGC = 1
  }

  ($1 ~ /:/) {
      for(i=0; i<count; i++) {
          dev="";
	  dev_class=""; 
           if(devices[i] ~ /[0-9]$/) { 
       	    dev=sprintf("%s:",devices[i]);
            if(dev == $1) { t=0;break}
	   }else {
           dev_class=sprintf("^%s[0-9]*:",devices[i]);
           if($1 ~ dev_class) {t=0;break}
           }
      }
     if(i == count) { t=1 }
  }
(t==1) {print}
