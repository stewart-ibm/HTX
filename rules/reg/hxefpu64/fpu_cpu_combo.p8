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

rule_id						fpu1
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

rule_id						fpu2
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

rule_id						fpu3
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

rule_id						fpu4
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

rule_id						fpu5
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

rule_id						fpu6
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

rule_id						fpu7
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

rule_id						fpu8
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

rule_id						fpu9
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

rule_id						fpu10
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

rule_id						fpu11
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

rule_id						fpu12
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

rule_id						fpu13
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

rule_id						fpu14
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

rule_id						fpu15
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

rule_id						fpu16
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

rule_id						fpu17
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


rule_id						fpu18
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

rule_id						fpu19
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

rule_id						fpu20
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30):(0x030000001fffffff,70)(0x0100000400000000,30)]

* VSX Vector Arithmetic 70% & VSX_Misc 30% on all threads with mixed data corners. (correctness)

rule_id						fpu21
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30):(0x01000000000c0000,70)(0x0100000400000000,30)]

* Data Security shift with 100% normal data

rule_id						fpu22
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa,0xa,0xa,0xa,0xa,0xa,0xa,0xa]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with 50/50% normal/denormal data

rule_id						fpu23
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with 100% denorm data.

rule_id						fpu24
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0,0xa0]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]

* Data Security shift with mix of all data corners

rule_id						fpu25
num_oper					2000
num_threads					0
seed						[0,0,0,0,0,0,0,0]
stream_depth				1000
test_method					1
fpscr						[0,0,0,0,0,0,0,0]
data_bias_mask				[0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111,0x1111111111]
ins_bias_mask				[(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x0100000000000fff,50)(0x020000000007ffff,50):(0x020000000007ffff,100):(0x0100000000000fff,100):(0x0100000000000fff,50)(0x020000000007ffff,50)]


* VMX only. With de-norm data only.

rule_id						fpu26
num_oper					500
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0]
ins_bias_mask				[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]


* VMX only. With norm and de-norm data.

rule_id						fpu27
num_oper					500
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]


* 50% VSX and 50% VMX. With norm and de-norm data.

rule_id						fpu28
num_oper					500
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50)]


* 50% VSX+VMX and 50% rest of all instructions. With norm and de-norm data.

rule_id						fpu29
num_oper					500
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25):(0x0100001fffffffff,25)(0x020000000007ffff,25)(0x030000001fffffff,25)(0x0400000003ffffff,25)]

* All types of CPU Instructions except thread priority instructions
rule_id						cpu30
num_oper					5000 
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100):(0x05000000000037ff,100)]

* Only LOADS and STORES and Thread Priority Instructions
rule_id						cpu31
num_oper					5000 
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100):(0x0500000000003841,100)]

* Only ARITHMETIC and ROTATE instructions
rule_id						cpu32
num_oper					5000 
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100):(0x0500000000000014,100)]

* Only Logical and Conditional Logic Instructions
rule_id						cpu33
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100):(0x050000000000000A,100)]


* Only  cache,storage and external instructions
rule_id						cpu34
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100):(0x0500000000000380,100)]

* Only  Branch Instructions
rule_id						cpu35
num_oper					5000	
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK               [(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100):(0x0500000000000400,100)]

* VMX only. With de-norm data only.
rule_id						cpu36
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0]
ins_bias_mask				[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]

* VMX only. With norm and de-norm data.
rule_id						cpu37
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100):(0x0400000003ffffff,100)]

* VSX only. With norm and de-norm data.
rule_id						cpu38
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100):(0x0100001fffffffff,100)]

* 50% VSX and 50% VMX. With norm and de-norm data.
rule_id						cpu39
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50):(0x0100001fffffffff,50)(0x0400000003ffffff,50)]

* 50% VSX+VMX and 50% rest of all instructions. With norm and de-norm data.
rule_id						cpu40
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
data_bias_mask				[0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55]
ins_bias_mask				[(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50):(0x0100001fffffffff,25)(0x0400000003ffffff,25)(0x0500000000003fff,50)]

* 50% Atomic Load & Stores and 50% all other CPU instructions
rule_id						cpu41
num_oper					30000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				2000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK				[(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50):(0x0500000000003000,50)(0x0500000000000fff,50)]
