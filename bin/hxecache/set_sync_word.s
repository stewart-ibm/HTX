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
.global  set_sync_word
.type set_sync_word,@function
.align 2        
set_sync_word:
.set SYNC_WORD,    3
.set SHIFT_COUNT,  4

# This function resets the bits of the sync word to all 1s

wait_loop:                         
          li 6,0x2                # Load the value 2 in register 6
          lwarx 5, 0, SYNC_WORD   # Load and reserve the Sync_word
          sld 5,6,SHIFT_COUNT     # Shift left the value in reg 6 by
                                  # value in reg 7 and store in reg 5
          addi 5,5,-0x1		  # subtract 1 from value in reg 5
                                  # i.e R5 = (R5 << R6)-1
          stwcx. 5, 0, SYNC_WORD  # Store the sync word
          bne wait_loop           # loop if reservation lost
          dcbf 0,SYNC_WORD
          sync
          icbi 0,SYNC_WORD
          isync
          blr                     # Return
