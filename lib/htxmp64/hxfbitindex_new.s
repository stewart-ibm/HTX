# @(#)72	1.1  src/htx/usr/lpp/htx/lib/htxmp64/hxfbitindex_new.s, htx_libhtxmp, htxubuntu 9/22/10 07:32:48
#
#
# COMPONENT_NAME: (HTXLIB) HTX Libraries
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business machines Corp. 1991
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#############################################################################

#*********************************************************
# scan memory for a non-zero bit and return its bit index*
#                                                        *
# input:  r3=address of start of scan                    *
#                                                        *
# output: r3=zero based index of the first one bit       *
#                                                        *
# N.B. This routine assumes that the input pointer is    *
#      word aligned and that a one bit will be found     *
#********************************************************
#
        S_PROLOG(bitindex)
        l       r4,0(r3)        #lwz Load first word - assume r3 word aligned
        ai      r6,r3,0         #addic Move address to r6 - free instruction under the load
        cmpli   0,0,r4,0          #Check for zero - cntlz doesn't do that
        cntlz   r3,r4           #cntlzw Free inst. waiting for cmpli to get to branch unit
        bner                    #bnelr Return if first word is it
bitlp:
        lu      r4,4(r6)        #lwzu Load next word
        cmpli   0,0,r4,0          #Compare first since cntlz doesn't do the right thing
        cntlz   r5,r4           #Free while waiting for compare
        a       r3,r3,r5        #addc Free - add number of bits to 1 or 32 if word is zero
        beq     bitlp           #This word zero - try again
        br
        FCNDES(bitindex)

