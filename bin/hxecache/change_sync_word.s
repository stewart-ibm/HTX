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
.section ".text"
.global change
.type change,@function
.align 2        
change:
.set SYNC_WORD,   3
.set CPU_ID_MASK, 4

# This function is used to turn off the id bit of the running thread in the
# Sync mask, by complementing the CPU_ID_MASK of the thread and then performing
# a logical AND with the SYNCWORD.

again:		                     # Stream synchronization code
        lwarx   0,0,SYNC_WORD        # Turn off my id bit in sync word
        andc    0,0,CPU_ID_MASK      # in an atomic manner
        stwcx.  0,0,SYNC_WORD	   	
        bne     again                # loop if reservation lost
        sync	
        isync
        blr                          # Return
