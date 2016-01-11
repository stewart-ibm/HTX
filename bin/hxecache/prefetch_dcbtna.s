# @(#)10       1.1  src/htx/usr/lpp/htx/bin/hxecache/prefetch_dcbtna.s, exer_cache, htxubuntu 11/7/12 18:31:26

# This module is called from prefetch_irritator.c 
# 3 - contains address of vector  
# 4 - Number of cache lines
# 5 - data to write ( thread index + 1 ).

# Declare the procedure descriptor for "prefetch"
# Start the "text" section
.section ".text"
.global prefetch_dcbtna
.type prefetch_dcbtna,@function

prefetch_dcbtna :
	.set    size_of_cacheline,128
	
# Store the starting address for later use
    or 9 , 3 , 3

# Save the loop count to counter register
	mtctr 4							# Save the value of 4 in CTR register

# Now touch an address and hint the block to be na
pf_dcbtna_std:
	.long 0x7e201a2c				# dcbt 0, r3, 0b10001
	std 5, 0x0(3)					# Store a double word from GPR 5 (index of thread) to GPR 3 
	addi 3, 3, size_of_cacheline	# Increment the target address by 138
	bdnz pf_dcbtna_std				# Decrement counter and jump to pf_dcbtna if ctr not zero

# Now restore the starting address into register 5
	or 3, 9 , 9
# Touch all the blocks again (this time for read) and compare with GPR 5
	mtctr 4							# Save the value of 4 in CTR register
pf_dcbtna_ld:
	.long 0x7e201a2c				# dcbt 0, r3, 0b10001
	ld 8, 0x0(3)					# Load a double word from GPR 3 to GPR 8
	cmpd 8, 5					# Compare GPR 8 and GPR 5 and set equal bit on condition register
	bne	pf_error
	addi 3,3,size_of_cacheline		# Increment the target address by 128
	bdnz pf_dcbtna_ld				# Decrement counter and jump to pf_dcbtna if ctr not zero

# If we reach here, it means everything is fine. Hence set return value as 0 and return
	li 3, 0						# Store 0 as return value ( in GPR 3 )
	b pf_exit						# Proceed towards exit 

# If we reach here it means error
pf_error:
	std 8, 0x0(6)					# Store the miscomparing address in 6
	std 5, 0x0(7)					# Store the pattern
	mfctr 3						# Store miscomparing cache line number as return value ( in GPR 3 )

# Exit from this sub routine
pf_exit:
	blr								# Return
