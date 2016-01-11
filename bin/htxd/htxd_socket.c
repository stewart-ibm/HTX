/* @(#)67	1.2  src/htx/usr/lpp/htx/bin/htxd/htxd_socket.c, htxd, htxubuntu 2/5/15 00:50:58 */



#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define BACKLOG 10
/* #define	SELECT_TIMEOUT_SECONDS			600 */
#define	SELECT_TIMEOUT_SECONDS			100
#define	SELECT_TIMEOUT_MICRO_SECONDS	0

#define COMMAND_HEADER_LENGTH 16
#define COMMAND_TAILER_LENGTH 16
#define COMMAND_STRING_LENGTH 10

int htxd_create_socket(void)
{
	int socket_fd;

	socket_fd = socket (AF_INET, SOCK_STREAM, 0);
	if(socket_fd == -1)
	{
		perror("ERROR: while creating socket. Exiting...");
		exit(1);
	}

	return socket_fd;
}

int htxd_set_socket_option(int socket_fd)
{
	int result;
	int option_value = 1;

	result = setsockopt (socket_fd, SOL_SOCKET, SO_REUSEADDR, (void *) &option_value, sizeof (option_value) );
	if(result == -1)
	{
		perror("ERROR: while setting socket options. Exiting...");
		exit(1);
	}

	return result;
}




int htxd_bind_socket(int socket_fd, struct sockaddr_in *local_address, int port_number)
{
	int result;


	result = bind (socket_fd, (struct sockaddr *) local_address, sizeof (struct sockaddr));
	if(result == -1)
	{
		perror("ERROR: while binding connection. Exiting...");
		exit(1);
	}

	return result;
}




int htxd_listen_socket(int socket_fd)
{
	int result;

	result = listen (socket_fd, BACKLOG);
	if(result == -1)
	{
		perror("ERROR: while listening connection. Exiting...");
		exit(1);
	}

	return result;
}


int htxd_select(int socket_fd)
{
	int				result;
	fd_set			read_fd_set;
	struct timeval	timeout;

	FD_ZERO (&read_fd_set);

	timeout.tv_sec	= SELECT_TIMEOUT_SECONDS;
	timeout.tv_usec	= SELECT_TIMEOUT_MICRO_SECONDS;

	FD_SET (socket_fd, &read_fd_set);

	result = select (socket_fd + 1, &read_fd_set, NULL, NULL, &timeout);
	if( (result == -1) && (errno != EINTR) ) {
		perror("ERROR: while selecting connection. Exiting...");
		exit(1);
	}

	return result;
}

int htxd_accept_connection(int socket_fd, struct sockaddr_in *p_client_address, socklen_t *p_address_length)
{
	int new_fd;

	memset((void*)p_client_address, 0, sizeof(struct sockaddr));
	*p_address_length = sizeof (struct sockaddr);
	new_fd = accept (socket_fd, (struct sockaddr *)p_client_address, p_address_length);
	if( (new_fd == -1) && (errno != EINTR) ) {
		perror("ERROR: while accepting connection. Exiting...");
		exit(1);
	}

	return new_fd; 
}



int htxd_send_ack_command_length(int new_fd)
{
	int result;
	char ack_string[] = ":length received:";
	
	result = send (new_fd, ack_string, strlen (ack_string), 0);
	if(result == -1)
	{
		return result;
	}

	return result;
}

int htxd_receive_bytes(int new_fd, char * receive_buffer, int receive_length)
{
	int received_bytes;
	int remaining_bytes;

	remaining_bytes = receive_length;
	while(remaining_bytes > 0) {
		received_bytes = recv(new_fd, receive_buffer, remaining_bytes, 0);
		if(received_bytes == -1) {
			return -1;
		}

		if(received_bytes == 0) {
			break;
		}

		remaining_bytes -= received_bytes;
		receive_buffer += received_bytes;
	}	
	return 	(receive_length - remaining_bytes);
}



/* incomming command string receives here */
char * htxd_receive_command(int new_fd)  /* note: have to change for error handling */
{
	int result;
	char * command_details_buffer = NULL;
	char temp_buffer[20];
	int command_length;


	memset(temp_buffer, 0, sizeof(temp_buffer) );
	/* receiving command  length from incomming commend */
	result = htxd_receive_bytes(new_fd, temp_buffer, 10);
	if(result == -1)
	{
		return NULL;
	}

	temp_buffer[COMMAND_STRING_LENGTH] = '\0';

	command_length = atoi(temp_buffer);

	/* now we are ready to receive the command string */
	command_details_buffer = malloc(command_length + 10);
	if(command_details_buffer == NULL)
	{
		printf("[DEBUG] : malloc() failed while allocating command_details_buffer"); fflush(stdout);
		return NULL;
	} 

	memset(command_details_buffer, 0, command_length + 10);

	/* receiving the command string */
	result = htxd_receive_bytes(new_fd, command_details_buffer, command_length);
	if(result == -1)
	{
		return NULL;
	}

	command_details_buffer[command_length] = '\0';

	return command_details_buffer;
}



int htxd_send_response(int new_fd, char *command_result, int command_return_code)
{
	int result = 0;
	char *response_buffer = NULL;
	int buffer_length;
	int number_of_bytes_sent;
	int cumulative_number_of_bytes_sent = 0;
	int number_of_bytes_to_send;


	buffer_length = strlen(command_result) + 10 + 10 + 10;
	response_buffer = malloc(buffer_length);
	memset(response_buffer, 0, buffer_length);
	sprintf(response_buffer, "%010d%010d%s", command_return_code,strlen(command_result), command_result);
	number_of_bytes_to_send = strlen(response_buffer);

	while(cumulative_number_of_bytes_sent < number_of_bytes_to_send) {
		number_of_bytes_sent = send(new_fd, response_buffer + cumulative_number_of_bytes_sent, number_of_bytes_to_send - cumulative_number_of_bytes_sent, 0);
		if(number_of_bytes_sent == -1)
		{
			printf("[DEBUG] : Error : htxd_send_response() send() returns -1, errno <%d\n", errno);
		}
		cumulative_number_of_bytes_sent += number_of_bytes_sent;
	}

	free(response_buffer);	

	return result;
}
