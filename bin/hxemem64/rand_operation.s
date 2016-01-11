##############################################################################
#
#  FUNCTION: rand_operation
#
#  DESCRIPTION:  stores & Compares buffer1 with 8 byte bit pattern
#
#  CALLING SEQUENCE: rand_operation(code,counter,dest_ptr,pattern_ptr,trap_flag,\
#                    segment_detail,stanza_ptr)
#
#  CONVENTIONS:
#                GPR3 - code
#                GPR4 - counter
#                GPR5 - destination pointer
#                GPR6 - pattern pointer
#                GPR7 - trap_flag
#                GPR8 - segment detail
#                GPR9 - stanza pointer
#                GPR10 - seed's Address
#
#
##############################################################################
    .file   "rand_operation.c"
    .section    ".text"
#    .align 2
#    .section    ".opd","aw"
    .align 3
    .globl rand_operation
rand_operation:
#    .quad   .rand_operation,.TOC.@tocbase,0
#    .previous
#    .size   rand_operation,24
    .type   .rand_operation,@function
    .globl  .rand_operation
.rand_operation:

    .set            r1,1
    .set            r2,2
    .set            r3,3
    .set            r4,4
    .set            r5,5
    .set            r6,6
    .set            r7,7
    .set            r8,8
    .set            r9,9
	.set			r10,10


    cmpi     1, 1, r4, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, no_trap                # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.
    
    # Save to Stack all the parameters passed to the routine 
    std        r4,-8(r1)        
    std        r5,-16(r1)        
    std        r6,-24(r1)        
    std        r7,-32(r1)        
    std        r8,-40(r1)        
    std        r9,-48(r1)        
    std        r10,-56(r1)        

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r4
    
    #move Starting Address value from r5 to r7
    or      r7,r5,r5   # r7 = r5 

	or 		r9,r10,r10   	#r9 = r10  --Put the seed's address in r9 
    ld		r10,0(r10) 		# Have the seed value in r10 

# We need mul_val and add_val constants for calculating the random number.
# We are loading 64 bit mul_val into r4
# 64 bit add_val into r5
# Random number is calculated like this:
# random_num = seed x mul_val
# random_num = random_num + add_val

    lis     r4,0x0019
    ori     r4,r4,0x660D
    li      r6,0x20
    rldcr   r4,r4,r6,0x3F
    oris    r4,r4,0x3490
    ori     r4,r4,0x660D
    lis     r5,0x0019
    ori     r5,r5,0x4791
    li      r6,0x20
    rldcr   r5,r5,r6,0x3F
    oris    r5,r5,0x3C6E
    ori     r5,r5,0xF35F


mem_ran_dword:
	cmpi    1,1, r3, 0x0001
    bne     cr1,mem_ran_word			# if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-8                    # Store buffer
ran_mem_dword:
    addi    r7,r7,8                     # Store Buffer incremented
	mulld	r10,r4,r10					# multiple r4 and r10 and store r10
	add		r10,r10,r5					# rand num(r10) = r10+r5 
    std     r10,0(r7)                   # Store 8 bytes to buffer
    bc      16, 0, ran_mem_dword	    # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

mem_ran_word:
    cmpi    1,1, r3, 0x0002
    bne     cr1,mem_ran_byte            # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-4                    # Store buffer
ran_mem_word:
    addi    r7,r7,4                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    stw     r10,0(r7)                   # Store 8 bytes to buffer
    bc      16, 0, ran_mem_word         # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

mem_ran_byte:
    cmpi    1,1, r3, 0x0003
    bne     cr1,rim_ran_dword          # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-1                    # Store buffer
ran_mem_byte:
    addi    r7,r7,1                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    stb     r10,0(r7)                   # Store 1 byte to buffer
    bc      16, 0, ran_mem_byte         # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

rim_ran_dword:
    cmpi    1,1, r3, 0x0004
    bne     cr1,rim_ran_word          # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-8                    # Store buffer
ran_rim_dword:
    addi    r7,r7,8                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    std     r10,0(r7)                   # Store 8 bytes to buffer
	dcbf	0,r7
    ld      r6,0(r7)                    # Load 8 bytes to compare
    cmpld   1, r6,r10                   # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_rim_dword       # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

