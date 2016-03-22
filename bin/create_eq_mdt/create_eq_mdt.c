/* IBM_PROLOG_BEGIN_TAG */
/*
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *               http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

/* @(#)37	1.3  src/htx/usr/lpp/htx/bin/create_eq_mdt/create_eq_mdt.c, htxconf, htxubuntu 1/21/16 02:10:07 */

/*************************************************************/
/* create_eq_mdt binary takes equaliser CFG file as a input  */
/* and generates corresponding mdt whose name is also given  */
/* as an argument.                                           */
/*************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "htxsyscfg64.h"


#define MAX_PARAMS_IN_ROW	7
#define MAX_LINE_SIZE		256
struct dev_exer_map {
    char dev[16];
    char exer[64];
    char adapt_desc[16];
    char dev_desc[24];
};

struct dev_exer_map map_array[] = {{"mem", "hxemem64", "64bit", "memory"},
                               {"fpu", "hxefpu64", "core", "floating_point"},
                               {"cpu", "hxecpu", "processor", "processor"},
                               {"sctu", "hxesctu", "cache", "coherence_test"},
                               {"cache", "hxecache", "", "Processor_Cache"},
                               {"tdp", "hxeewm", "", "energy workload"},
                               {"rdp", "hxeewm", "", "energy workload"},
                               {"fdaxpy", "hxeewm", "", "energy workload"},
                               {"mdaxpy", "hxeewm", "", "energy workload"},
                               {"ddot", "hxeewm", "", "energy workload"},
                               {"pv", "hxeewm", "", "energy workload"}
                              };
int line_num = 0, wof_test = 0;
int create_dev_stanza (FILE *fp, char *affinity_str, char *dev_name, char *rule_file);

int main(int argc, char **argv)
{
    int rc, i, err_no;
    char eq_file_name[128], mdt_file_name[128];
    char line[MAX_LINE_SIZE], affinity_str[64];
    char rule_file_name[128], dev_name[64];
    char *chr_ptr[MAX_PARAMS_IN_ROW], keywd[32];

    FILE *eq_fp, *mdt_fp;

    if (argc != 3) {
        printf ("Usage error: below is the syntax to call create_eq_mdt\n");
        printf ("create_eq_mdt <cfg_file> <mdt file>\n");
        printf ("NOTE: You need to give absolute path for cfg and mdt file\n");
        exit(1);
    }

    strcpy(eq_file_name, argv[1]);
    strcpy(mdt_file_name, argv[2]);

    eq_fp = fopen(eq_file_name, "r");
    if (eq_fp == NULL) {
        err_no = errno;
        printf("Error opening the equaliser file %s. errno: %d\n", eq_file_name, err_no);
        exit(1);
    }

    mdt_fp = fopen(mdt_file_name, "w");
    if (mdt_fp == NULL) {
        err_no = errno;
        printf("Error opening the mdt file %s. errno: %d\n", mdt_file_name, err_no);
        exit(1);
    }

    rc = init_syscfg_with_malloc();
    if (rc == -1) {
        printf("Could not collect system config detail. Exiting...\n");
        exit(1);
    }
    create_default_stanza(mdt_fp, eq_file_name);
    while (1) {
        line_num++;
        rc = get_line(line, MAX_LINE_SIZE, eq_fp);  /* gets one line at a time from config file  */
        if (rc == 0 || rc == 1) {  /* Line is either blank or a comment */
            continue;
        }
        else if (rc == -1) {  /* Error in reading file  */
            fclose(eq_fp);
            return(rc);
        }
        else if (rc == -2) {  /* End of file  */
            break;
        }
        else {  /* got some good data  */
            /* We need to get only device entries. otherwise, skip rest of the
             * code and get next line.
             */
            sscanf(line,"%s", keywd);
            if ((strcmp(keywd, "time_quantum") == 0) ||
                (strcmp(keywd, "startup_time_delay") == 0) ||
                (strcmp(keywd, "log_duration") == 0) ||
                (strcmp(keywd, "pattern_length") == 0)) {
                continue;
           } else {
               i= 0;
               if ((chr_ptr[i++] = strtok(line," \n\t")) != NULL) {
                   sscanf(chr_ptr[0], "%s", affinity_str);
                   for (; i < MAX_PARAMS_IN_ROW; i++) {
                       if ((chr_ptr[i] = strtok(NULL, " \t\n")) == NULL) {
                           printf("Improper data provided in line num %d.\n%s Exiting\n", line_num, line);
                           exit(1);
                       }
                   }
                   sscanf(chr_ptr[1], "%s", dev_name);
                   sscanf(chr_ptr[6], "%s", rule_file_name);
                   create_dev_stanza(mdt_fp, affinity_str, dev_name, rule_file_name);
               }
           }
       }
    }

    return 0;
}

