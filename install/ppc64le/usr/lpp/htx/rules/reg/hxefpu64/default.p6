* Rule file for hxefpu64 exerciser running on P6
* For details of rule keywords, please refer README.hxefpu64
* in /usr/lpp/htx/rules/reg/hxefpu64 dir on test machine.
*
* NOTE: Rule keyword & associated value should be all in one
*	line _only_. Currently '\n' (newline) is used as
*	delimiter to differentiate between keywords.
*
*



* All BFP, DFP & VMX instructions with normal data

rule_id						test1
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa, 0xa, 0xa, 0xa]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with 50% normal & 50% sub-normal data

rule_id						test2
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with 100% denormal data

rule_id						test3
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa0, 0xa0, 0xa0, 0xa0]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]




* All BFP, DFP & VMX instructions with 30% normal, 30% sub-normal, 20% infinity n 20% SNaN data

rule_id						test4
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x330022, 0x330022, 0x330022, 0x330022]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with data biasing towards 10% each of the data zones 

rule_id						test5
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* BFP DP ALL instructions with 50% normal & 50% denormal data zones

rule_id						test6
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x020000000003feb6, 100):	(0x020000000003feb6, 100): (0x020000000003feb6, 100): (0x020000000003feb6, 100)]



* BFP SP ALL instructions with 50% normal & 50% denormal data zones

rule_id						test7
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x0200000000000b49, 100):	(0x0200000000000b49, 100): (0x0200000000000b49, 100): (0x0200000000000b49, 100)]



* DFP_LONG ALL instructions with 50% normal & 50% sub-normal data zones

rule_id						test8
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x030000001bd2faaa, 100):	(0x030000001bd2faaa, 100): (0x030000001bd2faaa, 100): (0x030000001bd2faaa, 100)]



* DFP_QUAD ALL instructions with 50 % normal & 50 % sub-normal data

rule_id						test9
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x03000000052da555, 100):	(0x03000000052da555, 100): (0x03000000052da555, 100): (0x03000000052da555, 100)]



* VMX integer (XS & XC) instructions only with 100% norm number.
* All of the above instructions will be scheduled on pipe-0 only.

rule_id						test10
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa, 0xa, 0xa, 0xa]
INS_BIAS_MASK				[(0x040000000001fff8, 100):	(0x040000000001fff8, 100): (0x040000000001fff8, 100): (0x040000000001fff8, 100)]



* VMX permute (PM) and DFP instruction with 100% norm number only.
* All of the above instructions will be scheduled on pipe-1 only.

rule_id						test11
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa, 0xa, 0xa, 0xa]
INS_BIAS_MASK				[(0x0400000003000000,50)(0x030000001fffffff,50):(0x0400000003000000,50)(0x030000001fffffff,50):(0x0400000003000000,50)(0x030000001fffffff,50):(0x0400000003000000,50)(0x030000001fffffff,50)]



* VMX instructions interleaved with VSCR instructions.
* T0 & T3 - VMX FP instructions only with VSCR. (pipe0)
* T1 & T2 - VMX Permute (PM) only with VSCR. (pipe1)
* It will keep flushing the pipelines periodically. 

rule_id						test12
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa, 0xa, 0xa, 0xa]
INS_BIAS_MASK				[(0x04000000007e0000,70)(0x0400000000800000,30):(0x0400000001000000,70)(0x0400000000800000,30):(0x0400000001000000,70)(0x0400000000800000,30):(0x04000000007e0000,70)(0x0400000000800000,30)]



* All BFP & DFP Arithmatic only instructions with 10% all data types

rule_id						test13
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK				[(0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50)]



* All BFP, DFP & VMX instructions with normal data (correctness)

rule_id						test14
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa, 0xa, 0xa, 0xa]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with 50% normal & 50% sub-normal data (correctness)

rule_id						test15
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x55, 0x55, 0x55, 0x55]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with 100% denormal data (correctness)

rule_id						test16
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0xa0, 0xa0, 0xa0, 0xa0]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]




* All BFP, DFP & VMX instructions with 30% normal, 30% sub-normal, 20% infinity n 20% SNaN data (correctness)

rule_id						test17
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x330022, 0x330022, 0x330022, 0x330022]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]



* All BFP, DFP & VMX instructions with data biasing towards 10% each of the data zones (correctness)

rule_id						test18
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK				[(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30):(0x020000000007ffff,40)(0x030000001fffffff,30)(0x0400000003ffffff,30)]




* All BFP & DFP Arithmatic only instructions with 10% all data types (correctness)

rule_id						test19
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK				[(0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50): (0x02000000000003c0, 50)(0x0300000000000003, 50)]


* VMX & DFP all instructions with 10% all data types (correctness)

rule_id						test20
num_oper					5000
num_threads					0
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					2
unaligned_loads_stores_%	[0,0,0,0]
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    			[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK				[(0x0400000003ffffff, 50)(0x030000001fffffff, 50):(0x0400000003ffffff, 50)(0x030000001fffffff, 50):(0x0400000003ffffff, 50)(0x030000001fffffff, 50):(0x0400000003ffffff, 50)(0x030000001fffffff, 50)]


 
