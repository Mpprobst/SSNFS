/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
 // Based on file provided on canvas

#include <stdio.h>
#include <rpc/rpc.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "ssnfs.h"

CLIENT *clnt;

void ssnfsprog_1(char *host)
{
clnt = clnt_create (host, SSNFSPROG, SSNFSVER, "tcp");
	if (clnt == NULL) {
		clnt_pcreateerror (host);
		exit (1);
	}
}

/*
Open: opens a file with the given name in the user's directory. If file does
not exist, it is created. If file cannot be opened or created, return -1 as an
indicator of error.
returns: file descriptor of opened file.
*/
int Open(char *filename_to_open){
	printf("\nIn client: opening %s\n", filename_to_open);
  open_output  *result_1;
  open_input  open_file_1_arg;
  strcpy(open_file_1_arg.user_name, getpwuid(getuid())->pw_name);
  strcpy(open_file_1_arg.file_name,filename_to_open);
  result_1 = open_file_1(&open_file_1_arg, clnt);
	if (result_1 == (open_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	//printf ("In client: Directory name is:%s \nIn client: Name of the file opened is:%s \nIn client: file descriptor returned is:%d\n", open_file_1_arg.user_name, result_1->out_msg.out_msg_val,  result_1->fd);
	printf("In client: %s\n", result_1->out_msg.out_msg_val);
	if (result_1->fd > -1) {
		printf("In client: file descriptor returned is:%d\n", result_1->fd);
	}
	return result_1->fd;
}

void Write(int fd, char * buffer, int num_bytes_to_write){
	printf("\nIn client: writing \"%s\" (%dB) to fd:%d\n", buffer, num_bytes_to_write, fd);
	write_output *result_3;
	write_input write_file_1_arg;
	char message[num_bytes_to_write];
	memcpy(message, buffer, num_bytes_to_write);
	message[num_bytes_to_write-1] = '\0';
	//printf("full message(%dB): %s", sizeof(message), message);
	write_file_1_arg.fd = fd;
	write_file_1_arg.buffer.buffer_val = malloc(num_bytes_to_write);
	strcpy(write_file_1_arg.user_name, getpwuid(getuid())->pw_name);
	strcpy(write_file_1_arg.buffer.buffer_val, message);
	write_file_1_arg.buffer.buffer_len = num_bytes_to_write;
	write_file_1_arg.numbytes = num_bytes_to_write;
	result_3 = write_file_1(&write_file_1_arg, clnt);
		if (result_3 == (write_output *) NULL) {
			clnt_perror (clnt, "call failed");
		}
	printf("receive reply \n");
	buffer = malloc(result_3->out_msg.out_msg_len);
	strcpy(buffer, result_3->out_msg.out_msg_val);
	printf("%s\n", result_3->out_msg.out_msg_val);
}

void Read(int fd, char * buffer, int num_bytes_to_read){
	printf("\nIn client: reading %dB from fd: \n", num_bytes_to_read, fd);
  read_output * result_2;
	read_input read_file_1_arg;
	// ask server to read the file I own with a
	read_file_1_arg.fd = fd;
	strcpy(read_file_1_arg.user_name, getpwuid(getuid())->pw_name);
	read_file_1_arg.numbytes = num_bytes_to_read;

	result_2 = read_file_1(&read_file_1_arg, clnt);
	if (result_2 == (read_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	memcpy(buffer, result_2->out_msg.out_msg_val, result_2->out_msg.out_msg_len);
}

void Close(int fd){
	printf("\nIn client: closing fd: %dB", fd);
	close_output  *result_6;
	close_input  close_file_1_arg;
        result_6 = close_file_1(&close_file_1_arg, clnt);
	if (result_6 == (close_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
}

void List(){
	printf("\nIn client: requesting file list.");
	list_output  *result_4;
	list_input  list_files_1_arg;
	strcpy(list_files_1_arg.user_name, getpwuid(getuid())->pw_name);
	result_4 = list_files_1(&list_files_1_arg, clnt);
	if (result_4 == (list_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	printf("files owned by: %s\n%s", list_files_1_arg.user_name, result_4->out_msg.out_msg_val);
}

void Delete(int fd) {
	printf("\nIn client: delete fd: %dB", fd);
	delete_output  *result_5;
	delete_input  delete_file_1_arg;
	result_5 = delete_file_1(&delete_file_1_arg, clnt);
	if (result_5 == (delete_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
}

int main (int argc, char *argv[])
{
	char *host;

	if (argc < 2) {
		printf ("usage: %s server_host\n", argv[0]);
		exit (1);
	}
	host = argv[1];
	ssnfsprog_1 (host);

	int fd1 = Open("myfile");

	Write(fd1, "hi this is my file. it only prints half.\n", 20);

	int fd2 = Open("secret");
	int fd3 = Open("thirdfile");
	int fd4 = Open("michael");

	//List();

	int bytes_to_read = 20;
	char buffer[bytes_to_read];
	Read(fd1, buffer, bytes_to_read);
	printf("Reading fd %d:\n%s", fd1, buffer);

	//List();

exit (0);
}
