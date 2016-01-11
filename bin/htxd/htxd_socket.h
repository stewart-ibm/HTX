/* @(#)68	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_socket.h, htxd, htxubuntu 7/17/13 09:12:06 */



#ifndef HTXD__SOCKET__HEADER
#define HTXD__SOCKET__HEADER


extern int htxd_create_socket(void);
extern int htxd_set_socket_option(int);
extern int htxd_bind_socket(int, struct sockaddr_in *, int);
extern int htxd_listen_socket(int);
extern int htxd_select(int);
extern int htxd_accept_connection(int, struct sockaddr_in *, socklen_t *);
extern int htxd_receive_command_length(int);
extern char * htxd_receive_command(int);
extern int htxd_send_ack_command_length(int);
extern int htxd_send_response_length(int, int);
extern int htxd_send_response(int, char *, int);
extern  int htxd_receive_bytes(int, char *, int);


#endif
