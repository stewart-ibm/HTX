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
# @(#)57	1.1  src/htx/usr/lpp/htx/bin/hxemem64/comp_linux.s, exer_mem64, htxubuntu 7/4/02 00:24:20
gcc2_compiled.:
	.section        ".text"
	.align          2
	.globl          comp_dword_linux
	.type           comp_dword_linux,@function
	comp_dword_linux:
#

		.set    r0,0
		.set    r1,1
		.set    r2,2
		.set    r3,3
		.set    r4,4
		.set    r5,5
		.set    r6,6
		.set    r7,7
		.set    r8,8
		.set    r9,9
		.set    r10,10
		.set    r11,11
		.set    r12,12
		.set    r13,13
		.set    r14,14
		.set    r15,15
		.set    r16,16
		.set    r17,17
		.set    r18,18
		.set    r19,19
		.set    r20,20
		.set    r21,21
		.set    r22,22
		.set    r23,23
		.set    r24,24
		.set    r25,25
		.set    r26,26
		.set    r27,27
		.set    r28,28
		.set    r29,29
		.set    r30,30
		.set    r31,31

    cmpi     1, 0, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret            # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

	 lis		r8,gpr20@ha
 	 stw     r20,gpr20@l(r8) 				 # Save orginal value of r20 in gpr20
	 lis		r20,gpr6@ha						 # use r20
	 stw		r6, gpr6@l(r20)				 # Save the trap_flag at memory gpr6
    lwz		r6, 0(r3)                   # Load 4 bytes from buffer1
    lwz		r7, 0(r4)                   # Load 4 byte bit pattern
	 lis		r20,gpr3@ha
    stw     r3, gpr3@l(r20)
    lwz		r9, gpr3@l(r20)
    addi    r9,r9,-4            	# Index buffer1
    xor     r3,r3,r3

more_dwords:
    addi    r9,r9,4             	# Index buffer1
    lwz		r6,0(r9)            	# Load 8 bytes from buffer1
    cmp		1,  r6,r7           	# Compare 8 bytes
    bne     cr1,error_ret       	# branch if r6 != r7
    bc      16, 0, more_dwords	# Loop until CTR value is 0

    addi    r3, 0, 0			# Set GPR-3 to zero for return
    bclr    20, 0

error_ret:
    mfctr   r8
	 sub		r3,r5,r8			 # r3 represents the double word that had miscompare
	 addi		r3,r3,1			 # Increment r3 by 1, to cover the case where miscompare is in 0th word.
	 lis		r20,gpr13@ha
 	 stw     r13,gpr13@l(r20) # Save orginal value of r13 in gpr13
	 lis		r20,gpr6@ha
	 lwz		r13,gpr6@l(r20)			 # Get the trap_flag from memory to r13
	 cmpi		1,0,r13,0x0000	 # Compare r13 with 0 and load results in CR1

no_trap:
	 lis		r20,gpr8@ha
	 stw		r8,gpr8@l(r20)			# save r8 in gpr8
	 lis		r20,gpr13@ha
	 lwz		r13,gpr13@l(r20)		# restore r13 from gpr13
	 lis		r8,gpr20@ha
	 lwz		r20,gpr20@l(r8)		# restore r20 from gpr20

	 lis		r8,gpr0@ha
    stw     r0, gpr0@l(r8)
	 lis		r8,gpr1@ha
    stw     r1, gpr1@l(r8)
	 lis		r8,gpr2@ha
    stw     r2, gpr2@l(r8)
    #stw     r3, gpr3				# need not store gpr3.now gpr3 has address (shr_memp)@l(r8)
	 lis		r8,gpr4@ha
    stw     r4, gpr4@l(r8)
	 lis		r8,gpr5@ha
    stw     r5, gpr5@l(r8)

#	 lis		r8,gpr6@ha
#    stw     r6, gpr6@l(r8)	# Already saved

	 lis		r8,gpr7@ha
    stw     r7, gpr7@l(r8)

