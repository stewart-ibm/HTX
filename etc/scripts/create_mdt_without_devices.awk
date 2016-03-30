#!/bin/awk -f
# IBM_PROLOG_BEGIN_TAG
# 
# Copyright 2003,2016 IBM International Business Machines Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# 		 http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# IBM_PROLOG_END_TAG


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
