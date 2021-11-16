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
#include <stdlib.h>

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
	int fd;				// file descriptor - where in vm file is
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
	char data[FILE_SIZE*BLOCK_SIZE-20];
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
 }

 int ft = open(ft_filename, O_RDONLY);
 if (ft < 0) {
	 ft = open(ft_filename, O_CREAT);
 }

 close(vm);
 close(ft);
}


/*
given a table, write to the file table file.
called at the end of rpc operations that modify the table
*/
void update_table(struct table_entry changed_entry) {
	int table = open(ft_filename, O_RDWR);
	lseek(table, 0, SEEK_SET);
	struct table_entry entry;

	for (; read(table, &entry, sizeof(entry)) > 0;) {
		// seek to location of entry.
		if (entry.fd == changed_entry.fd) {
			lseek(table, -sizeof(entry), SEEK_CUR);
			write(table, &changed_entry, sizeof(changed_entry));
			break;
		}
	}

	close(table);
}

/*
given the index of where a file is located, return a file info.

*/
struct file_info get_open_file(int loc) {
	struct file_info file;
	int mem = open(vm_filename, O_RDONLY);
	lseek(mem, loc, SEEK_SET);
	read(mem, &file, FILE_SIZE*BLOCK_SIZE-1);	// only read username and filename at first
	//printf("size of memory: %ds size of file: %d\n", lseek(mem, 0, SEEK_END), sizeof(file));
	close(mem);
	return file;
}

/*
check the file table if the file is already open. optionally enter the file descriptor
if so, return table entry
*/
struct table_entry is_file_open(char * username, char * filename, int fd) {
	int isopen = -1;
	int table = open(ft_filename, O_RDONLY);
	int mem = open(vm_filename, O_RDONLY);
	struct table_entry entry;
	struct file_info info;

	lseek(table, 0, SEEK_SET);
	// check if the file is open in the file table
	for (; read(table, &entry, sizeof(entry)) > 0;) {
		// seek to location of entry.
		int loc = entry.fd;// * BLOCK_SIZE * FILE_SIZE;
		lseek(mem, loc, SEEK_SET);
		read(mem, &info, 20);	// only read username and filename at first
		if ((strcmp(info.name, filename)==0 && strcmp(info.user, username)==0) || fd == entry.fd) {
			isopen = 0;
			break;
		}
	}

	lseek(mem, 0, SEEK_SET);
	close(table);
	close(mem);
	if (isopen == -1) {
		entry.fd = -1;
		entry.fp = -1;
		entry.op = -1;
		printf("entry not found. file is not open\n");
	}
	else {
		printf("found entry-> fd:%d, fp:%d, op:%d\n", entry.fd, entry.fp, entry.op);
	}
	return entry;
}

/*
searches virtual disk for a file belonging to a specific user.
returns: block idx in which file exists.
*/
int file_exists(char * username, char * filename) {
	int exists = -1;
	int mem = open(vm_filename, O_RDONLY);
	struct file_info info;

	for (; read(mem, &info, sizeof(info)) > 0;) {
		if (strcmp(info.name, filename)==0 && strcmp(info.user, username)==0) {
			exists = read(mem, &info, BLOCK_SIZE * FILE_SIZE) - sizeof(info);
			break;
		}
	}
	close(mem);

	return exists;
}

/*
creates a file and adds it to the dictionary of files.
returns: file descriptor of new file (location of where file was stored)
*/
int create_file(char * username, char * filename) {
	struct file_info f;
	memcpy(f.user, username, 10);
	memcpy(f.name, filename, 10);
	// TODO: Insert file in next free space. Do this after implementing delete
	int mem = open(vm_filename, O_RDWR);
	int loc = lseek(mem, 0, SEEK_END);
	// TODO: check if memory is full

	printf("memory used: %.2f of %d\n", ((double)loc+sizeof(f))/1000000, DISK_SIZE);
	if ((loc+sizeof(f))/1000000 > DISK_SIZE) {
		printf("memory is full!\n");
	}
  write(mem, &f, sizeof(f));

	printf("user: %s created file: %s\n", f.user, f.name);
	close(mem);
	return loc;
}

/*
Opens or creates a file of name provided by client.
File can only be 64 blocks long and is allocated on creation
*/
open_output * open_file_1_svc(open_input *argp, struct svc_req *rqstp) {
	init_disk();
	printf("In server: filename recieved:%s\n",argp->file_name);
	printf("In server username received:%s\n",argp->user_name);

  struct table_entry entry = is_file_open(argp->user_name, argp->file_name, -1);
	if (entry.fd == -1) {
		// file isn't open get it from memory
		int loc = file_exists(argp->user_name, argp->file_name);
		if (loc == -1) {
			// file does not exist, so create it
			entry.fd = create_file(argp->user_name, argp->file_name);
			entry.fp = 0;
			entry.op = 0;
			// for now just append to the file table
		}
		// open the file aka add it to file table
		int table = open(ft_filename, O_RDWR);
		// TODO: check if file table is full
		// TODO: fill in next available entry
		int tbl_size = lseek(table, 0, SEEK_END);
		write(table, &entry, sizeof(entry));
		printf("added table entry for %s/%s at block %d. table is now size: %d\n", argp->user_name, argp->file_name, entry.fd, lseek(table, 0, SEEK_END));
		close(table);
	}

	static open_output result;
	result.fd=entry.fd;
	result.out_msg.out_msg_len=10;
	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
  strcpy(result.out_msg.out_msg_val, (*argp).file_name);

	return &result;
}

