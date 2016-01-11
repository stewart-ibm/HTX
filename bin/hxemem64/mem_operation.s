##############################################################################
#
#  FUNCTION: mem_operation
#
#  DESCRIPTION:  stores & Compares buffer1 with 8 byte bit pattern
#
#  CALLING SEQUENCE: mem_operation(code,counter,dest_ptr,pattern_ptr,trap_flag,\
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
#
#
##############################################################################
    .file   "mem_operation.c"
    .section    ".text"
#    .align 2
#    .section    ".opd","aw"
    .align 3
    .globl mem_operation
mem_operation:
#    .quad   .mem_operation,.TOC.@tocbase,0
#    .previous
#    .size   mem_operation,24
    .type   .mem_operation,@function
    .globl  .mem_operation
.mem_operation:

    .set            r1,1
    .set            r2,2
    .set            r3,3
    .set            r4,4
    .set            r5,5
    .set            r6,6
    .set            r7,7
    .set            r8,8
    .set            r9,9

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

    # Move the number of dword transfers into the count register (9). The
    # bc instruction below will decrement the count register during each
    # iteration of the loop.

    mtspr   9,   r4
    
    #move Starting Address value from r5 to r7
    or         r7,r5,r5   # r7 = r5 


    cmpi    1,1, r3, 0x0001
    bne     cr1,comp_mem_4              # if (r3 <code> == 1 ) then do MEM operation width = 8 
    ld      r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-8                    # Store buffer
mem_dword:
    addi    r7,r7,8                     # Store Buffer incremented 
    std     r6,0(r7)                    # Store 8 bytes to buffer
    bc      16, 0, mem_dword            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_mem_4:
    cmpi    1,1, r3, 0x0002
    bne     cr1,comp_mem_1              # if (r3 <code> == 2 ) then do MEM operation width = 4 
    lwz     r6, 0(r6)                   # Load 4 bytes of bit pattern
    addi    r7,r7,-4                    # Store buffer
mem_word:
    addi    r7,r7,4                     # Store Buffer incremented
    stw     r6,0(r7)                    # Store 8 bytes to buffer
    bc      16, 0, mem_word             # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_mem_1:
    cmpi    1,1, r3, 0x0003
    bne     cr1,comp_rim_8              # if (r3 <code> == 3 ) then do MEM operation width = 1
    lbz     r6, 0(r6)                   # Load 4 bytes of bit pattern
    addi    r7,r7,-1                    # Store buffer
mem_byte:
    addi    r7,r7,1                     # Store Buffer incremented
    stb     r6,0(r7)                    # Store 8 bytes to buffer
    bc      16, 0, mem_byte             # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_rim_8:
    cmpi    1,1, r3, 0x0004
    bne     cr1,comp_rim_4              # if (r3 <code> == 4 ) then do RIM operation width = 8
    ld      r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-8                    # Store buffer Pointer intialized
    xor     r3,r3,r3                    # make r3 = 0 for dcbf
rim_dword:
    addi    r7,r7,8                     # Store Buffer incremented
    std     r6,0(r7)                    # Store 8 bytes to buffer
    dcbf    r3,r7                       # dcbf
    ld      r5,0(r7)                    # Load 8 bytes from store buffer
    cmpld   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, rim_dword            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_rim_4:
    cmpi    1,1, r3, 0x0005
    bne     cr1,comp_rim_1              # if (r3 <code> == 5 ) then do RIM operation width = 4
    lwz     r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-4                    # Store buffer Pointer intialized
    xor     r3,r3,r3                    # make r3 = 0 for dcbf
rim_word:
    addi    r7,r7,4                     # Store Buffer incremented
    stw     r6,0(r7)                    # Store 8 bytes to buffer
    dcbf    r3,r7                       # dcbf
    lwz     r5,0(r7)                    # Load 8 bytes from store buffer
    cmplw   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, rim_word             # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_rim_1:
    cmpi    1,1, r3, 0x0006
    bne     cr1,comp_dword              # if (r3 <code> == 6 ) then do RIM operation width = 1
    lbz     r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-1                    # Store buffer Pointer intialized
    xor     r3,r3,r3                    # make r3 = 0 for dcbf
rim_byte:
    addi    r7,r7,1                     # Store Buffer incremented
    stb     r6,0(r7)                    # Store 8 bytes to buffer
    dcbf    r3,r7                       # dcbf
    lbz     r5,0(r7)                    # Load 8 bytes from store buffer
    cmplw   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, rim_byte             # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_dword:
    cmpi    1,1, r3, 0x0007
    bne     cr1,comp_word               # if (r3 <code> == 7 ) then do Comparision for width = 8
    ld      r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-8                    # Store buffer Pointer intialized
dword_comp:
    addi    r7,r7,8                     # Store Buffer incremented
    ld      r5,0(r7)                    # Load 8 bytes from store buffer
    cmpld   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, dword_comp           # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_word:
    cmpi    1,1, r3, 0x0008
    bne     cr1,comp_byte               # if (r3 <code> == 8 ) then do Comparision for width = 4
    lwz     r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-4                    # Store buffer Pointer intialized
