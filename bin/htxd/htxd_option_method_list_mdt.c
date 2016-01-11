/* @(#)55	1.3  src/htx/usr/lpp/htx/bin/htxd/htxd_option_method_list_mdt.c, htxd, htxubuntu 7/8/15 00:08:59 */



#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include "htxd.h"

#define LIST_ITEM_LENGTH 256
#define	BUFFER_INCREMENT_LEVEL 1024

int htxd_list_files(char *path_name, char **file_list)
{
	DIR *p_dir;
	struct dirent * p_dir_entry;
	int i = 0;
	char list_item[LIST_ITEM_LENGTH];
	int list_length = 0;
	int list_buffer_length = BUFFER_INCREMENT_LEVEL;

	*file_list = malloc(list_buffer_length);
	if(*file_list == NULL) {
		return -1;
	}

    p_dir = opendir(path_name);
    if(p_dir == NULL) {
        perror("dir open failed");
        return -1;
    }

    while ( (p_dir_entry = readdir(p_dir) ) != NULL) {
        if( (strcmp (p_dir_entry->d_name, ".") == 0) ||	/* skip . entry				*/
            (strcmp (p_dir_entry->d_name, "..") == 0 )) {	/* skip .. entry			*/
            continue;
        }
        sprintf(list_item, "%2d) %-30s", ++i, p_dir_entry->d_name);
		list_length += strlen(list_item);
		if(list_buffer_length > (list_length + 100) ) {
			strcat(*file_list, list_item);
		} else {
			list_buffer_length += BUFFER_INCREMENT_LEVEL;
			*file_list = realloc(*file_list, list_buffer_length);
			if(*file_list == NULL) {
				return -1;
			}
			strcat(*file_list, list_item);
		}
        if( i % 2 == 0) {
            strcat(*file_list, "\n");
        }
    }

    if(closedir(p_dir)) {
        perror("dir close failed");
       	return -1; 
    }

    return 0;
}


int htxd_option_method_list_mdt(char **result)
{
	int return_code = 0;

	return_code = htxd_list_files("/usr/lpp/htx/mdt", result);

	return return_code;
}

