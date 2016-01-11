* Rule file for hxefpu64 exerciser running on P8.
* For details of rule keywords, please refer README.hxecpu  
* in /usr/lpp/htx/rules/reg/hxecpu   dir on test machine.
*
* NOTE: Rule keyword & associated value should be all in one
*	line _only_. Currently '\n' (newline) is used as
*	delimiter to differentiate between keywords.
*
* All types of CPU Instructions                                   
rule_id						test1
num_oper					60000 
num_threads					1
seed						[0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0]
stream_depth				1000
test_method					1
unaligned_loads_stores_%	[0,0,0,0,0,0,0,0]
INS_BIAS_MASK              	[(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100):(0x0500000000003fff,100)]
