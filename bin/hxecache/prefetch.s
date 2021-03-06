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
# This module is called from prefetch_irritator.c 
# R3 - contains address of vector  
# R4 - Size of the vector that has to be prefetched 
# R5 - Pattern

# Declare the procedure descriptor for "prefetch"
# Start the "text" section
.section ".text"
.global prefetch
.type prefetch,@function

prefetch :
.set    size_of_cacheline,128

# Store the starting address for later use
    or 9 , 3 , 3

# Save the loop count to counter register
	mtctr 4							# Save the value of 4 in CTR register

# Now touch an address and hint the block to be na
prefetch_std :
	std 5, 0x0(3)					# Store a double word from GPR 5 pattern to GPR 3 
	addi 3, 3, size_of_cacheline	# Increment the target address by 128
	bdnz prefetch_std				# Decrement counter and jump to pf_dcbtna if ctr not zero

# Now restore the starting address into register 5
	or 3, 9 , 9
# Touch all the blocks again (this time for read) and compare with GPR 5
	mtctr 4							# Save the value of 4 in CTR register
prefetch_ld:
	ld 8, 0x0(3)					# Load a double word from GPR 3 to GPR 8
	cmpd 8, 5						# Compare GPR 8 and GPR 5 and set equal bit on condition register
	bne	pf_error
	addi 3,3,size_of_cacheline		# Increment the target address by 138
	bdnz prefetch_ld				# Decrement counter and jump to pf_dcbtna if ctr not zero

# If we reach here, it means everything is fine. Hence set return value as 0 and return
	li 3, 0							# Store 0 as return value ( in GPR 3 )
	b pf_exit						# Proceed towards exit 

# If we reach here it means error
pf_error:
	mfctr 3							# Store miscomparing cache line number as return value ( in GPR 3 )

# Exit from this sub routine
pf_exit:
	blr								# Return