word_comp:
    addi    r7,r7,4                     # Store Buffer incremented
    lwz     r5,0(r7)                    # Load 8 bytes from store buffer
    cmplw   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, word_comp            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_byte:
    cmpi    1,1, r3, 0x0009
    bne     cr1,write_addr              # if (r3 <code> == 9 ) then do Comparision for width = 1
    lbz     r6, 0(r6)                   # Load 8 byte bit pattern
    addi    r7,r7,-1                    # Store buffer Pointer intialized
byte_comp:
    addi    r7,r7,1                     # Store Buffer incremented
    lbz     r5,0(r7)                    # Load 8 bytes from store buffer
    cmplw   1, r5,r6                    # Compare 8 bytes
    bne     cr1,error_ret               # branch if r5 != r6
    bc      16, 0, byte_comp            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

write_addr:
    cmpi    1,1, r3, 0x000a
    bne     cr1,comp_addr               # if (r3 <code> == 10 (0xa)) then do WRITE ADDRESS
    addi    r7,r7,-8                    # Store buffer
addr_dword:
    addi    r7,r7,8                     # Store Buffer incremented
    std     r7,0(r7)                    # Store 8 bytes to buffer
    bc      16, 0, addr_dword           # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

comp_addr:
    cmpi    1,1, r3, 0x000b
    bne     cr1,wr_addr_comp            # if (r3 <code> == 11(0xb)) then do COMP ADDRESS 
    addi    r7,r7,-8                    # Store buffer
addr_comp:
    addi    r7,r7,8                     # Store Buffer incremented
    ld      r5,0(r7)                    # Store 8 bytes to buffer
    cmpld     1,r7,r5                   # Compare the value and address
    bne     cr1,error_ret               # branch if r5 != r7 
    bc      16, 0, addr_comp            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

wr_addr_comp:
    cmpi    1,1, r3, 0x000c
    bne     cr1,dword_read				# if (r3 <code> == 12(0xc)) then do WR RD COMP ADDRESS
    xor     r3,r3,r3                    # r3 = 0 for dcbf
    addi    r7,r7,-8                    # Store buffer
addr_wr_comp:
    addi    r7,r7,8                     # Store Buffer incremented
    std     r7,0(r7)                    # Store 8 bytes to buffer
    dcbf     r3,r7    
    ld      r5,0(r7)                    # Store 8 bytes to buffer
    cmpld   1,r7,r5                     # Compare the value and address
    bne     cr1,error_ret               # branch if r5 != r7
    bc      16, 0, addr_wr_comp         # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

dword_read:
    cmpi    1,1, r3, 0x000d
    bne     cr1,word_read				# if (r3 <code> == 13 ) then do read operation width = 8
    addi    r7,r7,-8                    # Read buffer
read_dword:
    addi    r7,r7,8                     # Read Buffer incremented
    ld      r6,0(r7)                    # Read 4 bytes from buffer
    bc      16, 0, read_dword           # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

word_read:
    cmpi    1,1, r3, 0x000e
    bne     cr1,byte_read 				# if (r3 <code> == 14 ) then do Read operation width = 4
    addi    r7,r7,-4                    # Read buffer
read_word:
    addi    r7,r7,4                     # Read Buffer incremented
    lwz     r6,0(r7)                    # Read 4 bytes from buffer
    bc      16, 0, read_word            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

byte_read:
    cmpi    1,1, r3, 0x000f
    bne     cr1,error_ret 				# if (r3 <code> == 15 ) then do Read operation width = 1
    addi    r7,r7,-1                    # Read buffer
read_byte:
    addi    r7,r7,1                     # Read Buffer incremented
    lbz     r6,0(r7)                    # Read 1 byte to buffer
    bc      16, 0, read_byte            # Loop until CTR value is 0
    xor     r3,r3,r3                    # Set GPR-3 to zero for return
    bclr    20, 0

error_ret:
    mfctr   r3
    sub     r4,r4,r3                    # r4 represents the dword/word/byte that had miscompare
    addi    r4,r4,1                     # Increment r4 by 1, to cover the case where miscompare is in 0th position.
    ld      r3,-32(r1)                # load trap flag into r3 from the stack
    cmpi    1,0,r3,0x0000
    bc        0xC,6,no_trap                
    cmpi    1,0,r3,0x0001               # If bit 6 in CR is zero, it means trap_flag=0. Go to no_trap
    bc      0xC,6,trap_kdb              # If bit 6 in CR is one, it means trap_flag=1. Go to trap_kdb    
    .long     0x200
trap_kdb:
    ld        r5,-16(r1)                # load shm starting pointer into r5 from the stack    
    ld        r6,-24(r1)                # load pattern buffer pointer into r6 from the stack    
    ld        r8,-40(r1)                # load structure pointer into r8 from the stack    
    ld        r9,-48(r1)                # load stanza pointer into r9 from the stack    
    addis     r3,0,0xBEEF               # load 0xBEEF into 32-47 bits in r3
    ori       r3,r3,0xDEAD              # load 0xDEAD into 48-63 bits in r3
    tw        4,r1,r1                     # Enter KDB
no_trap:
    xor     r3,r3,r3
    or      r3,r3,r4                 # r3 = r4 (no of dword/word/byte compared)
    bclr    20, 0

#    .long 0
#    .byte 0,0,0,0,128,1,0,1
#    .size   .mem_operation,.-.mem_operation
