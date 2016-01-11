# @(#)99	1.7  src/htx/usr/lpp/htx/bin/hxemem64/comp_linux64.s, exer_mem64, htxubuntu 5/30/07 01:49:43
#########################################################################
    .file    "comp_linux64.c"
    .section    ".text"
    .align 2
    .globl wr_cmp_dword
    .section    ".opd","aw"
    .align 3
wr_cmp_dword:
    .quad    .wr_cmp_dword,.TOC.@tocbase,0
    .previous
    .size    wr_cmp_dword,24
    .type    .wr_cmp_dword,@function
    .globl    .wr_cmp_dword
.wr_cmp_dword:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)

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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6
    cmpi     1, 1, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_wr            # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

    ld      r6, 0(r3)                   # Load 8 bytes from buffer1
    ld      r7, 0(r4)                   # Load 8 byte bit pattern
    lis     r20,gpr3@ha
    std     r3, gpr3@l(r20)
    mr r9,r3
    addi    r9,r9,-8                    # Index buffer1
    addi    r3, 0, 0                    # Set GPR-3 to zero for dcbf
more_cmp_dwords:
    addi    r9,r9,8                     # Index buffer1
    std     r7,0(r9)                    # Store 8 bytes to buffer

    dcbf    r3, r9                      #dcbf

    ld      r6,0(r9)                    # Load 8 bytes from buffer1
    cmpd    1,  r6,r7                   # Compare 8 bytes
    bne     cr1,error_ret_wr               # branch if r6 != r7
    bc      16, 0, more_cmp_dwords           # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_wr:   
    mfctr   r8  
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13

    cmpi           1,0,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, wr_no_trap         # Trap flag set to 0, dont trap to KDB

    cmpi           1,0,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, wr_no_trap         # Trap flag set to 1, trap to KDB

    cmpi           1,0,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, wr_attn         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here
wr_attn:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

wr_trap:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here
wr_no_trap:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8

    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
    lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
#   lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr
    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .wr_cmp_dword,.-.wr_cmp_dword
    .align 2
    .globl comp_dword
    .section    ".opd","aw"
    .align 3
comp_dword:
    .quad    .comp_dword,.TOC.@tocbase,0
    .previous
    .size    comp_dword,24
    .type    .comp_dword,@function
    .globl    .comp_dword
.comp_dword:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)


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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6

    cmpi     1 , 1, r5, 0x0000     # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.


    ld         r7, 0(r4)               # Load 4 byte bit pattern
    lis         r20,gpr3@ha
    std         r3, gpr3@l(r20)        # Save r3 at location gpr3
    mr         r9,r3
    addi        r9,r9,-8                # Index buffer1
    xor         r3,r3,r3        # clear r3

more_dwords:
    addi        r9,r9,8                 # Index buffer1
    ld         r6,0(r9)                # Load 8 bytes from buffer1
    cmpd         1,r6,r7               # Compare 8 bytes
    bne         cr1,error_ret           # branch if r6 != r7
    bc          16, 0, more_dwords      # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return

    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret:
    mfctr       r8
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13
    cmpi           1,1,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, no_trap         # Trap flag set to 0, dont trap to KDB

    cmpi           1,1,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, trap         # Trap flag set to 1, trap to KDB

    cmpi           1,1,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, attn         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here

attn:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

trap:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here

no_trap:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8
    
    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
#   lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
    lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr

    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .comp_dword,.-.comp_dword

    .align 2
    .globl comp_word
    .section    ".opd","aw"
    .align 3
comp_word:
    .quad    .comp_word,.TOC.@tocbase,0
    .previous
    .size    comp_word,24
    .type    .comp_word,@function
    .globl    .comp_word
.comp_word:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)


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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6

    cmpi     1 , 1, r5, 0x0000     # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_word       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.


    lwz         r7, 0(r4)               # Load 4 byte bit pattern
    lis         r20,gpr3@ha
    std         r3, gpr3@l(r20)        # Save r3 at location gpr3
    mr         r9,r3
    addi        r9,r9,-4                # Index buffer1
    xor         r3,r3,r3        # clear r3