int create_dev_stanza (FILE *fp, char *affinity_str, char *dev_name, char *rule_file)
{
    int num_entries, lcpu=0, start_index;
    int i, j, k, l;
    char dev_name_str[16], rule_file_name[64], *chr_ptr[4];
    char *ptr, tmp[4], tmp_str[16];
    int node_num, chip_num, core_num, thread_num;
    int num_of_nodes, num_of_chips, num_of_cores, num_of_threads;
    struct dev_exer_map cur_dev_map;

    num_entries = sizeof(map_array) / sizeof(struct dev_exer_map);
    for (i = 0; i < num_entries; i++) {
        if (strcasecmp (map_array[i].dev, dev_name) == 0) {
            strcpy(cur_dev_map.exer, map_array[i].exer);
            strcpy(cur_dev_map.adapt_desc, map_array[i].adapt_desc);
            strcpy(cur_dev_map.dev_desc, map_array[i].dev_desc);
            break;
        }
    }
    strcpy(rule_file_name, cur_dev_map.exer);
    strcat(rule_file_name, "/");
    strcat(rule_file_name, rule_file);

    if (i == num_entries) {
        printf ("dev %s is not supported under equaliser.\n", dev_name);
        exit(1);
    }

    i = 0;
    if ((chr_ptr[i++] = strtok(affinity_str, "NPCT")) != NULL) {
        for (; i < 4; i++) {
            if ((chr_ptr[i] = strtok(NULL, "NPCT")) == NULL) {
                printf("Improper data is provided in line %d. i: %d\n", line_num, i);
                exit(1);
            }
        }
    }
    if (chr_ptr[0][0] == '*') {
        node_num = 0;
        num_of_nodes = get_num_of_nodes_in_sys();
    } else if (strchr(chr_ptr[0], '[') != NULL) { /* means a range of nodes is given */
        strcpy(tmp_str, chr_ptr[0]);
        ptr = strtok(tmp_str, "[]-");
        node_num = atoi(ptr);
        ptr = strtok(NULL, "[]-");
        strcpy(tmp, ptr);
        if (tmp[0] == 'n' || tmp[0] == 'N') {
            num_of_nodes = get_num_of_nodes_in_sys();
            num_of_nodes = num_of_nodes - node_num;
        } else {
            num_of_nodes = atoi(tmp) - node_num + 1;
        }
    } else {
        node_num = atoi(chr_ptr[0]);
        num_of_nodes = 1;
    }
    for (i = 0; i < num_of_nodes; i++, node_num++) {
        if (chr_ptr[1][0] == '*') {
            chip_num = 0;
            num_of_chips = get_num_of_chips_in_node(node_num);
        } else if (strchr(chr_ptr[1], '[') != NULL) { /* means a range of chips is given */
            strcpy(tmp_str, chr_ptr[1]);
            ptr = strtok(tmp_str, "[]-");
            chip_num = atoi(ptr);
            ptr = strtok(NULL, "[]-");
            strcpy(tmp, ptr);
            if (tmp[0] == 'n' || tmp[0] == 'N') {
                num_of_chips = get_num_of_chips_in_node(node_num);
                num_of_chips -= chip_num;
            } else {
                num_of_chips = atoi(tmp) - chip_num + 1;
            }
        } else {
           chip_num = atoi(chr_ptr[1]);
           num_of_chips = 1;
        }
        for (j = 0; j < num_of_chips; j++, chip_num++) {
            if (chr_ptr[2][0] == '*') {
                core_num = 0;
                num_of_cores = get_num_of_cores_in_chip(node_num, chip_num);
            } else if (strchr(chr_ptr[2], '[') != NULL) { /* means a range of cores is defined */
                strcpy(tmp_str, chr_ptr[2]);
                ptr = strtok(tmp_str, "[]-");
                core_num = atoi(ptr);
                ptr = strtok(NULL, "[]-");
                strcpy(tmp, ptr);
                if (tmp[0] == 'n' || tmp[0] == 'N') {
                    num_of_cores = get_num_of_cores_in_chip(node_num, chip_num);
                    num_of_cores -= core_num;
                } else {
                    num_of_cores = atoi(tmp) - core_num + 1;
                }
            } else {
                core_num = atoi(chr_ptr[2]);
                num_of_cores = 1;
            }
            for (k = 0; k < num_of_cores; k++, core_num++) {
                /* Exclude N0P0C0T* if wof test is enabled */
                if (wof_test == 1 && core_num == 0 && chip_num == 0 && node_num == 0) {
                    continue;
                }
                if (chr_ptr[3][0] == '*') {
                    thread_num = 0;
                    num_of_threads = get_num_of_cpus_in_core(node_num, chip_num, core_num);
                } else if (strchr(chr_ptr[3], '[') != NULL) {
                    strcpy(tmp_str, chr_ptr[3]);
                    ptr = strtok(tmp_str, "[]-");
                    thread_num = atoi(ptr);
                    ptr = strtok(NULL, "[]-");
                    strcpy(tmp, ptr);
                    if (tmp[0] == 'n' || tmp[0] == 'N') {
                        num_of_threads = get_num_of_cpus_in_core(node_num, chip_num, core_num);
                        num_of_threads -= thread_num;
                    } else {
                        num_of_threads = atoi(tmp) - thread_num + 1;
                    }
                } else {
                    thread_num =  atoi(chr_ptr[3]);
                    num_of_threads = 1;
                }
                for (l = 0; l < num_of_threads; l++, thread_num++) {
                    lcpu = get_logical_cpu_num(node_num, chip_num, core_num, thread_num);
                    sprintf(dev_name_str, "%s%d", dev_name, lcpu);
                    fprintf(fp, "%s:\n", dev_name_str);

                    create_string_entry(fp, "HE_name", cur_dev_map.exer, "* Hardware Exerciser name, 14 char");
                    create_string_entry(fp, "adapt_desc", cur_dev_map.adapt_desc, "* adapter description, 11 char max");
                    create_string_entry(fp, "device_desc", cur_dev_map.dev_desc, "* device description, 15 char max.");
                    create_string_entry(fp, "reg_rules", rule_file_name, "* reg");
                    create_string_entry(fp, "emc_rules", rule_file_name, "* emc");
                    create_string_entry(fp, "cont_on_err", "NO", "* continue on error (YES/NO)");
                    fprintf(fp, "\n");
                }
            }
        }
    }

    return 0;
}

