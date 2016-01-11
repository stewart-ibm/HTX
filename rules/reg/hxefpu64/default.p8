* Rule file for hxefpu64 exerciser running on P7.
* For details of rule keywords,please refer README.hxefpu64
* in /usr/lpp/htx/rules/reg/hxefpu64 dir on test machine.
*
* NOTE:Rule keyword & associated value should be all in one
*	line _only_. Currently '\n' (newline) is used as
*	delimiter to differentiate between keywords.
*
*

* All VSX instructions on all SMT threads with normal data

rule_id						test1
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* All VSX instructions with 50% normal & 50% denormal data zones

rule_id						test2
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* All VSX instructions with 100% denormal data zones

rule_id						test3
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* All VSX instructions with mixed data zones

rule_id						test4
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* Bypass-MIX1 stanza <=> T0-3:VSX and integer interleaved
* VSX all instructions - 50%.
* VMX MISC (permute,shift,splat,merge,pack/unpack) - 50%
* with 50/50 normal/denormal

rule_id						test5
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask               [(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff, 50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff, 50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50)]

* Bypass-MIX2 stanza <=> T0-3:BFP and integer interleaved
* BFP all instruction - 50%.
* VMX MISC (permute,shift,splat,merge,pack/unpack) - 50% {PM class - integer}.
* and 50/50 normal/denormal data

rule_id						test6
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50)]

* FPSCR-BFU stanza.
* All BFP with FPSCR move to/from interleaved and 50/50 normal/denormal data

rule_id						test7
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20)]

* FPSCR-VSX stanza.
* All VSX with FPSCR move to/from interleaved and 50/50 normal/denormal data

rule_id						test8
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20)]

* Approximately equal stress on both pipes.
* DFP all instruction - 20% (p1 only)
* VMX FP all instruction - 20% (p0 only)
* VMX MISC (permute,shift,splat,merge,pack/unpack) - 20% (p1 only)
* VMX INT all instruction - 20% {p0 only)
* BFP all instruction - 10% (p0 n p1 both)
* VSX DP all instruction - 10% (p0 n p1 both) 
* with 50/50 normal/denormal data

rule_id						test9
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10):(0x030000001fffffff,20)(0x04000000007e0000,20)(0x0400000001000000,20)(0x040000000001FFF8,20)(0x020000000007ffff,10)(0x01000001fffff000,10)]

* All SMT threads stressing only pipe 1.
* DFP all instructions - 80%
* VSX MISC (logical,permute,splat,shift,select and merge) - 10%
* VMX MISC (permute,shift,splat,merge,pack/unpack) - 10%
* with norm/denorm data.

rule_id						test10
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10):(0x030000001fffffff,80)(0x0100000400000000,10)(0x0400000001000000,10)]

* All SMT threads stressing only pipe 0.
* VMX FP all instructions - 35%
* VMX INT all instructions - 35%
* BFP FPSCR all instructions - 30%
* with 50/50 norm/denorm data.
* Because of FPSCR instructions,pipeline will keep getting stalled.

rule_id						test11
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30):(0x04000000007e0000,35)(0x040000000001fff8,35)(0x0200000000020000,30)]

* All VSX instructions with 50% normal & 50% denormal data with all rounding modes.

rule_id						test12
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,1,2,3,0,1,2,3]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* VSX load/store all with alternate threads accessing unaligned memory location.

rule_id						test13
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[80,10,80,10,80,10,80,10]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100):(0x010000180000f003,100)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with normal data

rule_id						test14
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with denormal data

rule_id						test15
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with normal & denormal data

rule_id						test16
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX  instructions on all SMT threads with mixed data

