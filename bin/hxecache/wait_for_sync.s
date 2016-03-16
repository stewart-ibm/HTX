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
.global  wait_for
.type wait_for,@function
.align 2        
wait_for:
.set SYNC_WORD,  3

# This function checks and waits till the sync word has become zero

wait_loop:                   
                            
        lwz 4, 0(SYNC_WORD) # Load the word whose address is in reg 3 into reg 4
        cmpwi 4, 0x0	    # Compare it with 0
        bne wait_loop	    # If not zero then loop
        blr                 # Return
