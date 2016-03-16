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
# @(#)34        1.2  src/htx/usr/lpp/htx/bin/hxecache/prefetch_dcbtp.s, exer_cache, htx61D 8/10/08 23:16:36 

# This module is called from prefetch_irritator.c 
# G3 - contains address of vector  
# G4 - Size of the vector that has to be prefetched 
# G5 - Seed .
# G6 - iAddress of the thread specific data area 

# Layout of the thread specific data area 
#
# start_addr :  .long  0 , 0
# end_addr :    .long  0 , 0
# seed :        .long  0 , 0
# direction :   .long 0x0


# Declare the procedure descriptor for "partial_dcbt"
# Start the "text" section
.section ".text"
.global partial_dcbt
.type partial_dcbt,@function
	
partial_dcbt :
        .set    g0,0
        .set    g1,1
        .set    g2,2
        .set    g3,3
        .set    g4,4
        .set    g5,5
        .set    g6,6
        .set    g7,7
        .set    g8,8
        .set    g9,9
        .set    g10,10
        .set    g11,11
        .set    g12,12 
	.set    size_of_cacheline,128
	.set    jump_bytes,3456

# Note : Partial cache line dcbtp to work , 64B partial line is retrived and placed into L3 cache .
# For consumption of streams one is jumping (2+24+1)lines [2 lines are prefetched by L1 and 24 by L2].
# 128 * 27 = 3456 bytes 
        
	ld g7,_partial_dcbt_data@got(2)
	
# random seed generated using time stamp passed in g5 stored in time_seed
	std g5,0x10(g6)

# g3 will have the starting address of the vector and g4 will have the size of the vector 
# [ i.e passed by c function to assembly ]
	std g3,0x0(g6)
# calculate end of vector [ g5 ]
	add g5,g3,g4
	std g5,0x08(g6) 			 # store it in endaddress	
# find loop count  
	li g9,jump_bytes
	divd g11,g4,g9              # [ g11 <- g4 [vector size ]/g9 [ multiple of jump_bytes ] ]
	subi g11,g11,1
	mtctr g11                      # move the count to ctr

# Translations to be present for first and last page 
	ld g11,0x0(g3)
	ld g11,0x0(g5)
	nop

	li g4,jump_bytes
	li g9,0
	ldx g3,g9,g6

PF_phase1:
	# phase 1 dcbt ra,rb ,0b11000
	# EA interpreted as shown below
	#     +-----------------------------------------+-+----+
	# EA  |                EATRUNC                  |\| SZ |
	#     +-----------------------------------------+------+
	#     0                                         61 62 63
	#  0-60 bits => Effective Address
	#  61 => Reserved 
	#  62-63 Size of aligned cache block to be prefetched 
	#  0b00 8 bytes
	#  0b01 16 bytes
	#  0b10 32 bytes
	#  0b11 64 bytes

	li g10,0	
	li g11,3
	lwz g12,0x14(g6)  #Masking random number to obtain last two bits for SIZE field .
	and g11,g12,g11
	or  g3,g3,g11
	
	# do DCBT th = 11000 . dcbt g3,g10,0x18
	.long 0x7f03522c 
	 
	
	# phase 2 dcbt ra,rb,01010
 	# EA is interpreted as follows.  
	#
	#     +------------------+----+--+---+---+----------+---+-+----+
	#  EA |        ///       |GO S|/ |DEP|///| UNIT_CNT |T U|/| ID |
	#     +------------------+----+--+---+---+----------+---+-+----+
	#     0                 32  34 35  38    47        57 58 60  63
	
	# randomise DEPTH 36:38 and set U to 1 [unlimited number of data units ]
	lwz g9,0x10(g6)
	li g11,7
	and g10,g9,g11	
	# need to shift random number 25 bits align to depth field
	li g11,25
	slw g10,g10, g11  
	lis g11,0
	ori g11,g11,0x0020               # set U [Unlimited bit =1]
	or g11,g11,g10
	or g3,g3,g11

	#set all fields except for GO and STOP and do a DCBT
	lwz g10,0x0(g7)
	and g3,g3,g10		
	
	li g10,0
	li g12,0

loop1 :
	# dcbt g3,g10,0xA
	.long 0x7d43522c
	addi g3,g3,1
	addi g12,g12,1
	cmpwi g12,0xc
	bne loop1


	# phase 3 dcbt ra,rb,01010 with go bits set 
	#      +------------------+----+--+---+---+----------+---+-+----+
	#    EA|        ///       |GO S|/ |DEP|///| UNIT_CNT |T U|/| ID |
	#      +------------------+----+--+---+---+----------+---+-+----+
	#       0                 32  34 35  38    47        57 58 60  63

	# set go field
	lis g11,0x8000
	or g3,g3,g11
	li g10,0

	# One dcbt instruction with GO bit =1 is sufficient to kick off all the nascent streams .
	# dcbt g3,g10,0xA
	.long 0x7d43522c

	# load effective memory address from thread specific data segment and keep incrementing address by
	# size_of_jump_bytes. 
	li g11,0x0

mask_cont :
	ldx g3,g11,g6

PF_consume:	
	add g3,g3,g4
	lwz g11,0x0(g3) 
	bdnz PF_consume
	blr 

# Data segment
.section .data
_partial_dcbt_data :

mask:         .long 0x3ffffff0  # GO/STOP and STREAM IDS are set to 0


