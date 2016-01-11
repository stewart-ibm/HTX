# @(#)58	1.1  src/htx/usr/lpp/htx/bin/hxemem64/fill.s, exer_mem64, htxubuntu 7/4/02 00:25:59
	gcc2_compiled.:
	.section		".text"
	.align		2
	.globl		fill_byte_linux
fill_byte_linux:

		.set		rf3,3
		.set		r3,3
		.set		r4,4
		.set		r5,5
		.set		r6,6


    cmpi     1, 0, r5, 0x0000           # Compare numbytes to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_byte       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of byte transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9, r5

    addic   r3, r3, -1
    lbz     r6, 0(r4)                  # r6 contains byte to be filled

more_bytes:

    stbu    r6, 1(r3)                  # store same byte
    bc      16, 0, more_bytes          # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret_byte:

    addi    r3, 0, -1
    bclr    20, 0


##############################################################################
##############################################################################
##############################################################################
##############################################################################



	.section        ".text"
	.align          2
	.globl  fill_word_linux
fill_word_linux:

    .set rf3,3
	.set	r3,3
	.set	r4,4
	.set	r5,5
	.set	r6,6

    cmpi     1, 0, r5, 0x0000           # Compare numwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_word       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of word transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9, r5

    addic   r3, r3, -4
    lwz     r6, 0(r4)                  # Load 4 bytes into r6

more_words:

    stwu    r6, 4(r3)                  # Store same 4 bytes at destination
    bc      16, 0, more_words          # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret_word:

    addi    r3, 0, -1
    bclr    20, 0



##############################################################################
##############################################################################
##############################################################################
##############################################################################


	.section        ".text"
	.align          2
	.globl  fill_dword_linux
fill_dword_linux:

    .set rf3,3
	.set	r3,3
	.set	r4,4
	.set	r5,5
	.set	r6,6

    cmpi     1, 0, r5, 0x0000           # Compare numdwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_dword      # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of word transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9, r5

    addic   r3, r3, -8
    lfd     rf3, 0(r4)                  # Load 8 bytes into fp3

more_dwords:

    stfdu   rf3, 8(r3)                  # Store same 8 bytes at destination
    bc      16, 0, more_dwords          # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret_dword:

    addi    r3, 0, -1
    bclr    20, 0
