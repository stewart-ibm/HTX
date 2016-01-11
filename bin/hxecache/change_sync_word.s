.section ".text"
.global change
.type change,@function
.align 2        
change:
.set SYNC_WORD,   3
.set CPU_ID_MASK, 4

# This function is used to turn off the id bit of the running thread in the
# Sync mask, by complementing the CPU_ID_MASK of the thread and then performing
# a logical AND with the SYNCWORD.

again:		                     # Stream synchronization code
        lwarx   0,0,SYNC_WORD        # Turn off my id bit in sync word
        andc    0,0,CPU_ID_MASK      # in an atomic manner
        stwcx.  0,0,SYNC_WORD	   	
        bne     again                # loop if reservation lost
        sync	
        isync
        blr                          # Return
