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

# @(#)42        1.4.1.5  src/htx/usr/lpp/htx/bin/hxefpscr/fpscr_bust.s, exer_fpscr, htxubuntu 8/18/14 05:45:00
.machine "ppc64"
.section ".text"
.align 3
.globl fpscr_bust
.type  fpscr_bust,@function

.set num_instr,   0x5


.set    F0,  0
.set    F1,  1
.set    F2,  2
.set    F3,  3
.set    F4,  4
.set    F5,  5
.set    F6,  6
.set    F7,  7
.set    F8,  8
.set    F9,  9
.set    F10,10
.set    F11,11
.set    F12,12
.set    F13,13
.set    F14,14
.set    F15,15
.set    F16,16
.set    F17,17
.set    F18,18
.set    F19,19
.set    F20,20
.set    F21,21
.set    F22,22
.set    F23,23
.set    F24,24
.set    F25,25
.set    F26,26
.set    F27,27
.set    F28,28
.set    F29,29
.set    F30,30
.set    F31,31

.set    G0,  0
.set    G1,  1
.set    G2,  2
.set    G3,  3
.set    G4,  4
.set    G5,  5
.set    G6,  6
.set    G7,  7
.set    G8,  8
.set    G9,  9
.set    G10,10
.set    G11,11
.set    G12,12
.set    G13,13
.set    G14,14
.set    G15,15
.set    G16,16
.set    G17,17
.set    G18,18
.set    G19,19
.set    G20,20
.set    G21,21
.set    G22,22
.set    G23,23
.set    G24,24
.set    G25,25
.set    G26,26
.set    G27,27
.set    G28,28
.set    G29,29
.set    G30,30
.set    G31,31
.set    cr0,0
.set    cr1,1

.set    hdec,   310
.set    lpid,   319
.set    hrmor,  313
.set    accr,   0x1D
.set    dsisr,  0x12
.set    srr0,   0x1A
.set    srr1,   0x1B
.set    hsrr0,  314
.set    hsrr1,  315
.set    lpcr,   318
.set    sprg0,  0x110
.set    sprg1,  0x111
.set    sprg2,  0x112
.set    sprg3,  0x113
.set    sdr1,   0x19
.set    dar,    0x13
.set    lr,     0x8
.set    ctr,    0x9
.set    pir,    1023     # 604 book says this is 1013 on page 2-10
.set    mmcr0,  0x31B
.set    hid0,   0x3F0
.set    hid1,   0x3F1
.set    hid4,   0x3F4
.set    buscsr, 0x3F8
.set    l2cr,   0x3F9
.set    l2sr,   0x3FA
.set    ctrl,   152
.set    scomc,  276
.set    scomd,  277
.set    myblr1, 0x4e80
.set    myblr2, 0x0020
.set    myblr3, 0x0000
.set    myblr4, 0x0001


.set    GEN_MASK,0x0FF #No longer needed
#.set   GEN_CNT,((inlast-inlist)/8)+1
.set    GEN_CNT,((inlast-inlist)/8)


fpscr_bust:	lis  G8,save3@ha
                std  G3,save3@l(G8)         # Save Address of Gprs
                lis  G8,savepvr@ha
                stw  G6,savepvr@l(G8)
		mflr G3
		lis  G10,stdata@ha
		addi G10,G10,stdata@l 
		std  G3, (GP0-stdata)(G10)
		std  G2, (GP2-stdata)(G10) 
		std  G4, (GP4-stdata)(G10)  
                std  G5, (Addr-stdata)(G10)
                lwz  G6, 0(G5)
                stw  G6, (GP5-stdata)(G10)
                std  G14, (GP14-stdata) (G10)
                std  G15, (GP15-stdata) (G10)
                std  G16, (GP16-stdata) (G10)
                std  G17, (GP17-stdata) (G10)
                std  G18, (GP18-stdata) (G10)
                std  G19, (GP19-stdata) (G10)
		li   G8, 0x0
		stw  G8, (loopcnt-stdata)(G10)
