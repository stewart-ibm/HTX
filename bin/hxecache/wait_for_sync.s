.section ".text"
.global  wait_for
.type wait_for,@function
.align 2        
wait_for:
.set SYNC_WORD,  3

# This function checks and waits till the sync word has become zero

wait_loop:                   
                            
        lwz 4, 0(SYNC_WORD) # Load the word whose address is in reg 3 into reg 4
        cmpwi 4, 0x0	    # Compare it with 0
        bne wait_loop	    # If not zero then loop
        blr                 # Return
