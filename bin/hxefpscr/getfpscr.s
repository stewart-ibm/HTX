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
# @(#)43        1.1.4.1  src/htx/usr/lpp/htx/bin/hxefpscr/getfpscr.s, exer_fpscr, htxubuntu 10/28/13 04:41:02
#########################################################################

gcc2_compiled.:
.section		".text"
.align			2
.globl			getfpscr
.type			getfpscr,@function

getfpscr:
        .set    g0,0
        .set    rf0,0
        .set    g1,1
        .set    rf1,1
        .set    g2,2
        .set    rf2,2
        .set    g3,3
        .set    rf3,3
        .set    g4,4
        .set    rf4,4
        .set    g5,5
        .set    rf5,5
        .set    g6,6
        .set    rf6,6
        .set    g7,7
        .set    rf7,7
        .set    g8,8
        .set    rf8,8
        .set    g9,9
        .set    rf9,9
        .set    g10,10
        .set    rf10,10
        .set    g11,11
        .set    rf11,11
        .set    g12,12
        .set    rf12,12
        .set    g13,13
        .set    rf13,13
        .set    g14,14
        .set    rf14,14
        .set    g15,15
        .set    rf15,15
        .set    g16,16
        .set    rf16,16
        .set    g17,17
        .set    rf17,17
        .set    g18,18
        .set    rf18,18
        .set    g19,19
        .set    rf19,19
        .set    g20,20
        .set    rf20,20
        .set    g21,21
        .set    rf21,21
        .set    g22,22
        .set    rf22,22
        .set    g23,23
        .set    rf23,23
        .set    g24,24
        .set    rf24,24
        .set    g25,25
        .set    rf25,25
        .set    g26,26
        .set    rf26,26
        .set    g27,27
        .set    rf27,27
        .set    g28,28
        .set    rf28,28
        .set    g29,29
        .set    rf29,29
        .set    g30,30
        .set    rf30,30
        .set    g31,31
        .set    rf31,31
#
        mffs    rf1             	# Get current fpscr
        stfd    rf1,0(g3)		# G3 has address of FPSCR (64-bit). Just store FRP1 there.
        blr
#