fpscr_bust1:    bl      gen_data		# Generate random data for FPRs
                bl      gentrash                # Generate instruction stream
                bl      load_fprs1              # load FPRs with random data
                mtspr   9, G4
                bctrl
                mffs    F0                      # save pass 1 FPSCR
                stfd    F0,(fpscr1-stdata)(G10)
		bl      save_fprs1		#For performance reason at present commenting
                ld      G11,(fpscr1-stdata)(G10)
                bl      load_fprs1              # restore FPRs with same data
		mtspr   9, G4
		bctrl
		mffs    F0                      # save pass 2 FPSCR
                stfd    F0,(fpscr2-stdata)(G10)
		bl      save_fprs2		#For performance reason at present commenting
                ld      G8,(fpscr2-stdata)(G10)
                cmpd   G11,G8
		#.long   0x7C2B4000
                beq-    pass                   # bump pass count, run again

fail:           lis  G8, save3@ha
                ld   G3, save3@l(G8)          # Restore address of Gprs
                #stmw G0, 0(G3)                 # Save 32 grps in Gprs
		bl   save_gprs
		ld   G2, (GP2-stdata)(G10)
		ld   G3, (GP0-stdata)(G10)
                mtlr G3
		li   G3, -0x1
		ld   G6, (Addr-stdata) (G10)
		ld   G5, (GP5-stdata) (G10)
		std  G5, 0(G6)
                ld   G14, (GP14-stdata) (G10)
                ld   G15, (GP15-stdata) (G10)
                ld   G16, (GP16-stdata) (G10)
                ld   G17, (GP17-stdata) (G10)
                ld   G18, (GP18-stdata) (G10)
                ld   G19, (GP19-stdata) (G10)
		blr

pass: 		lwz  G8, (loopcnt-stdata)(G10)
		addi G8,G8,1
		stw  G8, (loopcnt-stdata)(G10)
		#li  G7,0x7530
		lwz  G7, (Iteration-stdata)(G10)
		cmpw 1,G8,G7 				
		bne cr1,fpscr_bust1

exit:           lis  G8, save3@ha
                ld   G3,save3@l(G8)          # Restore address of Gprs
                #stmw G0,0(G3)                 # Save 32 grps in Gprs
		bl   save_gprs
	        ld   G2, (GP2-stdata)(G10)
                ld   G3, (GP0-stdata)(G10)
                mtlr G3
                lwz  G3, (loopcnt-stdata)(G10)
		ld   G6, (Addr-stdata) (G10)
		ld   G5, (GP5-stdata) (G10)
		std  G5, 0(G6)
                ld   G14, (GP14-stdata) (G10)
                ld   G15, (GP15-stdata) (G10)
                ld   G16, (GP16-stdata) (G10)
                ld   G17, (GP17-stdata) (G10)
                ld   G18, (GP18-stdata) (G10)
                ld   G19, (GP19-stdata) (G10)
                blr


		
#-----------------------------------------
# Gen data
#-----------------------------------------

gen_data:       lwz     G3,(GP5-stdata)(G10)
                lwz     G8,(mulop-stdata)(G10)
                lwz     G11,(addop-stdata)(G10)
                li      G9,64           #generate 64 random words
                mtctr   G9
                la      G9,((rand_data-stdata)-4)(G10)
hashdata:       mullw   G3,G8,G3
                add     G3,G11,G3       # G3 has Random number
                stwu    G3,4(G9)        # store random data
                bdnz    hashdata
                stw     G3,(GP5-stdata)(G10)
                blr                     # Return to calling routine

#-----------------------------------------
# load_fprs
#-----------------------------------------
load_fprs1:
                lfd     F0,(zero-stdata)(G10)         # Reset Exceptions
	#For 64 bit FPSCR
		#mtfsf  0xFF,F0,1,0     #Clears all 64 bit of FPSCR
		.long   0xfffe058e
