#!/usr/bin/perl

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

$para_start = 0;
$do_once = 1;

#print "$ARGV[0]";
#print "opening file\n";
open ( FILEP, "/tmp/htxconf.op");

while( $line = <FILEP>) {
   #print "$line";
   #print "end line";
   if ( $do_once ) {
      if ( $line =~ s/://) {
           #printf "in :\n ******* $line ****\n";
           chomp($line);
           #printf "***$line***\n";
        if ($line =~ $ARGV[0] ) {
            $para_start = 1;
            print "$line:\n";
        } else  {
            if ( $para_start ) {
               $do_once = 0;
               #print "para end";
            }
            $para_start = 0;
        }
      } else {
         if ( $para_start ) {
            print $line;
            #print "inside loop ";
         }
      }
    }

}