read_output * read_file_1_svc(read_input *argp, struct svc_req *rqstp) {
	init_disk();

	// check if file is open. if not then do nothing and send err msg
	static read_output  result;

	free(result.out_msg.out_msg_val);
	printf("user: %s requesting to read %d bytes from fd: %d\n", argp->user_name, argp->numbytes, argp->fd);

	struct table_entry entry = is_file_open(argp->user_name, "", argp->fd);
	int num_bytes_to_read = argp->numbytes;
	if (entry.fd == -1) {
		// file is not open
		char * message = "file with given descriptor is not open or does not exist.\n";
		result.out_msg.out_msg_len=sizeof(message);
		result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
		strcpy(result.out_msg.out_msg_val, message);
	}
	else {
		// get file
		struct file_info file = get_open_file(entry.fd);
		printf("file: %s exists.\n", file.name);
		// don't read past file size
		int available_space = (FILE_SIZE*BLOCK_SIZE) - 20;	// can use full filesize because entry.fp initialized to 20
		if (available_space < num_bytes_to_read) {
			num_bytes_to_read = available_space;
		}
		if (num_bytes_to_read >= sizeof(file.data)) {
			num_bytes_to_read = sizeof(file.data);
		}

		char buffer[num_bytes_to_read];
		memcpy(buffer, &file.data, num_bytes_to_read-1);
		//entry.fp+=num_bytes_to_read;
		entry.op = 1;

		result.out_msg.out_msg_len=num_bytes_to_read;
		result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
		memcpy(result.out_msg.out_msg_val, buffer, num_bytes_to_read);
		printf("read file: %s from user %s\n", file.name, file.user);

		// update the file table and save the new fp
		update_table(entry);
	}

	return &result;
}

write_output * write_file_1_svc(write_input *argp, struct svc_req *rqstp)
{
	init_disk();

	static write_output  result;
	free(result.out_msg.out_msg_val);
	printf("user: %s requesting to write to file with descriptor: %d\n", argp->user_name, argp->fd);

	struct table_entry entry = is_file_open(argp->user_name, "", argp->fd);
	int num_bytes_to_write = argp->numbytes;

	if (entry.fd == -1) {
		// file is not open
		char * message = "file with given descriptor is not open or does not exist.\n";
		result.out_msg.out_msg_len=sizeof(message);
		result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
		strcpy(result.out_msg.out_msg_val, message);
	}
	else {
		// get file
		struct file_info file = get_open_file(entry.fd);
		// don't read past file size
		int available_space = (FILE_SIZE*BLOCK_SIZE) - entry.fp - 20; // total file size needs to be subtracted by 20
		printf("want to write %d bytes into available space = %d\n", num_bytes_to_write, available_space);
		if (available_space < num_bytes_to_write) {
			num_bytes_to_write = available_space;
		}

		int mem = open(vm_filename, O_RDWR);
		lseek(mem, entry.fd+entry.fp+20, SEEK_SET);
		write(mem, argp->buffer.buffer_val, num_bytes_to_write);
		printf("memory written\n");
		entry.fp+=num_bytes_to_write;
		entry.op = 2;
		char * message = (char *)malloc(512);
		sprintf(message, "%d bytes written to %s\n", num_bytes_to_write, file.name);
		result.out_msg.out_msg_len=sizeof(message);
		result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
		strcpy(result.out_msg.out_msg_val, message);
		printf("created message: %s\n", result.out_msg.out_msg_val);
		printf("user %s wrote %d bytes to file: %s\n", file.user, num_bytes_to_write, file.name);
		close(mem);
		// update the file table and save the new fp
		update_table(entry);
	}

	return &result;
}

/*
lists all the files in the user's directory
*/
list_output * list_files_1_svc(list_input *argp, struct svc_req *rqstp)
{
	init_disk();
	// TODO: not reaching this line
	printf("server: listing files\n");
	static list_output result;
	free(result.out_msg.out_msg_val);

	// append file names to the result
	int mem = open(vm_filename, O_RDONLY);
	struct file_info info;
	int n_files = 0;
	char * files = malloc(12); // 10 for filename, 1 for newline
	memset(files, ' ', 12);
	int range = lseek(mem, 0, SEEK_END) / (FILE_SIZE*BLOCK_SIZE);
	printf("there are %d files in memory of size: %d\n", range, range*FILE_SIZE*BLOCK_SIZE);

	// check if the file is open in the file table
	for (int i = 0; i < range; i++) {
		lseek(mem, i*FILE_SIZE*BLOCK_SIZE, SEEK_SET);
		printf("idx: %d\n", i*FILE_SIZE*BLOCK_SIZE);
		read(mem, &info, 20);
		printf("%s/%s\n", info.user, info.name);
		if (strcmp(info.user, argp->user_name)==0) {
			// append filename
			//printf("n_files: %d, file list:\n%s", n_files, files);
			char temp[n_files*11];
			memset(temp, ' ', n_files*11);
			memcpy(temp, files, n_files*11);
			printf("copied %d bytes: %s \n", sizeof(temp), temp);

			// resize files array
			n_files += 1;
			free(files);
			files = malloc(n_files*11);
			memset(files, ' ', n_files*11);
			memcpy(files, temp, (n_files-1)*11);
			strcpy(files, info.name);
			//memcpy(&files+(n_files-1)*11, info.name, sizeof(info.name));
			//memcpy(&files+(n_files-1)*12, info.name, sizeof(info.name));
			files[n_files*11-1] = '\n';
			printf("%d: %s\n", n_files, files);
			//free(temp);
		}
	}
	close(mem);
	printf("files found\n");
	result.out_msg.out_msg_len = n_files*12;
	result.out_msg.out_msg_val = malloc(n_files*12);
	printf("reply allocated\n");
	strcpy(result.out_msg.out_msg_val, files);
	printf("reply constructed\n");
	//free(files);
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