Bit32_FPSCR:    lfd     0,((rand_data-stdata)+0x00)(G10)
                lfd     1,((rand_data-stdata)+0x08)(G10)
                lfd     2,((rand_data-stdata)+0x10)(G10)
                lfd     3,((rand_data-stdata)+0x18)(G10)
                lfd     4,((rand_data-stdata)+0x20)(G10)
                lfd     5,((rand_data-stdata)+0x28)(G10)
                lfd     6,((rand_data-stdata)+0x30)(G10)
                lfd     7,((rand_data-stdata)+0x38)(G10)
                lfd     8,((rand_data-stdata)+0x40)(G10)
                lfd     9,((rand_data-stdata)+0x48)(G10)
                lfd     10,((rand_data-stdata)+0x50)(G10)
                lfd     11,((rand_data-stdata)+0x58)(G10)
                lfd     12,((rand_data-stdata)+0x60)(G10)
                lfd     13,((rand_data-stdata)+0x68)(G10)
                lfd     14,((rand_data-stdata)+0x70)(G10)
                lfd     15,((rand_data-stdata)+0x78)(G10)
                lfd     16,((rand_data-stdata)+0x80)(G10)
                lfd     17,((rand_data-stdata)+0x88)(G10)
                lfd     18,((rand_data-stdata)+0x90)(G10)
                lfd     19,((rand_data-stdata)+0x98)(G10)
                lfd     20,((rand_data-stdata)+0xA0)(G10)
                lfd     21,((rand_data-stdata)+0xA8)(G10)
                lfd     22,((rand_data-stdata)+0xB0)(G10)
                lfd     23,((rand_data-stdata)+0xB8)(G10)
                lfd     24,((rand_data-stdata)+0xC0)(G10)
                lfd     25,((rand_data-stdata)+0xC8)(G10)
                lfd     26,((rand_data-stdata)+0xD0)(G10)
                lfd     27,((rand_data-stdata)+0xD8)(G10)
                lfd     28,((rand_data-stdata)+0xE0)(G10)
                lfd     29,((rand_data-stdata)+0xE8)(G10)
                lfd     30,((rand_data-stdata)+0xF0)(G10)
                lfd     31,((rand_data-stdata)+0xF8)(G10)
                blr

#-----------------------------------------
# save_fprs pass 1
#-----------------------------------------
save_fprs1:
                stfd     0,((fprs_pass1-stdata)+0x00)(G10)
                stfd     1,((fprs_pass1-stdata)+0x08)(G10)
                stfd     2,((fprs_pass1-stdata)+0x10)(G10)
                stfd     3,((fprs_pass1-stdata)+0x18)(G10)
                stfd     4,((fprs_pass1-stdata)+0x20)(G10)
                stfd     5,((fprs_pass1-stdata)+0x28)(G10)
                stfd     6,((fprs_pass1-stdata)+0x30)(G10)
                stfd     7,((fprs_pass1-stdata)+0x38)(G10)
                stfd     8,((fprs_pass1-stdata)+0x40)(G10)
                stfd     9,((fprs_pass1-stdata)+0x48)(G10)
                stfd     10,((fprs_pass1-stdata)+0x50)(G10)
                stfd     11,((fprs_pass1-stdata)+0x58)(G10)
                stfd     12,((fprs_pass1-stdata)+0x60)(G10)
                stfd     13,((fprs_pass1-stdata)+0x68)(G10)
                stfd     14,((fprs_pass1-stdata)+0x70)(G10)
                stfd     15,((fprs_pass1-stdata)+0x78)(G10)
                stfd     16,((fprs_pass1-stdata)+0x80)(G10)
                stfd     17,((fprs_pass1-stdata)+0x88)(G10)
                stfd     18,((fprs_pass1-stdata)+0x90)(G10)
                stfd     19,((fprs_pass1-stdata)+0x98)(G10)
                stfd     20,((fprs_pass1-stdata)+0xA0)(G10)
                stfd     21,((fprs_pass1-stdata)+0xA8)(G10)
                stfd     22,((fprs_pass1-stdata)+0xB0)(G10)
                stfd     23,((fprs_pass1-stdata)+0xB8)(G10)
                stfd     24,((fprs_pass1-stdata)+0xC0)(G10)
                stfd     25,((fprs_pass1-stdata)+0xC8)(G10)
                stfd     26,((fprs_pass1-stdata)+0xD0)(G10)
                stfd     27,((fprs_pass1-stdata)+0xD8)(G10)
                stfd     28,((fprs_pass1-stdata)+0xE0)(G10)
                stfd     29,((fprs_pass1-stdata)+0xE8)(G10)
                stfd     30,((fprs_pass1-stdata)+0xF0)(G10)
                stfd     31,((fprs_pass1-stdata)+0xF8)(G10)
                blr