rim_ran_word:
    cmpi    1,1, r3, 0x0005
    bne     cr1,rim_ran_byte           # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-4                    # Store buffer
ran_rim_word:
    addi    r7,r7,4                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    stw     r10,0(r7)                   # Store 4 bytes to buffer
	dcbf	0,r7
    lwz     r6,0(r7)                    # Load 4 bytes to buffer
    cmplw   1, r6,r10                   # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_rim_word       # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

rim_ran_byte:
    cmpi    1,1, r3, 0x0006
    bne     cr1,comp_ran_dword	        # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-1                    # Store buffer
	addi	r8,0,0x00FF
ran_rim_byte:
    addi    r7,r7,1                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    stb     r10,0(r7)
	dcbf	0,r7
    lbz     r6,0(r7)
    and     r3,r10,r8
    cmplw   1, r6,r3                   # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_rim_byte        # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0
    
comp_ran_dword:
    cmpi    1,1, r3, 0x0007
    bne     cr1,comp_ran_word          # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-8                    # Store buffer
ran_comp_dword:
    addi    r7,r7,8                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    ld		r6,0(r7)                   	# Store 8 bytes to buffer
    cmpld   1, r6,r10                   # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_comp_dword		# Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_ran_word:
    cmpi    1,1, r3, 0x0008
    bne     cr1,comp_ran_byte           # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-4                    # Store buffer
ran_comp_word:
    addi    r7,r7,4                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
    lwz     r6,0(r7)                    # Store 8 bytes to buffer
    cmplw   1, r6,r10                   # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_comp_word       # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_ran_byte:
    cmpi    1,1, r3, 0x0009
    bne     cr1,error_ret	        # if (r3 <code> == 13 (0xd) ) then do MEM operation width = 8
    addi    r7,r7,-1                    # Store buffer
	addi	r8,0,0x00FF
ran_comp_byte:
    addi    r7,r7,1                     # Store Buffer incremented
    mulld   r10,r4,r10                  # multiple r4 and r10 and store r10
    add     r10,r10,r5                  # rand num(r10) = r10+r5
	lbz		r6,0(r7)	
	and		r3,r10,r8
    cmplw   1, r6,r3                    # Compare 4 bytes
    bne     cr1,error_ret               # branch if r5 != r8
    bc      16, 0, ran_comp_byte        # Loop until CTR value is 0
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret:
	std		r10,0(r9)					# Put the current seed value(r10) in the seed location(r9) 
    mfctr   r3
    ld      r4,-8(r1)        
    sub     r4,r4,r3                    # r4 represents the dword/word/byte that had miscompare
    addi    r4,r4,1                     # Increment r4 by 1, to cover the case where miscompare is in 0th position.
    ld      r3,-32(r1)                # load trap flag into r3 from the stack
    cmpi    1,0,r3,0x0000
    bc      0xC,6,no_trap                
    cmpi    1,0,r3,0x0001               # If bit 6 in CR is zero, it means trap_flag=0. Go to no_trap
    bc      0xC,6,trap_kdb              # If bit 6 in CR is one, it means trap_flag=1. Go to trap_kdb    
    .long     0x200
trap_kdb:
    ld        r5,-16(r1)                # load shm starting pointer into r5 from the stack    
    ld        r6,-24(r1)                # load pattern buffer pointer into r6 from the stack    
    ld        r8,-40(r1)                # load structure pointer into r8 from the stack    
    ld        r9,-48(r1)                # load stanza pointer into r9 from the stack    
    addis     r3,0,0xBEEF               # load 0xBEAF into 32-47 bits in r3
    ori       r3,r3,0xDEAD              # load 0xDEAD into 48-63 bits in r3
    tw        4,r1,r1                   # Enter KDB
no_trap:
    xor     r3,r3,r3
    or      r3,r3,r4                 # r3 = r4 (no of dword/word/byte compared)
    bclr    20, 0

#    .long 0
#    .byte 0,0,0,0,128,1,0,1
#    .size   .rand_operation,.-.rand_operation

