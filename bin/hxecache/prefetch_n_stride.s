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
# @(#)14        1.2  src/htx/usr/lpp/htx/bin/hxecache/prefetch_n_stride.s, exer_cache, htx61D 8/10/08 23:12:02

# This module is called from prefetch_irritator.c 
# G3 - contains address of vector  
# G4 - Size of the vector that has to be prefetched 
# G5 - Seed .
# G6 - iAddress of thread specific data area 

# Layout of the thread specific data area 
#
# start_addr :  .long  0 , 0
# end_addr :    .long  0 , 0
# seed :        .long  0 , 0
# direction :   .long 0x0

# Declare the procedure descriptor for "n_stride"
# Start the "text" section
.section ".text"
.global n_stride
.type n_stride,@function

n_stride :
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

	ld g7,_n_stride_data@got(2)
	
# random seed generated using time stamp passed in g5 stored in time_seed
	std g5,0x10(g6)
# g3 will have the starting address of the vector and g4 will have the size of the vector 
# [ i.e passed by c function to assembly ]
	std g3,0x0(g6)
# calculate end of vector [ g5 ]
	add g5,g3,g4
	std g5,0x08(g6) 			#store it in endaddress	

# Translations to be present for first and last page 
	ld g8,0x0(g3)
	ld g8,0x0(g5)
	nop

# find loop count
    li g9,0
    lwz g9,0x14(g6)
    li g11,14
    srd g9,g9,g11				# this gives one the number of words 
    li  g11 , 2 
    sld g9,g9,g11
	
    divd g11,g4,g9              # [ g11 <- g4 [vector size ]/g9 [ jump size ] ]
    mtctr g11                   # move the count to ctr
	
    mr g4,g9
    li g9,0
    ldx g3,g9,g6

# Determine to start prefecting forward or backwards
# TH = 01000 , 57th bit [ D ]of EA will determine the above
# D = 0  -> forwards
# D = 1  -> backwards
# chk EA i.e gpr3
# 57th bit of random number would determine value of ' D '  .
	ld g9,0x10(g6)
	extrdi. g10,g9,1,57
	cmpwi g10,0x0
	beq PF_cont

# need to load  g3 ending address
	neg g4,g4
	li g11,0x8
	ldx g3,g11,g6

PF_cont : 
#set 57 bit of g3 to D value .
	insrdi. g3 ,g10,1,57
	stw g10 , 0x18(g6) 

PF_phase1:
	# phase 1 dcbt ra,rb ,01000
	# EA interpreted as shown below
	#     +-----------------------------------------+----+-+------+
	# EA  |                EATRUNC                  |D UG|/|  ID  |
	#     +-----------------------------------------+----+-+------+
	#      0                                         57 58  60  63

	li g10,0	
	li g12,0

	li g11,4
	srd g3,g3,g11                #initialise bits 60-63 to zero  
	sld g3,g3,g11

loop1 :		
	 # do DCBT th = 01000 . dcbt g3,g10,0x8
	 .long 0x7d03522c
	 addi g3,g3,1
	 addi g12 ,g12 ,1                             # initialise all 6x2 streams SMT
	 cmpwi g12 ,0xc
	 bne loop1
	
	# phase 2 dcbt ra,rb,01010
 	# EA is interpreted as follows.  
	#
	#     +------------------+----+--+---+---+----------+---+-+----+
	#  EA |        ///       |GO S|/ |DEP|///| UNIT_CNT |T U|/| ID |
	#     +------------------+----+--+---+---+----------+---+-+----+
	#     0                 32  34 35  38    47        57 58 60  63
	
	# randomise DEPTH 36:38 and set U to 1 [unlimited number of data units ]
	lwz g9,0x10(g6)
	li g11 , 7
	and g10 , g9 ,g11	
	# need to shift random number 25 bits align to depth field
	li g11 ,25
	slw g10, g10, g11  
	lis g11 ,0
	ori g11,g11,0x0020               # set U [Unlimited bit =1]
	or g11,g11,g10
	or g3,g3,g11

	#set all fields except for GO and STOP and do a DCBT
	lwz g10 , 0x0(g7)
	and g3 , g3 , g10		
	
	li g10 , 0
	li g12 , 0

loop2 :
	# dcbt g3,g10,0xA
	.long 0x7d43522c
	addi g3,g3,1
	addi g12,g12,1
	cmpwi g12,0xc
	bne loop2

	# phase 3 dcbt/st ra , rb ,01011
	# EA is interpreted as follows.
	#
	#     +------------------+------------------+------+------+---+---+
	#  EA |        ///       | STRIDE           |OFFSET|UNITSZ|// |ID |
	#     +------------------+------------------+------+------+---+---+
	#     0                 32                  50     55     58  60  63

        # All the fields are set on randomly .
        # set  the Stream - ID .to Zero .
	ld g3 , 0x10(g6)
	li g11 , 6
	sld g3 , g3 ,g11	

	li g12 , 0
	li g10 , 0

loop3 :
	#dcbt/st g3 ,g10 , 0xB
	.long 0x7d63522c 
	addi g3 , g3 , 1
	addi g12 , g12 ,1
	cmpwi g12 , 0xc
	bne loop3


	# phase 4 dcbt ra,rb,01010 with go bits set 
	#      +------------------+----+--+---+---+----------+---+-+----+
	#    EA |        ///       |GO S|/ |DEP|///| UNIT_CNT |T U|/| ID |
	#      +------------------+----+--+---+---+----------+---+-+----+
	#       0                 32  34 35  38    47        57 58 60  63

	# set go field
	lis g11,0x8000
	or g3,g3,g11
	li g11,4
	srd g3,g3,g11                #initialise all 6x2 streams SMT
	sld g3,g3,g11
	li g12,0
	li g10,0

	# One dcbt instruction with GO bit =1 is sufficient to kick off all the nascent streams .
	# dcbt g3,g10,0xA
	.long 0x7d43522c

	# load effective memory address from "address" and keep incrementing address by size_of_cacheline 
	# 128 bytes to consume the stream . 
	lwz g12,0x18(g6)
	cmpwi g12 , 0x0
	bne loop4 
	li g11,0x0
	b mask_cont
	
loop4 : 
	li g11,0x8

mask_cont :
	ldx g3,g11,g6

PF_consume:	
	add g3,g3,g4
	lwz g11,0x0(g3) 
	bdnz PF_consume
	blr 

# Data segment
.section .data
_n_stride_data :

mask:         .long 0x3ffffff0  # GO/STOP and STREAM IDS are set to 0