#-----------------------------------------
# save_fprs pass 2
#-----------------------------------------
save_fprs2:
                stfd     0,((fprs_pass2-stdata)+0x00)(G10)
                stfd     1,((fprs_pass2-stdata)+0x08)(G10)
                stfd     2,((fprs_pass2-stdata)+0x10)(G10)
                stfd     3,((fprs_pass2-stdata)+0x18)(G10)
                stfd     4,((fprs_pass2-stdata)+0x20)(G10)
                stfd     5,((fprs_pass2-stdata)+0x28)(G10)
                stfd     6,((fprs_pass2-stdata)+0x30)(G10)
                stfd     7,((fprs_pass2-stdata)+0x38)(G10)
                stfd     8,((fprs_pass2-stdata)+0x40)(G10)
                stfd     9,((fprs_pass2-stdata)+0x48)(G10)
                stfd     10,((fprs_pass2-stdata)+0x50)(G10)
                stfd     11,((fprs_pass2-stdata)+0x58)(G10)
                stfd     12,((fprs_pass2-stdata)+0x60)(G10)
                stfd     13,((fprs_pass2-stdata)+0x68)(G10)
                stfd     14,((fprs_pass2-stdata)+0x70)(G10)
                stfd     15,((fprs_pass2-stdata)+0x78)(G10)
                stfd     16,((fprs_pass2-stdata)+0x80)(G10)
                stfd     17,((fprs_pass2-stdata)+0x88)(G10)
                stfd     18,((fprs_pass2-stdata)+0x90)(G10)
                stfd     19,((fprs_pass2-stdata)+0x98)(G10)
                stfd     20,((fprs_pass2-stdata)+0xA0)(G10)
                stfd     21,((fprs_pass2-stdata)+0xA8)(G10)
                stfd     22,((fprs_pass2-stdata)+0xB0)(G10)
                stfd     23,((fprs_pass2-stdata)+0xB8)(G10)
                stfd     24,((fprs_pass2-stdata)+0xC0)(G10)
                stfd     25,((fprs_pass2-stdata)+0xC8)(G10)
                stfd     26,((fprs_pass2-stdata)+0xD0)(G10)
                stfd     27,((fprs_pass2-stdata)+0xD8)(G10)
                stfd     28,((fprs_pass2-stdata)+0xE0)(G10)
                stfd     29,((fprs_pass2-stdata)+0xE8)(G10)
                stfd     30,((fprs_pass2-stdata)+0xF0)(G10)
                stfd     31,((fprs_pass2-stdata)+0xF8)(G10)
                blr

#-----------------------------------------
# save_gprs
#-----------------------------------------
save_gprs:
                std       0,0x00(G3)
                std       1,0x08(G3)
                std       2,0x10(G3)
                std       3,0x18(G3)
                std       4,0x20(G3)
                std       5,0x28(G3)
                std       6,0x30(G3)
                std       7,0x38(G3)
                std       8,0x40(G3)
                std       9,0x48(G3)
                std       10,0x50(G3)
                std       11,0x58(G3)
                std       12,0x60(G3)
                std       13,0x68(G3)
                std       14,0x70(G3)
                std       15,0x78(G3)
                std       16,0x80(G3)
                std       17,0x88(G3)
                std       18,0x90(G3)
                std       19,0x98(G3)
                std       20,0xA0(G3)
                std       21,0xA8(G3)
                std       22,0xB0(G3)
                std       23,0xB8(G3)
                std       24,0xC0(G3)
                std       25,0xC8(G3)
                std       26,0xD0(G3)
                std       27,0xD8(G3)
                std       28,0xE0(G3)
                std       29,0xE8(G3)
                std       30,0xF0(G3)
                std       31,0xF8(G3)
                blr



#-----------------------------------------
# Gen trash
#-----------------------------------------
gentrash:	lwz     G12,(GP5-stdata)(G10)
                lwz     G8,(mulop-stdata)(G10)
                lwz     G11,(addop-stdata)(G10)
                li      G9,num_instr
                mtctr   G9
                la      G9,(inlist-stdata)(G10)
                ld      G4,(GP4-stdata)(G10)
hashtrash:      mullw   G12,G8,G12
                add     G12,G11,G12       # G12 has Random number
                rotrwi  G3,G12,23
                li      G5,GEN_CNT      # load the # of instructions in G5(in optable)
		lwz     G7,(savepvr-stdata)(G10)
		lwz     G6,(pvr-stdata)(G10)
		cmp     0,G7,G6
		bne     0,ins_cnt
		li      G7,0x03
		subf    G5,G7,G5
ins_cnt:	#divdu  G6,G3,G5        # divide (rand #)/(inst #), G6=quotient
		.long   0x7CC32B92
                #mulld  G6,G6,G5        # G6 = quotient*divisor
		.long   0x7CC629D2 
                subf    G3,G6,G3