more_words:
    addi        r9,r9,4                 # Index buffer1
    lwz         r6,0(r9)                # Load 4 bytes from buffer1
    cmplw         1,r6,r7               # Compare 4 bytes
    bne         cr1,error_ret_word	# branch if r6 != r7
    bc          16, 0, more_words      # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return

    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_word:
    mfctr       r8
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13
    cmpi           1,1,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, no_trap_word         # Trap flag set to 0, dont trap to KDB

    cmpi           1,1,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, trap_word         # Trap flag set to 1, trap to KDB

    cmpi           1,1,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, attn_word         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here

attn_word:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

trap_word:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here

no_trap_word:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8
    
    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
#   lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
    lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr

    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .comp_word,.-.comp_word

    .align 2
    .globl comp_byte
    .section    ".opd","aw"
    .align 3
comp_byte:
    .quad    .comp_byte,.TOC.@tocbase,0
    .previous
    .size    comp_byte,24
    .type    .comp_byte,@function
    .globl    .comp_byte
.comp_byte:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)


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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6

    cmpi     1 , 1, r5, 0x0000     # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_byte       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.


    lbz         r7, 0(r4)               # Load 4 byte bit pattern
    lis         r20,gpr3@ha
    std         r3, gpr3@l(r20)        # Save r3 at location gpr3
    mr         r9,r3
    addi        r9,r9,-1                # Index buffer1
    xor         r3,r3,r3        # clear r3

more_bytes:
    addi        r9,r9,1                 # Index buffer1
    lbz         r6,0(r9)                # Load 1 byte from buffer1
    cmplw       1,r6,r7               # Compare 4 bytes
    bne         cr1,error_ret_byte           # branch if r6 != r7
    bc          16, 0, more_bytes      # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return

    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_byte:
    mfctr       r8
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13
    cmpi           1,1,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, no_trap_byte         # Trap flag set to 0, dont trap to KDB

    cmpi           1,1,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, trap_byte         # Trap flag set to 1, trap to KDB

    cmpi           1,1,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, attn_byte         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here

attn_byte:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

trap_byte:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here

no_trap_byte:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8
    
    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
#   lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
    lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr

    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .comp_byte,.-.comp_byte

    .align 2
    .globl comp_dword_addr
    .section    ".opd","aw"
    .align 3
comp_dword_addr:
    .quad    .comp_dword_addr,.TOC.@tocbase,0
    .previous
    .size    comp_dword_addr,24
    .type    .comp_dword_addr,@function
    .globl    .comp_dword_addr
.comp_dword_addr:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)


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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6

    cmpi     1 , 1, r5, 0x0000     # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_addr       # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.


#    ld         r7, 0(r4)               # Load 4 byte bit pattern
    lis         r20,gpr3@ha
    std         r3, gpr3@l(r20)        # Save r3 at location gpr3
    mr         r9,r3
    addi        r9,r9,-8                # Index buffer1
    xor         r3,r3,r3        # clear r3

more_dwords_addr:
    addi        r9,r9,8                 # Index buffer1
    ld         r6,0(r9)                # Load 8 bytes from buffer1
    or         r7,r9,r9
    cmpd         1,r6,r7               # Compare 8 bytes
    bne         cr1,error_ret_addr           # branch if r6 != r7
    bc          16, 0, more_dwords_addr      # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return

    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_addr:
    mfctr       r8
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13
    cmpi           1,1,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, no_trap_addr         # Trap flag set to 0, dont trap to KDB

    cmpi           1,1,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, trap_addr         # Trap flag set to 1, trap to KDB

    cmpi           1,1,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, attn_addr         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here

attn_addr:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

trap_addr:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here

no_trap_addr:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8
    
    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
#   lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
    lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr

    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .comp_dword_addr,.-.comp_dword_addr



    .align 2
    .globl wr_addr_cmp_dword
    .section    ".opd","aw"
    .align 3
wr_addr_cmp_dword:
    .quad    .wr_addr_cmp_dword,.TOC.@tocbase,0
    .previous
    .size    wr_addr_cmp_dword,24
    .type    .wr_addr_cmp_dword,@function
    .globl    .wr_addr_cmp_dword
.wr_addr_cmp_dword:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)

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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6
    cmpi     1, 1, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_wr_addr            # Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

    ld      r6, 0(r3)                   # Load 8 bytes from buffer1
    ld      r7, 0(r4)                   # Load 8 byte bit pattern
    lis            r20,gpr3@ha
    std     r3, gpr3@l(r20)
    mr r9,r3
    addi    r9,r9,-8                    # Index buffer1
    addi    r3, 0, 0                    # Set GPR-3 to zero for dcbf
