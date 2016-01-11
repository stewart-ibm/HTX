

/* @(#)41	1.2  src/htx/usr/lpp/htx/bin/eservd/config.h, eserv_daemon, htxubuntu 1/4/16 23:56:17 */

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

struct thread_config_parameters {
    char        dev_name[16];
    int         lcpu;
    int         pcpu;
    uint32      util_pattern;
    uint32      utilization_pattern;
    uint16      pattern_length;
    uint16      sequence_length;
    uint16      utilization_sequence[MAX_UTIL_SEQUENCE_LENGTH];
};
typedef struct thread_config_parameters thread_config_params;

struct run_time_thread_config_structure {
    thread_config_params    th_config;
    run_time_data           data;
};
typedef struct run_time_thread_config_structure run_time_thread_config;

struct test_config_structure {
    uint32                      time_quantum;
    uint16                      num_tests_configured;
    uint32                      wof_test;
    uint32                      startup_time_delay;
    uint32                      log_duration;
    run_time_thread_config      *thread;
};
typedef struct test_config_structure test_config_struct;


