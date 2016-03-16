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
# @(#)98	1.3  src/htx/usr/lpp/htx/bin/hxemem64/fill64.s, exer_mem64, htxubuntu 4/20/07 07:15:57
#########################################################################
	.file	"fill64.c"
	.section	".text"
	.align 2
	.globl fill_byte
	.section	".opd","aw"
	.align 3
fill_byte:
	.quad	.fill_byte,.TOC.@tocbase,0
	.previous
	.size	fill_byte,24
	.type	.fill_byte,@function
	.globl	.fill_byte
.fill_byte:
	std 31,-8(1)
	stdu 1,-64(1)
	mr 31,1
	std 3,112(31)
	std 4,120(31)
	std 5,128(31)

    .set            rf3,3
    .set            r3,3
    .set            r4,4
    .set            r5,5
    .set            r6,6

    cmpi     1, 1, r5, 0x0000           # Compare numbytes to 0 and place
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
	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr

error_ret_byte:

    addi    r3, 0, -1
    bclr    20, 0

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,0,0,0,128,1,0,1
	.size	.fill_byte,.-.fill_byte

	.align 2
	.globl fill_word
	.section	".opd","aw"
	.align 3
fill_word:
	.quad	.fill_word,.TOC.@tocbase,0
	.previous
	.size	fill_word,24
	.type	.fill_word,@function
	.globl	.fill_word
.fill_word:
	std 31,-8(1)
	stdu 1,-64(1)
	mr 31,1
	std 3,112(31)
	std 4,120(31)
	std 5,128(31)

    .set rf3,3
    .set    r3,3
    .set    r4,4
    .set    r5,5
    .set    r6,6

    cmpi     1, 1, r5, 0x0000           # Compare numwords to 0 and place
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
	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr


error_ret_word:

    addi    r3, 0, -1
    bclr    20, 0

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,0,0,0,128,1,0,1
	.size	.fill_word,.-.fill_word

	.align 2
	.globl fill_dword
	.section	".opd","aw"
	.align 3
fill_dword:
	.quad	.fill_dword,.TOC.@tocbase,0
	.previous
	.size	fill_dword,24
	.type	.fill_dword,@function
	.globl	.fill_dword
.fill_dword:
	std 31,-8(1)
	stdu 1,-64(1)
	mr 31,1
	std 3,112(31)
	std 4,120(31)
	std 5,128(31)

    .set rf3,3
    .set    r3,3
    .set    r4,4
    .set    r5,5
    .set    r6,6

    cmpi     1, 1, r5, 0x0000           # Compare numdwords to 0 and place
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

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr

error_ret_dword:

    addi    r3, 0, -1
    bclr    20, 0

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,0,0,0,128,1,0,1
	.size	.fill_dword,.-.fill_dword

    .align 2
	.globl fill_dword_addr
	.section	".opd","aw"
	.align 3
fill_dword_addr:
	.quad	.fill_dword_addr,.TOC.@tocbase,0
	.previous
	.size	fill_dword_addr,24
	.type	.fill_dword_addr,@function
	.globl	.fill_dword_addr
.fill_dword_addr:
	std 31,-8(1)
	stdu 1,-64(1)
	mr 31,1
	std 3,112(31)
	std 4,120(31)
	std 5,128(31)

    .set rf3,3
    .set    r3,3
    .set    r4,4
    .set    r5,5
    .set    r6,6

    cmpi     1, 1, r5, 0x0000           # Compare numdwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_addr      # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of word transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9, r5

    addic   r3, r3, -8
#    lfd     rf3, 0(r4)                  # Load 8 bytes into fp3
#    ld     r7,0(r4)

more_addrs:

#    stfdu   rf3, 8(r3)                  # Store same 8 bytes at destination
#    addi    r3,r3,8 
    addi     r3,r3,8
    std     r3,0(r3)
    bc      16, 0, more_addrs          # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr

error_ret_addr:

    addi    r3, 0, -1
    bclr    20, 0

	ld 3,112(31)
	ld 4,120(31)
	ld 5,128(31)
	ld 1,0(1)
	ld 31,-8(1)
	blr
	.long 0
	.byte 0,0,0,0,128,1,0,1
	.size	.fill_dword_addr,.-.fill_dword_addr


	.ident	"GCC: (GNU) 3.2.3 20030329 (prerelease)"
