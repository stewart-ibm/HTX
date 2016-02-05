* Rule file for hxecache
*
* Following is an explaination of supported keywords and default values.
*
* keyword - 	rule_id: 
* Description - Its stanza name and should be a char string of 20 or less bytes.
* Default - 	NULL string
*
* keyword - 	test: 
* Description - Type of test. Current supported values are 0 (i.e. L2 cache 
*		test) and 1 (i.e. L3 cache test).
* Default - 	0
*
* keyword - 	compare
* Description -	Whether to do data integrity checks or not. 0 - No, i.e. only
*		cache writes. 1 - Yes, i.e. read/compare the results against 
*		what was written. 0 can also be refered as stress mode.
* Default - 	1
*
* keyword - 	bound
* Description -	whether to bind to cpu.
* 		Please note this is only applicable when num_threads = 1, 
*		i.e. Uni Processor version of hxecache. 
* 		-1 - no binding. <any +ve no> - bind to that logical cpu no
*
* Keyword -	data_width
* Description -	Type of write/read ops to be used while stressing cache.
*		0 - byte, 1 - short, 2 - int, 3 - long int, 4 - random 
* Default -	3	
*
* Keyword -	target_set
* Description -	which particular set to stress. 16-way L3 cache will take
*		values in the range of 0 to 15. -1 indicates all sets.
* Default -	-1
*
*
* Keyword -	crash_on_misc
* Description -	Whether to crash to kernel debugger incase of miscompare.
*		0 - No, 1 - Yes. Following will be the contents of registers
*		while dropping to kernel debugger:
*		r3 - 0xBEEFDEAD
*		r4 - EA of miscompare location
*		r5 - pointer to 8 byte pattern
*		r6 - thread id (i.e. logical cpu no)
*		r7 - length of operation (i.e. data_width)
*		r8 - Thread no [ sequence of creation ] 
* Default -	1
*
* Keyword - num_oper 
* Description - Number of times L2 or L3 cache would roll over. 
*		Value assigned can be any +ve integer 
*		0 - to run infinitely but this would execute only one stanza and 
*		does not increment count if executed with supervisor interface.        
* Default - 10 
*
* Keyword - prefetch_irritator
* Description - Whether to run prefetch irritator along with hxecache .
*		0 - No , 1 - Yes .
* Default - 0 
* Keyword - prefetch_nstride
* Description - Whether to run prefetch stride n along with hxecache .
*		0 - No , 1 - Yes .
* Default - 1 
*
* Keyword - prefetch_transient 
* Description - Whether to run transient prefetch along with hxecache .
*		0 - No , 1 - Yes .
* Default - 1 
*
* Keyword - prefetch_partial
* Description - Whether to run prefetch partial along with hxecache .
*		0 - No , 1 - Yes .
* Default - 1 
*
* Keyword - seed
* Description - To run the test with a particular seed . Used for recreate in case of a failure .
*			  0 - Normal run . 
* Default - 1 
*
* Keyword - testcase_type
* Description - 0 - Bounce. This causes cache bouncing.
* 	        1 - Rollover.This causes cache to rollover. 
*		2 - Cache Test Off
* Default - 1
*
* Keyword - gang_size
* Description - 1 - When run for Equaliser
* 	        16 - All other cases
* Default - 16
* Following is a sample stanza
* rule_id	1
* test		1
* compare	1
* bound		1
* target_set 	-1
* data_width	0
* crash_on_misc	0
* num_oper	100
* prefetch_irritator	0
* prefetch_nstride	1
* prefetch_partial	1
* prefetch_transient 	1
* testcase_type         1
* gang_size 		8
*
* Important Note: 
* Its mandatory to have one blank line between two stanzas.
* It acts as stanza delimiter. If you have only one stanza 
* in the rule file than you should have newline(enter key)
* after the last line in stanza.
*

rule_id			L3Bounce_1
test	 		1	
compare			1
bound			0
target_set 		-1
data_width		3
crash_on_misc		1
num_oper		50
prefetch_irritator	1
prefetch_nstride	0
prefetch_partial	0
prefetch_transient	0
seed 			0
testcase_type           0
gang_size		8

rule_id			L2Bounce_2
test	 		0	
compare			1
bound			0
target_set 		-1
data_width		3
crash_on_misc		1
num_oper		50
prefetch_irritator	1
prefetch_nstride	0
prefetch_partial	0
prefetch_transient	0
seed 			0
testcase_type           0
gang_size		8




