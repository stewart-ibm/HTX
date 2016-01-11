/* @(#)20	1.1  src/htx/usr/lpp/htx/bin/htxd/htxcmd_display_result.c, htxd, htxubuntu 7/17/13 02:03:59 */



#include <stdio.h>


void htxcmd_display_result(char *result_string)
{
	printf("\n######################## Result Starts Here ################################");
	printf("\n%s\n", result_string);
	printf("######################### Result Ends Here #################################\n");
}
