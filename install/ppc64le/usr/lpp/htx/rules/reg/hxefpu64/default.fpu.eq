* Rule file for hxefpu64 exerciser running on P7.
* For details of rule keywords, please refer README.hxefpu64
* in /usr/lpp/htx/rules/reg/hxefpu64 dir on test machine.
*
* NOTE: Rule keyword & associated value should be all in one
*	line _only_. Currently '\n' (newline) is used as
*	delimiter to differentiate between keywords.
*


* All VSX instructions with mixed data zones

rule_id						test1
num_oper					10000
num_threads					1
seed						[0x0, 0x0, 0x0, 0x0]
stream_depth					1000
test_method					1
fpscr						[0x0, 0x0, 0x0, 0x0]
DATA_BIAS_MASK    				[0x1111111111, 0x1111111111, 0x1111111111, 0x1111111111]
INS_BIAS_MASK					[(0x01000007ffffffff, 100):	(0x01000007ffffffff, 100): (0x01000007ffffffff, 100): (0x01000007ffffffff, 100)]
