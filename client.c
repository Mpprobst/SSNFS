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
	printf("%s\n", result_1->out_msg.out_msg_val);
	if (result_1->fd > -1) {
		printf("file descriptor returned is:%d\n", result_1->fd);
	}

	return result_1->fd;
}

void Write(int fd, char * buffer, int num_bytes_to_write){
	printf("length of buffer: %d\n", strlen(buffer));
	if (num_bytes_to_write >= strlen(buffer)) {
		num_bytes_to_write = strlen(buffer);
	}
	buffer[num_bytes_to_write-1] = '\0';
	printf("\nIn client: writing (%dB) to fd:%d\n", num_bytes_to_write, fd);
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
	//memcpy(buffer, result_2->out_msg.out_msg_val, result_2->out_msg.out_msg_len);
	printf("File contents:\n%s\n+---------------+\n", result_2->out_msg.out_msg_val);
}

void Close(int fd){
	printf("\nIn client: closing fd: %d\n", fd);
	close_output  *result_6;
	close_input  close_file_1_arg;
	close_file_1_arg.fd = fd;
	strcpy(close_file_1_arg.user_name, getpwuid(getuid())->pw_name);
  result_6 = close_file_1(&close_file_1_arg, clnt);
	if (result_6 == (close_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	printf("%s", result_6->out_msg.out_msg_val);
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

void Delete(char * filename) {
	printf("\nIn client: delete file %s", filename);
	delete_output  *result_5;
	delete_input  delete_file_1_arg;
	strcpy(delete_file_1_arg.file_name, filename);
	strcpy(delete_file_1_arg.user_name, getpwuid(getuid())->pw_name);
	result_5 = delete_file_1(&delete_file_1_arg, clnt);
	if (result_5 == (delete_output *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	printf("%s", result_5->out_msg.out_msg_val);
}

void OpenTest() {
	printf("+---TEST 0---+\ntest a long filename\n");
	Open("this_is_a_long_file_name");
	printf("+----------+\n");

	// open a lot of files
	printf("+---TEST 1---+\nopen more files than file table can hold\n");
	char fname[6] = "file01";
	for (int i = 0; i < 25; i++) {
		sprintf(fname, "file%02d", i);
		Open(fname);
	}
	for (int i = 0; i < 20; i++) {
		sprintf(fname, "file%02d", i);
		Close(i);
	}
	printf("+----------+\n");

	// open same file many times
	printf("+---TEST 2---+\nopen same file many times\n");
	for (int i = 0; i < 5; i++) {
		Open("test");
	}
}

void WriteTest() {
	char buffer[512];
	for (int i = 0; i < 512; i++) {
		buffer[i] = 'a'+i%26;
	}

	printf("+---TEST 0---+\nwrite to unopened file\n");
	Write(2, "This file is not open\n", 10);
	printf("+----------+\n");

	printf("+---TEST 1---+\nwrite 0 bytes\n");
	int fd1 = Open("emptyfile");
	Write(fd1, "this will not be written", 0);
	Close(fd1);

	fd1 = Open("emptyfile");
	char result1[0];
	memset(result1, ' ', 0);
	Read(fd1, result1, 0);
	//Close(fd1);
	printf("+----------+\n");

	printf("+---TEST 2---+\nwrite more bytes than buffer is\n");
	int fd2 = Open("overwrite");
	Write(fd2, "a", 10);
	//Close(fd2);
	printf("+----------+\n");

	printf("+---TEST 3---+\nwrite over existing file\n");
	char fname3[7] = "myfile\0";
	int fd3 = Open(fname3);
	Write(fd3, "This will be overwritten\n", 25);

	Close(fd3);
	fd3 = Open(fname3);
	char result3[100];
	Read(fd3, result3, 25);

	Close(fd3);
	fd3 = Open(fname3);
	Write(fd3, "THE OLD FILE CONTENTS WERE OVERRIDDEN SUCCESSFULLY\n", 41);
	Close(fd3);

	fd3 = Open(fname3);
	Read(fd3, result3, 41);
	printf("+----------+\n");

	printf("+---TEST 4---+\nwrite between blocks\n");
	int fd4 = Open("test4");
	char buffer4[768];
	memset(buffer4, 'a', 512);
	memset(buffer4[511], 'b', 256);
	Write(fd4, buffer4, 768);
	Close(fd4);
	fd4 = Open("test4");
	char result4[384];
	Read(fd4, result4, 384);
	Read(fd4, result4, 384);
	printf("+----------+\n");

	printf("+---TEST 5---+\nwrite file with max size\n");
	int fd5 = Open("test5");
	// write a full file (plus extra block)
	// allows only 64 blocks. do 1 more to see what happens after
	for (int i = 0; i < 65; i++) {
		printf("write %d", i);	// should only allow 64 writes for this file
		Write(fd5, buffer, 512);
	}
	/*
	printf("+---TEST 6---+\nfill up memory completely\n");
	// fill up memory completely
	int fd6 = -1;
	char fname6[8] = "test000\0";
	// server supports max 512 files. 1 full file exists with annother with 1 block
	// should get notification of full memory on file 511 (named 510)
	for (int i = 0; i < 512; i++) {
		sprintf(fname6, "test%03d\0", i);
		printf("new file: %s", fname6);
		fd6 = Open(fname6);
		for (int j = 0; j < 64; j++) {
			Write(fd6, buffer, 512);
		}
		Close(fd6);
	}
	printf("+----------+\n");*/ // temporarily disabled to save time
}

void ReadTest() {
	char buffer[512];
	for (int i = 0; i < 512; i++) {
		buffer[i] = 'a'+i%26;
	}

	printf("+---TEST 0---+\nread invalid and unopened file");
	char result0[2];

	Read(50, result0, 2);
	Read(0, result0, 2);
	printf("+----------+\n");

	printf("+---TEST 1---+\nread exactly 1 byte and successive reads\n");
	int fd1 = Open("file1\0");
	Write(fd1, "123", 1);
	Close(fd1);
	fd1 = Open("file1\0");
	char result1[1];
	Read(fd1, result1, 1);
	Read(fd1, result1, 1);
	Read(fd1, result1, 1);
	Close(fd1);
	printf("+----------+\n");

	printf("+---TEST 2---+\nread more than is in a file\n");
	int fd2 = Open("eoftest");
	Write(fd2, "abc", 3);
	char result2[10];
	Read(fd2, result2, 10);
	Close(fd2);
	printf("+----------+\n");

	printf("+---TEST 3---+\nread over two blocks\n");
	int fd3 = Open("longread");
	Write(fd3, buffer, 512);
	Write(fd3, buffer, 512);
	Close(fd3);
	fd3 = Open(fd3);
	char result3[1024];
	Read(fd3, result3, 1024);
	printf("+----------+\n");

	printf("+---TEST 4---+\nread same file that has many file descriptors\n");
	int fd4[5];
	for (int i = 0; i < 5; i++) {
		fd4[i] = Open("test4");
	}
	Write(fd4[0], "This should be the same.", 24);
	for (int i = 0; i < 5; i++) {
		char result4[24];
		Read(fd4[i], result4, 24);	// fd4[0] should give error
	}
	printf("+----------+\n");

	printf("+---TEST 5---+\ntry to read deleted file\n");
	int fd5a = Open("test5");
	int fd5b = Open("test5");
	Write(fd5a, "This will be deleted", 20);
	Delete(fd5a);
	char result5[20];
	Read(fd5b, result5, 20);
	printf("+----------+\n");
}

void CloseTest() {
	printf("+---TEST 0---+\nclose unopened file\n");
	Close(0);
	printf("+----------+\n");

	printf("+---TEST 1---+\nclose file, reopen and read it\n");
	int fd1 = Open("readtest1");
	Write(fd1, "This should be here when I read again.", 38);
	Close(fd1);
	fd1 = Open("readtest1");
	char result1[38];
	Read(fd1, result1, 38);
	printf("+----------+\n");

	printf("+---TEST 2---+\nopen file multiple times, close one and read it using other descriptors\n");
	int fd2a = Open("readtest2");
	int fd2b = Open("readtest2");
	Write(fd2a, "hello", 5);
	Close(fd2a);
	char result2[5];
	Read(fd2b, result2, 5);
	printf("+----------+\n");
}

void DeleteTest() {
	printf("+---TEST 0---+\ndelete file that does not exist\n");
	Delete("dne");
	printf("+----------+\n");

	printf("+---TEST 1---+\ndelete file, create new one with same name\n");
	char fname1[12] = "deletetest1\0";
	int fd1 = Open(fname1);
	Write(fd1, "this will be deleted", 10);
	Delete(fname1);
	fd1 = Open(fname1);
	char result1[10];
	Read(fd1, result1, 10); 	// should give eof warning
	printf("+----------+\n");

	printf("+---TEST 2---+\ndelete file that is open in multiple descriptors\n");
	char fname2[12] = "deletetest2\0";
	int fd2a = Open(fname2);
	int fd2b = Open(fname2);
	Write(fd2a, "testing testing 123", 19);
	Delete(fname2);
	char result2[19];
	Read(fd2b, result2, 19);	// should give file not exist error
	printf("+----------+\n");


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

	// Test all cases for open
	//OpenTest();
	WriteTest();
	/*
	int fd1 = Open("myfile");

	Write(fd1, "hi this is my file. it only prints half.\n", 20);

	Close(fd1);

	fd1 = Open("myfile");

	char buffer1[20];
	Read(fd1, buffer1, 20);

	List();

	Delete("myfile");

	fd1 = Open("myfile");

	Read(fd1, buffer1, 20);
	*/
	/*
	int fd2 = Open("secret");

	char long_str[1000];
	memset(long_str, 'a', 1000);
	Write(fd2, long_str, 1000);

	char buffer0[1000];
	Read(fd2, buffer0, 1000);

	int fd3 = Open("thirdfile");
	int fd4 = Open("michael");

	List();

	int bytes_to_read = 20;
	char buffer[bytes_to_read];
	Read(fd1, buffer, bytes_to_read);
	printf("Reading fd %d:\n%s", fd1, buffer);

	char buffer2[10];
	Read(10, buffer2, 10);

	List();
*/
exit (0);
}