more_cmp_dwords_addr:
    addi    r9,r9,8                     # Index buffer1
    or      r7,r9,r9                    # mov Address value in r9 into r7 
    std     r7,0(r9)                    # Store 8 bytes to buffer
    dcbf    r3, r9                      #dcbf

    ld      r6,0(r9)                    # Load 8 bytes from buffer1
    cmpd    1,  r6,r7                   # Compare 8 bytes
    bne     cr1,error_ret_wr_addr               # branch if r6 != r7
    bc      16, 0, more_cmp_dwords_addr           # Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_wr_addr:   
    mfctr   r8  
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13

    cmpi           1,0,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, wr_no_trap_addr         # Trap flag set to 0, dont trap to KDB

    cmpi           1,0,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, wr_no_trap_addr         # Trap flag set to 1, trap to KDB

    cmpi           1,0,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, wr_attn_addr         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here
wr_attn_addr:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

wr_trap_addr:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here
wr_no_trap_addr:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8

    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
    lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
#   lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr
    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .wr_addr_cmp_dword,.-.wr_addr_cmp_dword

    .align 2
    .globl wr_cmp_word
    .section    ".opd","aw"
    .align 3
wr_cmp_word:
    .quad    .wr_cmp_word,.TOC.@tocbase,0
    .previous
    .size    wr_cmp_word,24
    .type    .wr_cmp_word,@function
    .globl    .wr_cmp_word
.wr_cmp_word:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)

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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6
    cmpi     1, 1, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_wr_word	# Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

    ld      r6, 0(r3)                   # Load 8 bytes from buffer1
    lwz     r7, 0(r4)                   # Load 8 byte bit pattern
    lis     r20,gpr3@ha
    std     r3, gpr3@l(r20)
    mr r9,r3
    addi    r9,r9,-4                    # Index buffer1
    addi    r3, 0, 0                    # Set GPR-3 to zero for dcbf
more_cmp_words:
    addi    r9,r9,4                     # Index buffer1
    stw     r7,0(r9)                    # Store 4 bytes to buffer
    dcbf    r3, r9                      #dcbf

    lwz      r6,0(r9)                    # Load 8 bytes from buffer1
    cmplw    1,  r6,r7                   # Compare 8 bytes
    bne     cr1,error_ret_wr_word        # branch if r6 != r7
    bc      16, 0, more_cmp_words	# Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_wr_word:   
    mfctr   r8  
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13

    cmpi           1,0,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, wr_no_trap_word     # Trap flag set to 0, dont trap to KDB

    cmpi           1,0,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, wr_no_trap_word       # Trap flag set to 1, trap to KDB

    cmpi           1,0,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, wr_attn_word         # Trap flag set to 2, call attn 
    .long 0                 # should not have reached here
wr_attn_word:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

wr_trap_word:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here
wr_no_trap_word:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8

    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
    lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
#   lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr
    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .wr_cmp_word,.-.wr_cmp_word

    .align 2
    .globl wr_cmp_byte
    .section    ".opd","aw"
    .align 3
wr_cmp_byte:
    .quad    .wr_cmp_byte,.TOC.@tocbase,0
    .previous
    .size    wr_cmp_byte,24
    .type    .wr_cmp_byte,@function
    .globl    .wr_cmp_byte
.wr_cmp_byte:
    std 31,-8(1)
    stdu 1,-64(1)
    mr 31,1
    mr 0,3
    mr 9,4
    mr 11,5
    mr 10,6
    mr 12,7
    std 0,112(31)
    mr 0,9
    std 0,120(31)
    mr 0,11
    std 0,128(31)
    mr 0,10
    std 0,136(31)
    mr 0,12
    std 0,144(31)

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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr        9,  r5
    lis            r8,gpr20@ha
    std         r20,gpr20@l(r8) # Save orginal value of r20 in gpr20
    lis            r20,gpr13@ha
    std         r13,gpr13@l(r20)# Save orginal value of r13 in gpr13
    lis            r8,gpr7@ha
    std         r7,gpr7@l(r8)   # Save orginal value of r7 in gpr7
    lis            r20,gpr6@ha     # use r20
    std            r6,gpr6@l(r20)  # save the trap_flag at memory gpr6
    cmpi     1, 1, r5, 0x0000           # Compare num dwords to 0 and place
                                        # results into condition register
                                        # field 1.

    bc       4, 5, error_ret_wr_byte	# Using bit 5 of CR (>0) branch if bit
                                        # is cleared.  They either passed 0
                                        # or negative length.

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r5

    ld      r6, 0(r3)                   # Load 8 bytes from buffer1
    lbz     r7, 0(r4)                   # Load 1 byte from bit pattern
    lis     r20,gpr3@ha
    std     r3, gpr3@l(r20)
    mr r9,r3
    addi    r9,r9,-1                    # Index buffer1
    addi    r3, 0, 0                    # Set GPR-3 to zero for dcbf
