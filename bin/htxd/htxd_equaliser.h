/* @(#)45	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_equaliser.h, htxd, htxubuntu 7/17/13 08:48:07 */



#ifndef HTXD__EQUALISER__HEADER
#define HTXD__EQUALISER__HEADER


#define UTILIZATION_QUEUE_LENGTH     20
#define MAX_UTIL_SEQUENCE_LENGTH     10
#define FILENAME	"/usr/lpp/htx/htx_eq.cfg"
#define LOGFILE       "/tmp/eq_status"
#define LOGFILE_SAVE  "/tmp/eq_status_save"

#define UTIL_LEFT	0
#define UTIL_RIGHT	1
#define UTIL_RANDOM	2
#define UTIL_PATTERN	3

typedef char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

struct run_time_data {
    uint32 target_utilization;
    uint32 current_seq_step;
    uint8 current_step;
};
typedef struct run_time_data run_time_data;

struct thread_config_structure {
	char        dev_name[16];
	uint32      util_pattern;
	uint32      utilization_pattern;
	uint16      pattern_length;
	uint16      sequence_length;
	uint16      utilization_sequence[MAX_UTIL_SEQUENCE_LENGTH];
	run_time_data   data;
};
typedef struct thread_config_structure thread_config_struct;
struct test_config_structure {
    uint32              time_quantum;
    uint16              num_tests_configured;
    uint32              startup_time_delay;
    uint32              log_duration;
    thread_config_struct    *thread;
};
typedef struct test_config_structure test_config_struct;


extern void htxd_equaliser(void);

#endif