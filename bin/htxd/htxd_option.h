/* @(#)52	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_option.h, htxd, htxubuntu 7/17/13 08:57:48 */



#ifndef HTXD__OPTION__HEADER
#define HTXD__OPTION__HEADER


typedef struct
{
	char	option_string[128];
	int		(*option_method)(char **);
	int		parameter_flag;
	int		running_ecg_display_flag;
}option;




#endif