rule_id						test17
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask			 	[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* Stress Stanza. Sub-part of integer (XS) instructions only.
* XS includes following classes (with VMX_INT having mul/div extra - Need to remove that)
*	(1) VMX_INT_ARITHMETIC_SIGNED_ONLY | VMX_INT_ARITHMETIC_UNSIGNED_ONLY
*	(2) VMX_INT_AVERAGE_SIGNED_ONLY | VMX_INT_AVERAGE_UNSIGNED_ONLY
*	(3) VMX_INT_MAX_MIN_SIGNED_ONLY | VMX_INT_MAX_MIN_UNSIGNED_ONLY
*	(4) VMX_INT_CMP_UNSIGNED_ONLY | VMX_INT_CMP_SIGNED_ONLY
*	(5) VMX_INT_LOGICAL_ONLY
*	(6) VMX_INT_ROTATE_SHIFT_ONLY
*	(7) VMX_FP_CMP_ONLY
*	(8) VMX_FP_MAX_MIN_ONLY
*	(9) VMX_VSCR_ONLY


rule_id						test18
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100):(0x0400000000a9fe18,100)]

* Pipe1-lock Stanza. Sub-part of integer (PM) instructions and DFP.
* DFP 70% & VMX MISC 30% on all threads with mixed data corners.

rule_id						test19
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30):(0x030000001fffffff,70)(0x0400000001000000,30)]

* DFP 70% & VSX_Misc 30% on all threads with mixed data corners. (correctness)

rule_id						test20
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30)]

* VSX Vector Arithmetic 70% & VSX_Misc 30% on all threads with mixed data corners. (correctness)

rule_id						test21
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30)]

* Data Security shift with 100% normal data

rule_id						test22
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with 50/50% normal/denormal data

rule_id						test23
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with 100% denorm data.

rule_id						test24
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with mix of all data corners

rule_id						test25
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* VSX instructions with integer interleaved with 50/50 normal/denormal (correctness)
* VSX Instructions - 50%
* VMX_MISC (integer) - 50%

rule_id						test26
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask               [(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff, 50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50):(0x0100001fffffffff, 50)(0x040000000301fff8,50):(0x0100001fffffffff,50)(0x040000000301fff8,50)]


* All BFP instructions with integer interleaved and 50/50 normal/denormal data (correctness)
* BFP Instructions - 50%
* VMX_MISC (integer) - 50%

rule_id						test27
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50):(0x020000000007ffff,50)(0x040000000301fff8,50)]

* All BFP with FPSCR move to/from interleaved and 50/50 normal/denormal data (correctness)

rule_id						test28
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20):(0x020000000005ffff,80)(0x0200000000020000,20)]

* All VSX with FPSCR move to/from interleaved and 50/50 normal/denormal data (correctness)

rule_id						test29
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20):(0x0100001fffffffff,80)(0x0200000000020000,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with normal data (correctness)

rule_id						test30
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with denormal data (correctness)

rule_id						test31
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with normal & denormal data (correctness)

rule_id						test32
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* VSX,BFP,DFP & VMX instructions on all SMT threads with mixed data (correctness)

rule_id						test33
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20):(0x0100001fffffffff,40)(0x020000000007ffff,20)(0x030000001fffffff,20)(0x0400000003ffffff,20)]

* All VSX instructions on all SMT threads with normal data (correctness)

rule_id						test34
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* All VSX instructions with mixed data zones (correctness)

rule_id						test35
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* VMX only. With de-norm data only.

rule_id				test36
num_oper			500
num_threads			0
seed				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth			2000
test_method			2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask			[0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0]
ins_bias_mask			[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]


* VMX only. With norm and de-norm data.

rule_id				test37
num_oper			500
num_threads			0
seed				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth			2000
test_method			2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask			[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask			[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]


* VSX only. With norm and de-norm data.

rule_id				test38
num_oper			500
num_threads			0
seed				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth			2000
test_method			2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask			[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask			[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]


* 50% VSX and 50% VMX. With norm and de-norm data.

rule_id				test39
num_oper			500
num_threads			0
seed				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth			2000
test_method			2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask			[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask			[(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50)]


* 50% VSX+VMX and 50% rest of all instructions. With norm and de-norm data.

rule_id				test40
num_oper			500
num_threads			0
seed				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth			2000
test_method			2
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr				[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask			[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask			[(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25)]

