/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
 // Based on file provided on canvas
#include "ssnfs.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define BLOCK_SIZE 512
#define FILE_SIZE 64	// blocks
#define DISK_SIZE 16 // MB
#define TABLE_SIZE 20	// number of files that may be open at a time

/*
Must use a linux file (.dat) as a virtual disk to store client files.
Virtual disk is a sequence of blocks containing 512B with max size of 16MB (32,000 blocks)
Users are given a home directory, but cannot create subdirectories (handle this case)
File table maintains information about files such as: user, fd, file pointer.
This persists such that memory can be restored if client crashes.
	i.e. if client crashes, the files remain open
	if server crashes, file table is preserved and can be loaded
*/

const char * vm_filename = "virtual_mem.dat";
const char * ft_filename = "file_table.dat";
//char * dict_filename = "file_dict.dat";

struct table_entry {
	int block_id;	// id of block at which file starts
	int fd;				// file descriptor
	int fp;				// location of file pointer
	int op;				// operation id. what user is doing to file: 0-open, 1-read, 2-write
};

// structure of a file stored in the virtual memory.
// note that there is no restriction on username and filename length. This will
// cut into the space available for data, but it is the user's responsibility to
// give names that do not take up too much space.
struct file_info {
  char user[10];
	char name[10];
	char * data;
};

// TODO: try storing a dictionary of which users own which files

/*
Creates the virtual disk and file table if they do not exist.
If file table exists, its state is restored (TODO)
*/
void init_disk() {
 int vm = open(vm_filename, O_RDONLY);
 if (vm < 0) {
	 vm = open(vm_filename, O_CREAT);
	 printf("virtual memory created.\n");
 }

 int ft = open(ft_filename, O_RDONLY);
 if (ft < 0) {
	 ft = open(ft_filename, O_CREAT);
	 printf("file table created.\n");
 }

 close(vm);
 close(ft);
}


/*
given a table, write to the file table file.
called at the end of rpc operations that modify the table
*/
void update_table(struct table_entry * table) {

}

/*
check the file table if the file is already open.
if so, return the table entry, null otherwise
*/
struct table_entry is_file_open(char * username, char * filename) {
	int table = open(ft_filename, O_RDONLY);
	int mem = open(vm_filename, O_RDONLY);
	struct table_entry entry;
	struct file_info info;

	// check if the file is open in the file table
	for (; read(table, &entry, sizeof(entry)) > 0;) {
		// seek to location of entry.
		int loc = entry.block_id * BLOCK_SIZE * FILE_SIZE;
		lseek(mem, loc, SEEK_SET);
		read(mem, &info, 20);	// only read username and filename at first
		printf("byte: %d user: %s file: %s\n", loc, info.user, info.name);
		if (strcmp(info.name, filename)==0 && strcmp(info.user, username)==0) {
			read(mem, &info, BLOCK_SIZE * FILE_SIZE);
			break;
		}
	}

	close(table);
	close(mem);
	return entry;
}

/*
searches virtual disk and file table for a file belonging to a specific user.
returns: block idx in which file exists.
*/
int file_exists(char * username, char * filename) {

}

/*
creates a file and adds it to the dictionary of files.
returns: file descriptor of new file
*/
int create_file(char * username, char * filename, struct file_info * f) {
	//struct file_info f;
	memcpy(f->user, username, 10);
	memcpy(f->name, filename, 10);
	f->data = (char*)malloc(FILE_SIZE*BLOCK_SIZE);
	// TODO: Insert file in next free space. Do this after implementing delete
	int mem = open(vm_filename, O_RDWR);
	lseek(mem, -1, SEEK_END);
	// TODO: check if memory is full
	int block_id = mem;
	printf("\nfile size = %d size of vm: %d\n", sizeof(*f), mem);
	write(mem, f, sizeof(*f));

	printf("user: %s created file: %s\n", f->user, f->name);
	printf("size of vm: %d\n", mem);
	close(mem);
	return block_id;
}

/*
Opens or creates a file of name provided by client.
File can only be 64 blocks long and is allocated on creation
*/
open_output * open_file_1_svc(open_input *argp, struct svc_req *rqstp)
{
	init_disk();

	static open_output  result;

	result.fd=20;
	result.out_msg.out_msg_len=10;
	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
  strcpy(result.out_msg.out_msg_val, (*argp).file_name);
	printf("In server: filename recieved:%s\n",argp->file_name);
	printf("In server username received:%s\n",argp->user_name);
	//	fflush((FILE *) 1);

	// TODO: create a file then read it just to make sure this is all good and dandy.
	// No need to enforce checking, lets just get files saved and read.
	//TODO: check if file exists or is open before creating
	struct file_info file;
  struct table_entry entry;
	entry.block_id = create_file(argp->user_name, argp->file_name, &file);
	entry.fd = 20;
	entry.op = 0;
	// for now just append to the file table
	// open the file aka add it to file table
	int table = open(ft_filename, O_RDWR);
	// TODO: check if file table is full
	// TODO: fill in next available entry
	lseek(table, 0, SEEK_END);
	write(table, &entry, sizeof(entry));
	close(table);
	// check if file exists
	// check if file is already open
	// add file to file table

	is_file_open(argp->user_name, argp->file_name);
	return &result;
}

read_output * read_file_1_svc(read_input *argp, struct svc_req *rqstp)
{
	static read_output  result;



	return &result;
}

write_output * write_file_1_svc(write_input *argp, struct svc_req *rqstp)
{
	static write_output  result;



	return &result;
}

list_output * list_files_1_svc(list_input *argp, struct svc_req *rqstp)
{
	static list_output  result;



	return &result;
}

delete_output * delete_file_1_svc(delete_input *argp, struct svc_req *rqstp)
{
	static delete_output  result;



	return &result;
}

close_output * close_file_1_svc(close_input *argp, struct svc_req *rqstp)
{
	static close_output  result;



	return &result;
}