pickop:         slwi    G3,G3,3         # multiply by 8
                lwzux   G5,G3,G9        # G5 gets skelton instruction
                lwz     G6,4(G3)        # G6 gets mask
                and     G6,G12,G6        # AND mask with random number
                or      G6,G6,G5        # OR random mask onto instruction
                stw     G6,0x0(G4)        # store random op in stream 1
                addi    G4, G4, 0x4
                bdnz    hashtrash
                stw     G12,(GP5-stdata)(G10)
		lis     G12,myblr1
		addi    G12,G12,myblr2
                stw     G12,0x0(G4)        # store random op in stream 1
                ld      G4,(GP4-stdata)(G10)
                dcbf    G0,G4
		sync
		icbi    G0,G4
		isync                   # no_icbis needed.
                blr                     # Return to calling routine
                nop
                nop
                nop
end:            nop







                .section        ".data"
		.globl stdata
stdata:           
loopcnt:        .double   0
intcnt:         .double   0              			# 5000
seed:           .long   0xABCDEF01				# Seed/Last Seed
lastseed:       .long   0x0FEDCBA9
mulop:          .long   0x0019660D				# For Random Number Gen
addop:          .long   0x3C6EF35F
zero:           .double 0					# Zero
fpscr1:         .double 0					# Zero
fpscr2:         .double 0					# Zero
rand_data:      .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1		# reserve storage for random data
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
fprs_pass1:     .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
fprs_pass2:     .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
                .long 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