void create_string_entry(FILE *fp, char *param_name, char *param_value, char *desc)
{
    int start_index;
    char comment[64];
    char tmp[2] = "";

    start_index = 40 - strlen(param_name) - strlen(param_value);
    sprintf(comment, "%*s", start_index, tmp);
    fprintf(fp, "\t%s = \"%s\" %s%s\n", param_name, param_value, comment, desc);
}

void create_numeric_entry(FILE *fp, char *param_name, char *param_value, char *desc)
{
    int start_index;
    char comment[64], tmp[2] = "";

    start_index = 42 - strlen(param_name) - strlen(param_value);
    sprintf(comment, "%*s", start_index, tmp);
    fprintf(fp, "\t%s = %s %s%s\n", param_name, param_value, comment, desc);
}

void create_default_stanza(FILE *fp, char *eq_file)
{
    char buf[24], str[128], *tmp_str;
    FILE *tmp_fp;

    sprintf(str, "cat %s | grep wof_test | grep -v \"#\" | awk '{ print $NF}'", eq_file);
    tmp_fp = popen (str, "r");
    if (tmp_fp == NULL) {
        printf("Error for popen in create_eq_mdt. Exiting...");
        exit(1);
    }
    if (fgets(buf,24,fp) != NULL) {
        sscanf(buf, "%d", &wof_test);
    }
    pclose(tmp_fp);
    fprintf(fp, "default:\n");
    create_string_entry(fp, "HE_name", "", "* Hardware Exerciser name, 14 char");
    create_string_entry(fp, "adapt_desc", "", "* adapter description, 11 char max");
    create_string_entry(fp, "device_desc", "", "* Hardware Exerciser name, 14 char");
    create_string_entry(fp, "reg_rules", "", "* reg rules");
    create_string_entry(fp, "emc_rules", "", "* emc rules");
    create_numeric_entry(fp, "dma_chan", "0", "* DMA channel number");
    create_numeric_entry(fp, "idle_time", "0", "* idle time (secs)");
    create_numeric_entry(fp, "intrpt_lev", "0", "* interrupt level");
    create_numeric_entry(fp, "load_seq", "32768", "* load sequence (1 - 65535)");
    create_numeric_entry(fp, "max_run_tm", "7200", "* max run time (secs)");
    create_string_entry(fp, "port", "0", "* port number");
    create_numeric_entry(fp, "priority", "17", "* priority (1=highest to 19=lowest");
    create_string_entry(fp, "slot", "0", "* slot number");
    create_string_entry(fp, "max_cycles", "0", "* max cycles");
    create_numeric_entry(fp, "hft", "0", "* hft number");
    create_string_entry(fp, "cont_on_err", "NO", "* continue on error (YES/NO)");
    create_string_entry(fp, "halt_level", "1", "* level <= which HE halts");
    create_string_entry(fp, "start_halted", "n", "* exerciser halted at startup");
    create_string_entry(fp, "dup_device", "n", "* duplicate the device");
    create_string_entry(fp, "log_vpd", "y", "* Show detailed error log");
    create_numeric_entry(fp, "equaliser_flag", "1", "* Equaliser flag enabled for supervisor");
    create_numeric_entry(fp, "equaliser_debug_flag", "0", "* Equaliser debug flag for supervisor");
    if ( wof_test == 1) {
        create_numeric_entry(fp, "wof_test", "1", "* Wof test enabled for supervisor");
    }
    strcpy(str, eq_file);
    tmp_str = basename(str);
    create_string_entry(fp, "cfg_file", tmp_str, "* Corresponding cfg file for this mdt");
    fprintf(fp, "\n");
}

int parse_line(char s[])
{
        int i = 0;

        while(s[i] == ' ' || s[i] == '\t') {
                i++;
        }
        if(s[i] == '#') {
                return(0);
        }
        return((s[i] == '\n') ? 1: 2);
}

int get_line(char line[], int lim, FILE *fp)
{
        int c = 0, rc, err_no;
        char *p, msg[256];

        p = fgets(line, lim, fp);
        err_no = errno;
        if (p == NULL && feof(fp)) {
                return (-2);
        }
        else if (p == NULL && ferror(fp)) {
                printf(msg, "Error (errno = %d) in reading config file. exiting...\n", err_no);
                exit(1);
        }
        else {
                rc = parse_line(line);
                return rc;
        }
}

