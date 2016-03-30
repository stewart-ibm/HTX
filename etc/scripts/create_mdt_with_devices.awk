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
