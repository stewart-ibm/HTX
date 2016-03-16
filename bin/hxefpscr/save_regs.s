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
# @(#)45        1.1.4.1  src/htx/usr/lpp/htx/bin/hxefpscr/save_regs.s, exer_fpscr, htxubuntu 10/28/13 04:41:08
#########################################################################

gcc2_compiled.:
.section	".text"
.align		2
.globl		save_regs
.type		save_regs,@function

save_regs:
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
        .set    g13,13
        .set    g14,14
        .set    g15,15
        .set    g16,16
        .set    g17,17
        .set    g18,18
        .set    g19,19
        .set    g20,20
        .set    g21,21
        .set    g22,22
        .set    g23,23
        .set    g24,24
        .set    g25,25
        .set    g26,26
        .set    g27,27
        .set    g28,28
        .set    g29,29
        .set    g30,30
        .set    g31,31


        .set    fr0,0
        .set    fr1,1
        .set    fr2,2
        .set    fr3,3
        .set    fr4,4
        .set    fr5,5
        .set    fr6,6
        .set    fr7,7
        .set    fr8,8
        .set    fr9,9
        .set    fr10,10
        .set    fr11,11
        .set    fr12,12
        .set    fr13,13
        .set    fr14,14
        .set    fr15,15
        .set    fr16,16
        .set    fr17,17
        .set    fr18,18
        .set    fr19,19
        .set    fr20,20
        .set    fr21,21
        .set    fr22,22
        .set    fr23,23
        .set    fr24,24
        .set    fr25,25
        .set    fr26,26
        .set    fr27,27
        .set    fr28,28
        .set    fr29,29
        .set    fr30,30
        .set    fr31,31


# 

	lis g5, fpr8@ha
	stfd g8, fpr8@l(g5)
	lis  g8, fpr5@ha
	stfd g5, fpr5@l(g8)

	lis 	g8, fpr0@ha 
        stfd    fr0,fpr0@l(g8)
	lis 	g8, fpr1@ha 
        stfd    fr1,fpr1@l(g8)
	lis 	g8, fpr2@ha 
        stfd    fr2,fpr2@l(g8)
	lis 	g8, fpr3@ha 
        stfd    fr3,fpr3@l(g8)
	lis 	g8, fpr4@ha 
        stfd    fr4,fpr4@l(g8)
	lis 	g8, fpr5@ha 
        stfd    fr5,fpr5@l(g8)
	lis 	g8, fpr0@ha 
        stfd    fr6,fpr6@l(g8)
	lis 	g8, fpr7@ha 
        stfd    fr7,fpr7@l(g8)
	lis 	g8, fpr8@ha 
        stfd    fr8,fpr8@l(g8)
	lis 	g8, fpr9@ha 
        stfd    fr9,fpr9@l(g8)
	lis 	g8, fpr10@ha 
        stfd    fr10,fpr10@l(g8)
	lis 	g8, fpr11@ha 
        stfd    fr11,fpr11@l(g8)
	lis 	g8, fpr12@ha 
        stfd    fr12,fpr12@l(g8)
	lis 	g8, fpr13@ha 
        stfd    fr13,fpr13@l(g8)
	lis 	g8, fpr14@ha 
        stfd    fr14,fpr14@l(g8)
	lis 	g8, fpr15@ha 
        stfd    fr15,fpr15@l(g8)
	lis 	g8, fpr16@ha 
        stfd    fr16,fpr16@l(g8)
	lis 	g8, fpr17@ha 
        stfd    fr17,fpr17@l(g8)
	lis 	g8, fpr18@ha 
        stfd    fr18,fpr18@l(g8)
	lis 	g8, fpr19@ha 
        stfd    fr19,fpr19@l(g8)
	lis 	g8, fpr20@ha 
        stfd    fr20,fpr20@l(g8)
	lis 	g8, fpr21@ha 
        stfd    fr21,fpr21@l(g8)
	lis 	g8, fpr22@ha 
        stfd    fr22,fpr22@l(g8)
	lis 	g8, fpr23@ha 
        stfd    fr23,fpr23@l(g8)
	lis 	g8, fpr24@ha 
        stfd    fr24,fpr24@l(g8)
	lis 	g8, fpr25@ha 
        stfd    fr25,fpr25@l(g8)
	lis 	g8, fpr26@ha 
        stfd    fr26,fpr26@l(g8)
	lis 	g8, fpr27@ha 
        stfd    fr27,fpr27@l(g8)
	lis 	g8, fpr28@ha 
        stfd    fr28,fpr28@l(g8)
	lis 	g8, fpr29@ha 
        stfd    fr29,fpr29@l(g8)
	lis 	g8, fpr30@ha 
        stfd    fr30,fpr30@l(g8)
	lis 	g8, fpr31@ha 
        stfd    fr31,fpr31@l(g8)

        blr
#
.section	".data"
.align		3
.globl		Fprs
.globl		Gprs
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
gpr23:  .long   0x00000000        
        .long   0x00000000
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

Fprs:
fpr0:   .long   0x00000000
        .long   0x00000000
fpr1:   .long   0x00000000
        .long   0x00000000
fpr2:   .long   0x00000000
        .long   0x00000000
fpr3:   .long   0x00000000
        .long   0x00000000
fpr4:   .long   0x00000000
        .long   0x00000000
fpr5:   .long   0x00000000
        .long   0x00000000
fpr6:   .long   0x00000000
        .long   0x00000000
fpr7:   .long   0x00000000
        .long   0x00000000
fpr8:   .long   0x00000000
        .long   0x00000000
fpr9:   .long   0x00000000
        .long   0x00000000
fpr10:  .long   0x00000000
        .long   0x00000000
fpr11:  .long   0x00000000
        .long   0x00000000
fpr12:  .long   0x00000000
        .long   0x00000000
fpr13:  .long   0x00000000
        .long   0x00000000
fpr14:  .long   0x00000000
        .long   0x00000000
fpr15:  .long   0x00000000
        .long   0x00000000
fpr16:  .long   0x00000000
        .long   0x00000000
fpr17:  .long   0x00000000
        .long   0x00000000
fpr18:  .long   0x00000000
        .long   0x00000000
fpr19:  .long   0x00000000
        .long   0x00000000
fpr20:  .long   0x00000000
        .long   0x00000000
fpr21:  .long   0x00000000
        .long   0x00000000
fpr22:  .long   0x00000000
        .long   0x00000000
fpr23:  .long   0x00000000
        .long   0x00000000
fpr24:  .long   0x00000000
        .long   0x00000000
fpr25:  .long   0x00000000
        .long   0x00000000
fpr26:  .long   0x00000000
        .long   0x00000000
fpr27:  .long   0x00000000
        .long   0x00000000
fpr28:  .long   0x00000000
        .long   0x00000000
fpr29:  .long   0x00000000
        .long   0x00000000
fpr30:  .long   0x00000000
        .long   0x00000000
fpr31:  .long   0x00000000
        .long   0x00000000

#
