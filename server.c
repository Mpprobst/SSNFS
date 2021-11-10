/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */
 // Based on file provided on canvas
#include "ssnfs.h"

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

char * vm_filename = "virtual_mem.dat";
char * ft_filename = "file_table.dat";
char * dict_filename = "file_dict.dat";

struct table_entry {
	int block_id;	// id of block at which file starts
	char user[10];// user controlling file
	char file[10];
	int fd;				// file descriptor
	int fp;				// current position of file pointer
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
 FILE * vm = fopen(vm_filename, "r");
 if (!vm) {
	 vm = fopen(vm_filename, "w");
	 printf("virtual memory created.\n");
 }

 FILE * ft = fopen(ft_filename, "r");
 if (!ft) {
	 ft = fopen(ft_filename, "w");
	 printf("file table created.\n");
 }

 FILE * dict = fopen(dict_filename, "r");
 if (!dict) {
	 dict = fopen(dict_filename, "w");
	 printf("file dictionary created.\n")
 }

 fclose(vm);
 fclose(ft);
 fclose(fd);
}

/*
opens the file table file and returns an array describing it
intended to be called only once per rpc operation and modified.
at the end of the rpc operation, call update_table
*/
struct table_entry * get_file_table() {
	struct table_entry table[TABLE_SIZE];
	// read the table file and populate this array
	int fd = open(ft_filename, "r");
	struct table_entry entry;
	for (int i = 0; read(fd, &entry, sizeof(entry)) > 0; i++) {
		memcpy(table[i], entry, sizeof(entry));
	}
	return table;
}

/*
given a table, write to the file table file.
called at the end of rpc operations that modify the table
*/
void update_table(struct table_entry * table) {

}

/*
check the file table if the file is already open. pass in a table so file is
only read once per rpc call
if so, return idx, else return -1
*/
int is_file_open(char * username, char * filename, struct table_entry * table) {
	// check if the file is open in the file table
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (strcmp(table[i].file, filename)==0 && strcmp(table[i].user, username)==0) {
			return i;
		}
	}

	return -1;
}

/*
searches virtual disk and file table for a file belonging to a specific user.
returns: block idx in which file exists.
*/
int file_exists(char * username, char * filename) {

}

// UNTESTED
struct file_info get_file_from_memory(char * username, char * filename, int block_idx=-1) {
	struct file_info f;
	FILE * mem = fopen(vm_filename, "r");
	long memlen = ftell(mem);

	if (block_idx == -1){
		// scan block by block until we find the file
		for (int i = 0; i < memlen; i+=BLOCK_SIZE) {
			memcpy(f.user, mem+i, sizeof(f.user));
			memcpy(f.name, mem+i+sizeof(f.user), sizeof(f.name));
			if (strcmp(f.user, username)==0 && strcmp(f.name, filename)==0) {
				block_idx = i;
				break;
			}
		}
	}

	// index the block directly and populate it into a file_info.
	memcpy(f, mem+block_idx, sizeof(f));
	//memcpy(f.user, mem+block_idx, sizeof(f.user));
	//memcpy(f.name, mem+block_idx+sizeof(f.user), sizeof(f.name));
	//memcpy(f.data, mem+block_idx+sizeof(f.user)+sizeof(f.name));

	return f;
}

/*
creates a file and adds it to the dictionary of files.
returns: file descriptor of new file
*/
int create_file(char * username, char * filename) {
	struct file_info f;
	FILE * mem = fopen(vm_filename, "a");
	long memlen = ftell(mem);

	memcpy(f.user, username);
	memcpy(f.name, filename);
	f.data = (char*)malloc(FILE_SIZE*BLOCK_SIZE);
	// append to memory
	fprintf(mem, "%s", f.user);
	fprintf(mem, "%s", f.name);
	fprintf(mem, "%s", f.data);
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

	create_file(argp->user_name, argp->file_name);
	// check if file exists
	// check if file is already open
	// add file to file table

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