#	 lis		r8,gpr8@ha
#    stw     r8, gpr8@l(r8)	# Already saved

	 lis		r8,gpr9@ha
    stw     r9, gpr9@l(r8)
	 lis		r8,gpr10@ha
    stw     r10, gpr10@l(r8)
	 lis		r8,gpr11@ha
    stw     r11, gpr11@l(r8)
	 lis		r8,gpr12@ha
    stw     r12, gpr12@l(r8)
	 lis		r8,gpr13@ha
    #stw     r13, gpr13					# Already stored
	 lis		r8,gpr14@ha
    stw     r14, gpr14@l(r8)
	 lis		r8,gpr15@ha
    stw     r15, gpr15@l(r8)
	 lis		r8,gpr16@ha
    stw     r16, gpr16@l(r8)
	 lis		r8,gpr17@ha
    stw     r17, gpr17@l(r8)
	 lis		r8,gpr18@ha
    stw     r18, gpr18@l(r8)
	 lis		r8,gpr19@ha
    stw     r19, gpr19@l(r8)
	 lis		r8,gpr20@ha
    stw     r20, gpr20@l(r8)
	 lis		r8,gpr21@ha
    stw     r21, gpr21@l(r8)
	 lis		r8,gpr22@ha
    stw     r22, gpr22@l(r8)
	 lis		r8,gpr23@ha
    stw     r23, gpr23@l(r8)
	 lis		r8,gpr24@ha
    stw     r24, gpr24@l(r8)
	 lis		r8,gpr25@ha
    stw     r25, gpr25@l(r8)
	 lis		r8,gpr26@ha
    stw     r26, gpr26@l(r8)
	 lis		r8,gpr27@ha
    stw     r27, gpr27@l(r8)
	 lis		r8,gpr28@ha
    stw     r28, gpr28@l(r8)
	 lis		r8,gpr29@ha
    stw     r29, gpr29@l(r8)
	 lis		r8,gpr30@ha
    stw     r30, gpr30@l(r8)
	 lis		r8,gpr31@ha
    stw     r31, gpr31@l(r8)
#   addi    r3, r3, -1
    bclr    20, 0

##############################################################################
##############################################################################
##############################################################################
##############################################################################

	.section		".text"
	.align		2
	.globl		wr_cmp_dword_linux
	.type       wr_cmp_dword_linux,@function
wr_cmp_dword_linux:

	.set	r0,0
	.set	r1,1
	.set	r2,2
	.set	r3,3
	.set	r4,4
	.set	r5,5
	.set	r6,6
	.set	r7,7
	.set	r8,8
	.set	r9,9
	.set	r10,10
	.set	r11,11
	.set	r12,12
	.set	r13,13
	.set	r14,14
	.set	r15,15
	.set	r16,16
	.set	r17,17
	.set	r18,18
	.set	r19,19
	.set	r20,20
	.set	r21,21
	.set	r22,22
	.set	r23,23
	.set	r24,24
	.set	r25,25
	.set	r26,26
	.set	r27,27
	.set	r28,28
	.set	r29,29
	.set	r30,30
	.set	r31,31

#

    cmpi     1, 0, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_wr            # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

    lwz      r6, 0(r3)                   # Load 8 bytes from buffer1
    lwz      r7, 0(r4)                   # Load 8 byte bit pattern
	 lis		r20,gpr3@ha
    stw     r3, gpr3@l(r20)
    lwz      r9, gpr3@l(r20)
    addi    r9,r9,-4                    # Index buffer1
    addi    r3, 0, 0                    # Set GPR-3 to zero for dcbf