GP0:		.long 1,1
GP1:		.long 0x1314,1
GP2:		.long 1,1
GP4:            .long 1,1
GP5:		.long 1,1
GP14:           .long 1,1
GP15:           .long 1,1
GP16:           .long 1,1
GP17:           .long 1,1
GP18:           .long 1,1
GP19:           .long 1,1
Addr:		.long 1,1
save3:          .long 0,0
Iteration:	.long 0x000f4240
savepvr:        .long 0x00000000
pvr:            .long 0x3e
# skeleton/mask pairs for floating point instruction stream generation
inlist:
        #FLOATING POINT
        .long  0xFC00048E, 0x03E00000   # X O S R S mffs(.) (63-583)
        .long  0xFC00048E, 0x03E00000   # X O S R S mffs(.) (63-583)
        .long  0xFC00048E, 0x03E00000   # X O S R S mffs(.) (63-583)
        .long  0xFC00048E, 0x03E00000   # X O S R S mffs(.) (63-583)
        .long  0xFC00008C, 0x03E00000   # X O S R S mtfsb0(.) (63-070)
        .long  0xFC00004C, 0x01E00000   # X O S R S mtfsb1(.) (63-038)
        .long  0xFC00010C, 0x0180F000   # X O S R S mtfsfi(.) (63-134)
        .long  0xFC00058E, 0x03FFF801   # XFL O S R S mtfsf (63-711)
        .long  0xFC00002A, 0x03FFF801   # A F S R S fadd(.) (63-21)
        .long  0xFC000040, 0x039FF800   # X F S R S fcmpo(.) (63-032)
        .long  0xFC000000, 0x039FF800   # X F S R S fcmpu(.) (63-000)
        .long  0xFC00065C, 0x03E0F800   # X F S R S fctid(.) (63-814)
        .long  0xFC00065E, 0x03E0F800   # X F S R S fctidz(.) (63-815)
        .long  0xFC00001C, 0x03E0F800   # X F S R S fctiw(.) (63-014)
        .long  0xFC00001E, 0x03E0F800   # X F S R S fctiwz(.) (63-015)
        .long  0xFC000024, 0x03FFF800   # A F S R S fdiv(.) (63-18)
        .long  0xFC00003A, 0x03FFFFC1   # A F S R S fmadd(.) (63-29)
        .long  0xFC000090, 0x03E0F800   # X F S R S fmr(.) (63-072)
        .long  0xFC000038, 0x03FFFFC0   # A F S R S fmsub(.) (63-28)
        .long  0xFC000032, 0x03FF07C0   # A F S R S fmul(.) (63-25)
        .long  0xFC000050, 0x03E0F800   # X F S R S fneg(.) (63-040)
        .long  0xFC00003E, 0x03FFFFC0   # A F S R S fnmadd(.) (63-31)
        .long  0xFC00003C, 0x03FFFFC0   # A F S R S fnmsub(.) (63-30)
        .long  0xFC000034, 0x03E0F800   # A F S R S frsqrte(.) (63-26)
        .long  0xFC00002E, 0x03FFFFC0   # A F S R S fsel(.) (63-23)
        .long  0xFC00002C, 0x03E0F800   # A F S R S fsqrt(.) (63-22)
        .long  0xFC000028, 0x03FFF800   # A F S R S fsub(.) (63-20)
        .long  0xFC00002A, 0x03FFF800   # A F S R S fadd(.) (63-21)
        .long  0xFC000040, 0x039FF800   # X F S R S fcmpo(.) (63-032)
        .long  0xFC000000, 0x039FF800   # X F S R S fcmpu(.) (63-000)
        .long  0xFC00065C, 0x03E0F800   # X F S R S fctid(.) (63-814)
        .long  0xFC00065E, 0x03E0F800   # X F S R S fctidz(.) (63-815)
        .long  0xFC00001C, 0x03E0F800   # X F S R S fctiw(.) (63-014)
        .long  0xFC00001E, 0x03E0F800   # X F S R S fctiwz(.) (63-015)
        .long  0xFC000024, 0x03FFF800   # A F S R S fdiv(.) (63-18)
        .long  0xFC00003A, 0x03FFFFC0   # A F S R S fmadd(.) (63-29)
        .long  0xFC000090, 0x03E0F800   # X F S R S fmr(.) (63-072)
        .long  0xFC000038, 0x03FFFFC0   # A F S R S fmsub(.) (63-28)
        .long  0xFC000032, 0x03FF07C0   # A F S R S fmul(.) (63-25)
        .long  0xFC000050, 0x03E0F800   # X F S R S fneg(.) (63-040)
        .long  0xFC00003E, 0x03FFFFC0   # A F S R S fnmadd(.) (63-31)
        .long  0xFC00003C, 0x03FFFFC0   # A F S R S fnmsub(.) (63-30)
        .long  0xFC000018, 0x03E0F800   # X F S R S frsp(.) (63-012)
        .long  0xFC000034, 0x03E0F800   # A F S R S frsqrte(.) (63-26)
        .long  0xFC00002E, 0x03FFFFC0   # A F S R S fsel(.) (63-23)
        .long  0xFC00002C, 0x03E0F800   # A F S R S fsqrt(.) (63-22)
        .long  0xFC000028, 0x03FFF800   # A F S R S fsub(.) (63-20)
        .long  0xFC00002A, 0x03FFF800   # A F S R S fadd(.) (63-21)
        .long  0xFC000040, 0x039FF800   # X F S R S fcmpo(.) (63-032)
        .long  0xFC000000, 0x039FF800   # X F S R S fcmpu(.) (63-000)
        .long  0xFC00065C, 0x03E0F800   # X F S R S fctid(.) (63-814)
        .long  0xFC00065E, 0x03E0F800   # X F S R S fctidz(.) (63-815)
        .long  0xFC00001C, 0x03E0F800   # X F S R S fctiw(.) (63-014)
        .long  0xFC00001E, 0x03E0F800   # X F S R S fctiwz(.) (63-015)
        .long  0xFC000024, 0x03FFF800   # A F S R S fdiv(.) (63-18)
        .long  0xFC00003A, 0x03FFFFC0   # A F S R S fmadd(.) (63-29)
        .long  0xFC000090, 0x03E0F800   # X F S R S fmr(.) (63-072)
        .long  0xFC000038, 0x03FFFFC0   # A F S R S fmsub(.) (63-28)
        .long  0xFC000032, 0x03FF07C0   # A F S R S fmul(.) (63-25)
        .long  0xFC000050, 0x03E0F800   # X F S R S fneg(.) (63-040)
        .long  0xFC00003E, 0x03FFFFC0   # A F S R S fnmadd(.) (63-31)
        .long  0xFC00003C, 0x03FFFFC1   # A F S R S fnmsub(.) (63-30)
        .long  0xFC000034, 0x03E0F800   # A F S R S frsqrte(.) (63-26)
        .long  0xFC00002E, 0x03FFFFC0   # A F S R S fsel(.) (63-23)
        .long  0xFC00002C, 0x03E0F800   # A F S R S fsqrt(.) (63-22)
        .long  0xFC000028, 0x03FFF800   # A F S R S fsub(.) (63-20)
	.long  0xFC00069C, 0x03E0F800   # X F S R S fcfid(.) (63-846)
	.long  0xFC00069C, 0x03E0F800   # X F S R S fcfid(.) (63-846)
	.long  0xFC00069C, 0x03E0F800   # X F S R S fcfid(.) (63-846)
inlast:         