more_cmp_bytes:
    addi    r9,r9,1                     # Index buffer1
    stb	    r7,0(r9)                    # Store a byte to buffer
    dcbf    r3, r9                      #dcbf

    lbz     r6,0(r9)                    # Load a byte from buffer1
    cmplw   1,  r6,r7                   # Compare 4 bytes
    bne     cr1,error_ret_wr_byte	# branch if r6 != r7
    bc      16, 0, more_cmp_bytes	# Loop until CTR value is 0

    addi    r3, 0, 0                    # Set GPR-3 to zero for return
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)  # Restore r20 from gpr20 
    ld 1,0(1)
    ld 31,-8(1)
    blr

error_ret_wr_byte:   
    mfctr   r8  
    sub            r3,r5,r8                 # r3 represents the double word that had miscompare
    addi           r3,r3,1                  # Increment r3 by 1, to cover the case where miscompare is in 0th word.
    lis            r20,gpr6@ha
    ld            r13,gpr6@l(r20)          # Get the trap_flag from memory to r13

    cmpi           1,0,r13,0x0000        # Compare r13 with 0 and load results in CR1
    bc        12, 6, wr_no_trap_byte	# Trap flag set to 0, dont trap to KDB

    cmpi           1,0,r13,0x0001        # Compare r13 with 1 and load results in CR1
    bc        12, 6, wr_no_trap_byte	# Trap flag set to 1, trap to KDB

    cmpi           1,0,r13,0x0002        # Compare r13 with 2 and load results in CR1
    bc        12, 6, wr_attn_byte	# Trap flag set to 2, call attn 
    .long 0                 # should not have reached here
wr_attn_byte:
    lis     r20,gpr23@ha
    ld     r3, gpr23@l(r20)               # Get the trap_flag from memory to r13
    ld    4, 112(31)             # restore shared memory pointer, r3
    ld    5, 128(31)             # restore number of dwords, r5
    ld    7, 120(31)             # restore bit pattern pointer, r4
    sub    r6, r5, r8             # Number of dwords left to compare
    .long 0x200                 # Call attention

wr_trap_byte:
    lis            r4,gpr20@ha
    ld            r20,gpr20@l(r4)       # Restore r20 from gpr20 
    lis            r4,gpr13@ha
    ld            r13,gpr13@l(r4)       # Restore r13 from gpr13 
    ld 1, 0(1)
    ld 31,-8(1)
    blr                     # will call do_trap_htx from c function
    .long 0                     # should not have reached here
wr_no_trap_byte:
    lis            r20,gpr8@ha
    std            r8,gpr8@l(r20)           # save r8 in gpr8

    lis            r13,gpr0@ha
    std     r0, gpr0@l(r13)
    lis            r13,gpr1@ha
    std     r1, gpr1@l(r13)
    lis            r13,gpr2@ha
    std     r2, gpr2@l(r13)
    lis            r13,gpr3@ha      #storing the starting location of shared seg into gpr8 from gpr3 
    ld     r20, gpr3@l(r13)     
    lis            r13,gpr8@ha
    std    r20,  gpr8@l(r13)
    lis            r13,gpr23@ha     #loading BEEFDEAD into gpr3  from gpr23
    ld     r20,  gpr23@l(r13)
    lis            r13,gpr3@ha
    std    r20,  gpr3@l(r13)
    lis            r13,gpr4@ha
    std     r4, gpr4@l(r13)
    lis            r13,gpr5@ha
    std     r5, gpr5@l(r13)