more_cmp_dwords:
    addi    r9,r9,4                     # Index buffer1
	stw     r7,0(r9)                    # Store 8 bytes to buffer

	dcbf    r3, r9                      #dcbf 

    lwz      r6,0(r9)                    # Load 8 bytes from buffer1
    cmp    1,  r6,r7                   # Compare 8 bytes
    bne     cr1,error_ret_wr               # branch if r6 != r7
    bc      16, 0, more_cmp_dwords           # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret_wr:
    mfctr   r8
	 lis		r20,gpr8@ha
 	 stw		r8,gpr8@l(r20)			# Save r8.
	 lis		r8,gpr20@ha
	 lwz		r20,gpr20@l(r8)		# Restore r20
	
	 lis		r8,gpr0@ha
    stw     r0, gpr0@l(r8)
	 lis		r8,gpr1@ha
    stw     r1, gpr1@l(r8)
	 lis		r8,gpr2@ha
    stw     r2, gpr2@l(r8)
	 lis		r8,gpr3@ha
    stw     r3, gpr3@l(r8)
	 lis		r8,gpr4@ha
    stw     r4, gpr4@l(r8)
	 lis		r8,gpr5@ha
    stw     r5, gpr5@l(r8)
	 lis		r8,gpr6@ha
    stw     r6, gpr6@l(r8)
	 lis		r8,gpr7@ha
    stw     r7, gpr7@l(r8)
	 lis		r8,gpr8@ha
    stw     r8, gpr8@l(r8)
	 lis		r8,gpr9@ha
    stw     r9, gpr9@l(r8)
	 lis		r8,gpr10@ha
    stw     r10, gpr10@l(r8)
	 lis		r8,gpr11@ha
    stw     r11, gpr11@l(r8)
	 lis		r8,gpr12@ha
    stw     r12, gpr12@l(r8)
	 lis		r8,gpr13@ha
    stw     r13, gpr13@l(r8)
	 lis		r8,gpr14@ha
    stw     r14, gpr14@l(r8)
	 lis		r8,gpr15@ha
    stw     r15, gpr15@l(r8)
	 lis		r8,gpr16@ha
    stw     r16, gpr16@l(r8)
	 lis		r8,gpr17@ha
    stw     r17, gpr17@l(r8)
	 lis		r8,gpr18@ha
    stw     r18, gpr18@l(r8)
	 lis		r8,gpr19@ha
    stw     r19, gpr19@l(r8)
	 lis		r8,gpr20@ha
    stw     r20, gpr20@l(r8)
	 lis		r8,gpr21@ha
    stw     r21, gpr21@l(r8)
	 lis		r8,gpr22@ha
    stw     r22, gpr22@l(r8)
	 lis		r8,gpr23@ha
    stw     r23, gpr23@l(r8)
	 lis		r8,gpr24@ha
    stw     r24, gpr24@l(r8)
	 lis		r8,gpr25@ha
    stw     r25, gpr25@l(r8)
	 lis		r8,gpr26@ha
    stw     r26, gpr26@l(r8)
	 lis		r8,gpr27@ha
    stw     r27, gpr27@l(r8)
	 lis		r8,gpr28@ha
    stw     r28, gpr28@l(r8)
	 lis		r8,gpr29@ha
    stw     r29, gpr29@l(r8)
	 lis		r8,gpr30@ha
    stw     r30, gpr30@l(r8)
	 lis		r8,gpr31@ha
    stw     r31, gpr31@l(r8)
    addi    r3, 0, -1
    bclr    20, 0

		.section	".data"
        .globl  Gprs1,Gprs
_wr_cmp_dword:
Gprs:
Gprs1:
gpr0:   .long   0x00000000
gpr1:   .long   0x00000000
gpr2:   .long   0x00000000
gpr3:   .long   0x00000000
gpr4:   .long   0x00000000
gpr5:   .long   0x00000000
gpr6:   .long   0x00000000
gpr7:   .long   0x00000000
gpr8:   .long   0x00000000
gpr9:   .long   0x00000000
gpr10:  .long   0x00000000
gpr11:  .long   0x00000000
gpr12:  .long   0x00000000
gpr13:  .long   0x00000000
gpr14:  .long   0x00000000
gpr15:  .long   0x00000000
gpr16:  .long   0x00000000
gpr17:  .long   0x00000000
gpr18:  .long   0x00000000
gpr19:  .long   0x00000000
gpr20:  .long   0x00000000
gpr21:  .long   0x00000000
gpr22:  .long   0x00000000
gpr23:  .long   0x00000000
gpr24:  .long   0x00000000
gpr25:  .long   0x00000000
gpr26:  .long   0x00000000
gpr27:  .long   0x00000000
gpr28:  .long   0x00000000
gpr29:  .long   0x00000000
gpr30:  .long   0x00000000
gpr31:  .long   0x00000000
