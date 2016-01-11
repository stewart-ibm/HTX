# @(#)19        1.2  src/htx/usr/lpp/htx/bin/hxecache/prefetch_dcbtt.s, exer_cache, htx61D 8/10/08 23:13:36
# IBM_PROLOG_BEGIN_TAG 
# This is an automatically generated prolog. 
#  
# htxltsbml src/htx/usr/lpp/htx/bin/hxecache/prefetch_dcbtt.s 1.1 
#  
# Licensed Materials - Property of IBM 
#  
# COPYRIGHT International Business Machines Corp. 2010 
# All Rights Reserved 
#  
# US Government Users Restricted Rights - Use, duplication or 
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp. 
#  
# IBM_PROLOG_END_TAG 

# This module is called from prefetch_irritator.c 
# 3 - contains address of vector  
# 4 - Size of the vector that has to be prefetched 
# 5 - Seed 
# 6 - Address of the thread specific data area 

# Layout of the thread specific data area 
#
# start_addr :  .long  0 , 0
# end_addr :    .long  0 , 0
# seed :        .long  0 , 0
# direction :   .long 0x0

# Declare the procedure descriptor for "transient_dcbt"
# Start the "text" section
.section ".text"
.global transient_dcbt
.type transient_dcbt,@function
	
transient_dcbt :
.set    size_of_cacheline,128

# Store the starting address for later use
    or 9 , 3 , 3

# Save the loop count to counter register
	mtctr 4							# Save the value of 4 in CTR register

# Now touch an address and hint the block to be na
transient_dcbt_std :
	.long 0x7e001a2c				# dcbt 0, r3, 0b10000
	std 5, 0x0(3)					# Store a double word from GPR 5 (index of thread) to GPR 3 
	addi 3, 3, size_of_cacheline	# Increment the target address by 138
	bdnz transient_dcbt_std				# Decrement counter and jump to pf_dcbtna if ctr not zero

# Now restore the starting address into register 5
	or 3, 9 , 9
# Touch all the blocks again (this time for read) and compare with GPR 5
	mtctr 4							# Save the value of 4 in CTR register
transient_dcbt_ld:
	.long 0x7e001a2c				# dcbt 0, r3, 0b10000
	ld 8, 0x0(3)					# Load a double word from GPR 3 to GPR 8
	cmpd 8, 5					# Compare GPR 8 and GPR 5 and set equal bit on condition register
	bne	pf_error
	addi 3,3,size_of_cacheline		# Increment the target address by 138
	bdnz transient_dcbt_ld			# Decrement counter and jump to pf_dcbtna if ctr not zero

# If we reach here, it means everything is fine. Hence set return value as 0 and return
	li 3, 0							# Store 0 as return value ( in GPR 3 )
	b pf_exit						# Proceed towards exit 

# If we reach here it means error
pf_error:
	mfctr 3							# Store miscomparing cache line number as return value ( in GPR 3 )

# Exit from this sub routine
pf_exit:
	blr								# Return