#   lis            r13,gpr6@ha
#   std     r6, gpr6@l(r13)     # Already saved
    lis            r13,gpr7@ha
    std     r7, gpr7@l(r13)
    lis            r13,gpr9@ha
    std     r9, gpr9@l(r13)
    lis            r13,gpr10@ha
    std     r10, gpr10@l(r13)
    lis            r13,gpr11@ha
    std     r11, gpr11@l(r13)
    lis            r13,gpr12@ha
    std     r12, gpr12@l(r13)
    lis            r13,gpr13@ha
    #std     r13, gpr13                                 # Already stored
    lis            r13,gpr14@ha
    std     r14, gpr14@l(r13)
    lis            r13,gpr15@ha
    std     r15, gpr15@l(r13)
    lis            r13,gpr16@ha
    std     r16, gpr16@l(r13)
    lis            r13,gpr17@ha
    std     r17, gpr17@l(r13)
    lis            r13,gpr18@ha
    std     r18, gpr18@l(r13)
    lis            r13,gpr19@ha
    std     r19, gpr19@l(r13)
    lis            r13,gpr20@ha
    std     r20, gpr20@l(r13)
    lis            r13,gpr21@ha
    std     r21, gpr21@l(r13)
    lis            r13,gpr22@ha
    std     r22, gpr22@l(r13)
    lis            r13,gpr23@ha
#   std     r23, gpr23@l(r13)
#   lis            r13,gpr24@ha
    std     r24, gpr24@l(r13)
    lis            r13,gpr25@ha
    std     r25, gpr25@l(r13)
    lis            r13,gpr26@ha
    std     r26, gpr26@l(r13)
    lis            r13,gpr27@ha
    std     r27, gpr27@l(r13)
    lis            r13,gpr28@ha
    std     r28, gpr28@l(r13)
    lis            r13,gpr29@ha
    std     r29, gpr29@l(r13)
    lis            r13,gpr30@ha
    std     r30, gpr30@l(r13)
    lis            r13,gpr31@ha
    std     r31, gpr31@l(r13)
         
    lis            r13,gpr13@ha
    ld            r13,gpr13@l(r13)         # restore r13 from gpr13
    lis            r20,gpr20@ha
    ld            r20,gpr20@l(r20)          # restore r20 from gpr20

    ld 1,0(1)
    ld 31,-8(1)
    blr
    .long 0
    .byte 0,0,0,0,128,1,0,1
    .size    .wr_cmp_byte,.-.wr_cmp_byte

#DATA Section 
    .globl  Gprs
    .globl  Gprs1
_wr_cmp_dword:
#Gprs1:
    .section ".data"
    .align 4
Gprs1:
Gprs:
gpr0:   .long   0x00000000
        .long   0x00000000
gpr1:   .long   0x00000000
        .long   0x00000000
gpr2:   .long   0x00000000
        .long   0x00000000
gpr3:   .long   0x00000000
        .long   0x00000000
gpr4:   .long   0x00000000
        .long   0x00000000
gpr5:   .long   0x00000000
        .long   0x00000000
gpr6:   .long   0x00000000
        .long   0x00000000
gpr7:   .long   0x00000000
        .long   0x00000000
gpr8:   .long   0x00000000
        .long   0x00000000
gpr9:   .long   0x00000000
        .long   0x00000000
gpr10:  .long   0x00000000
        .long   0x00000000
gpr11:  .long   0x00000000
        .long   0x00000000
gpr12:  .long   0x00000000
        .long   0x00000000
gpr13:  .long   0x00000000
        .long   0x00000000
gpr14:  .long   0x00000000
        .long   0x00000000
gpr15:  .long   0x00000000
        .long   0x00000000
gpr16:  .long   0x00000000
        .long   0x00000000
gpr17:  .long   0x00000000
        .long   0x00000000
gpr18:  .long   0x00000000
        .long   0x00000000
gpr19:  .long   0x00000000
        .long   0x00000000
gpr20:  .long   0x00000000
        .long   0x00000000
gpr21:  .long   0x00000000
        .long   0x00000000
gpr22:  .long   0x00000000
        .long   0x00000000
gpr23:  .long   0xBEEFDEAD
        .long   0xFFFFFFFF
gpr24:  .long   0x00000000
        .long   0x00000000
gpr25:  .long   0x00000000
        .long   0x00000000
gpr26:  .long   0x00000000
        .long   0x00000000
gpr27:  .long   0x00000000
        .long   0x00000000
gpr28:  .long   0x00000000
        .long   0x00000000
gpr29:  .long   0x00000000
        .long   0x00000000
gpr30:  .long   0x00000000
        .long   0x00000000
gpr31:  .long   0x00000000
        .long   0x00000000

    .ident    "GCC: (GNU) 3.2.3 20030329 (prerelease)"
