# @(#)19        1.2  src/htx/usr/lpp/htx/bin/hxecache/prefetch_dcbtt.s, exer_cache, htx61D 8/10/08 23:13:36

# This module is called from prefetch_irritator.c 
# G3 - contains address of vector  
# G4 - Size of the vector that has to be prefetched 
# G5 - Seed 
# G6 - Address of the thread specific data area 

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
        
	ld g7,_transient_dcbt_data@got(2)

# random seed generated using time stamp passed in g5 stored in time_seed
	std g5,0x10(g6)
# g3 will have the starting address of the vector and g4 will have the size of the vector 
# [ i.e passed by c function to assembly ]
	std g3,0x0(g6)
# calculate end of vector [ g5 ]
	add g5,g3,g4
	std g5,0x08(g6) 			#store it in endaddress	
# find loop count  
	li g9,size_of_cacheline
	divd g11,g4,g9              # [ g11 <- g4 [vector size ]/g9 [ multiple of size_of_cacheline ] ]
	mtctr g11                   # move the count to ctr

# Translations to be present for first and last page 
	ld g11 , 0x0(g3)
	ld g11 , 0x0(g5)
	nop

	li g4,size_of_cacheline
	li g9,0
	ldx g3,g9,g6

# Determine to start prefecting forward or backwards
# TH = 01000 , 57th bit [ D ]of EA will determine the above
# D = 0  -> forwards
# D = 1  -> backwards
# chk EA i.e gpr3
# 57th bit of random number would determine value of ' D '  .
	ld g9,0x10(g6)
	extrdi. g10 , g9 ,1 ,57
	cmpwi g10 , 0x0
	beq PF_cont

# need to load  g3 ending address
	neg g4 , g4
	li g11,0x8
	ldx g3,g11,g6

PF_cont : 
#set 57 bit of g3 to D value .
	insrdi. g3,g10,1,57
	stw g10,0x18(g6) 
	li g10,0

Prephase_Transient : 
# This is to hint that all prefetch done further down are transient data .
# dcbt ra,rb,10000
# dcbt g3,g10,0x10
	.long 0x7e03522c 


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
	 addi g12 ,g12 ,1                             # initialise all 8x2 streams SMT
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


	# phase 3 dcbt ra,rb,01010 with go bits set 
	#      +------------------+----+--+---+---+----------+---+-+----+
	#    EA |        ///       |GO S|/ |DEP|///| UNIT_CNT |T U|/| ID |
	#      +------------------+----+--+---+---+----------+---+-+----+
	#       0                 32  34 35  38    47        57 58 60  63

	# set go field
	lis g11,0x8000
	or g3,g3,g11
	li g11,4
	srd g3,g3,g11                #initialise all 8x2 streams SMT
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
	bne loop3 
	li g11 ,0x0
	b mask_cont

loop3 : 
	li g11 ,0x8

mask_cont :
	ldx g3 ,g11 ,g6
	ldx g9 ,g11 , g6

mask_bits:
	li g8,0x7f            # masking all bits except the last six bits which decides offset in the line 
	and g9,g9,g8	

PF_consume:	
	add g3,g3,g4
	or  g9,g9,g3
	lwz g11,0x0(g9) 
	bdnz PF_consume
	blr 

# Data segment
.section .data
_transient_dcbt_data :

mask:         .long 0x3ffffff0  # GO/STOP and STREAM IDS are set to 0


