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
#define USERNAME_LEN 15
#define FILENAME_LEN 20
#define MAX_USERS 10
#define FILES_PER_USER 50

/*
Must use a linux file (.dat) as a virtual disk to store client files.
Virtual disk is a sequence of blocks containing 512B with max size of 16MB (32,000 blocks)
Users are given a home directory, but cannot create subdirectories (handle this case)
File table maintains information about files such as: user, fd, file pointer.
This persists such that memory can be restored if client crashes.
	i.e. if client crashes, the files remain open
	if server crashes, file table is preserved and can be loaded
*/

struct table_entry {
	//int fd;				// unique identifier for the file
	char username[USERNAME_LEN];
	char filename[FILENAME_LEN];
	int fp;				// location of file pointer
};

// structure of a file stored in the virtual memory.
// note that there is no restriction on username and filename length. This will
// cut into the space available for data, but it is the user's responsibility to
// give names that do not take up too much space.
struct file_info {
  char username[USERNAME_LEN];
	char filename[FILENAME_LEN];
	int blocks[FILE_SIZE];
	int curr_size;	// how many bytes have been written to the file
};

// 16MB of data stores server files
const char * memory_filename = "memory.dat";
// contains info on which files each user owns
const char * metadata_filename = "metadata.dat";
// file table tracking what files are currently open
struct table_entry table[TABLE_SIZE];
int initialized = -1;

/*
Creates the disk, metadata, and file table if they do not exist.
*/
void init_disk() {	// char * username
	if (initialized == -1) {
		int mem = open(memory_filename, O_RDONLY);
		if (mem < 0) {
			mem = open(memory_filename, O_CREAT);
		}

		int meta = open(metadata_filename, O_RDONLY);
		if (meta < 0) {
			meta = open(metadata_filename, O_CREAT);
		}

		for (int i = 0; i < TABLE_SIZE; i++) {
			memset(table[i].username, ' ', USERNAME_LEN);
			memset(table[i].filename, ' ', FILENAME_LEN);
			table[i].fp = -1;
		}
		initialized = 1;
		close(mem);
		close(meta);
	}
}

/*
check the file table if the file is already open.
if so, return the index where the file exists
*/
int is_file_open(char * username, char * filename) {
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (strcmp(table[i].filename, filename)==0 && strcmp(table[i].username, username)==0) {
		return i;
		}
	}
	return -1;
}

/*
searches metadata for a file belonging to a specific user.
returns: -1 for failure, 1 for success
*/
struct file_info file_exists(char * username, char * filename) {
	int exists = -1;
	int meta = open(metadata_filename, O_RDONLY);
	struct file_info fi;
	for (; read(meta, &fi, sizeof(fi)) > 0;) {
		if (strcmp(fi.filename, filename)==0 && strcmp(fi.username, username)==0) {
			exists = 1;
			break;
		}
	}
	close(meta);
	if (exists == -1) {
		fi.curr_size = -1;
	}
	return fi;
}

/*
given a file descriptor, return metadata for the file
*/
struct file_info get_open_file(int fd) {
	struct table_entry entry = table[fd];
	return file_exists(entry.username, entry.filename);
}

/*
gets next available block in memory
*/
int get_free_block() {
	int mem = open(memory_filename, O_RDONLY);
	int meta = open(metadata_filename, O_RDONLY);

	int n_blocks = lseek(mem, 0, SEEK_END) / BLOCK_SIZE - 1;
	int blocks[n_blocks];
	for (int i = 0; i < n_blocks; i++) {
		blocks[i] = i;
	}

	struct file_info fi;
	for (; read(meta, &fi, sizeof(fi)) > 0;) {
		for (int i = 0; i < FILE_SIZE; i++) {
			blocks[fi.blocks[i]] = -1;
		}
	}

	for (int i = 0; i < n_blocks; i++) {
		if (blocks[i] > 0) {
			printf("Reallocating block %d\n", blocks[i]);
			return blocks[i];
		}
	}
	close(mem);
	return -1;
	//return add_block();
}

/*
appends a block of memory to disk.
returns block idx of new block. if memory is full, return -1
*/
int add_block() {
	int mem = open(memory_filename, O_RDONLY);
	int size = lseek(mem, 0, SEEK_END);
	printf("Adding block to memory\n");
	printf("memory used: %.2f of %d\n", ((double)size/1000000, DISK_SIZE));
	if ((size+BLOCK_SIZE)/1000000 > DISK_SIZE) {
		printf("memory is full!\n");
		return -1;
	}

	int n_blocks = size / BLOCK_SIZE;
	char blank[BLOCK_SIZE];
	memset(blank, ' ', BLOCK_SIZE);
	write(mem, blank, BLOCK_SIZE);
	close(mem);
	return n_blocks;
}

/*
creates file metadata and allocates memory for it on the disk
returns: metadata for new file
*/
struct file_info create_file(char * username, char * filename) {
	struct file_info fi;
	int meta = open(metadata_filename, O_RDWR);
	int size = lseek(meta, 0, SEEK_END);
	int meta_idx = -1;
	for (; read(meta, &fi, sizeof(fi)) > 0;) {
		// when file is deleted, curr size is set to -1
		meta_idx++;
		if (fi.curr_size == -1) {
			break;
		}
	}

	memcpy(fi.username, username, USERNAME_LEN);
	memcpy(fi.filename, filename, FILENAME_LEN);
	fi.curr_size = 0;
	for (int i = 0; i < FILE_SIZE; i++) {
		fi.blocks[i] = -1;
	}
	//if (meta_idx == -1) {
		// append to metadata file
	//	meta_idx = lseek(meta, 0, SEEK_END) / sizeof(fi) - 1;
	//}
	lseek(meta, meta_idx*sizeof(fi), SEEK_SET);
	write(meta, &fi, sizeof(fi));

	printf("user: %s created file: %s\n", fi.username, fi.filename);
	close(meta);
	return fi;
}

/*
Opens or creates a file of name provided by client.
File can only be 64 blocks long and is allocated on creation
*/
open_output * open_file_1_svc(open_input *argp, struct svc_req *rqstp) {
	printf("\nIn server: %s attempting to open file: %s\n", argp->user_name, argp->file_name);
	init_disk();
	static open_output result;
	char message[100];
	memset(message, ' ', 100);

	// get open file table entry
	int fd = -1;	// file descriptor will correlate to index in file table
	for (int i = 0; i < TABLE_SIZE; i++) {
		if (table[i].fp == -1) {
			fd = i;
			break;
		}
	}

	if (fd == -1) {
		strcpy(message, "ERROR: File not opened.\nFile table is full. Please close a file to open a new one.\n");
	}
	else {
		struct file_info fi = file_exists(argp->user_name, argp->file_name);
		if (fi.curr_size == -1) {
			// file does not exist, create it
			fi = create_file(argp->user_name, argp->file_name);
			sprintf(message, "Created new file: %s", fi.filename);
		}
		else {
			sprintf(message, "Opened existing file: %s\n", fi.filename);
		}
		// if table entry is free. if free, username and filename are blank strings
		strcpy(table[fd].username, fi.username);
		strcpy(table[fd].filename, fi.filename);
		table[fd].fp = 0;
		printf("added table entry for %s/%s at fd: %d\n", argp->user_name, argp->file_name, fd);
	}

	// prepare reply
	free(result.out_msg.out_msg_val);
	result.fd=fd;
	result.out_msg.out_msg_len=sizeof(message);
	result.out_msg.out_msg_val=(char *)malloc(result.out_msg.out_msg_len);
  strcpy(result.out_msg.out_msg_val, message);
	return &result;
}

read_output * read_file_1_svc(read_input *argp, struct svc_req *rqstp) {
	printf("\nIn server: %s attempting to read %dB from file: \n", argp->user_name, argp->numbytes, argp->fd);
	init_disk();
	// check if file is open. if not then do nothing and send err msg
	static read_output result;
	char * message;
	int message_size;

	struct file_info fi = get_open_file(argp->fd);
	// file is open
	if (fi.curr_size == -1) {
		message_size = 100;
		message = malloc(message_size);
		memset(message, ' ', message_size);
		sprintf(message, "ERROR: file with descriptor %d is not open\n", argp->fd);
	}
	else {
		table[argp->fd].fp = 0;
		// read the file
		int mem = open(memory_filename, O_RDONLY);
		int bytes_read = 0;
		char buffer[argp->numbytes];
		memset(buffer, ' ', argp->numbytes);
		int start = table[argp->fd].fp / BLOCK_SIZE;
		int max_read = fi.curr_size - table[argp->fd].fp;

		// TODO: if bytes to read > max_read return error that use r is tyring to read too much
		printf("max bytes to read = %d-%d=%d\nstarting block = ", fi.curr_size, table[argp->fd].fp, max_read, fi.blocks[start]);
		for (int i = start; (fi.blocks[i] > -1) && (bytes_read < max_read); i++) {
			int bytes_in_block = BLOCK_SIZE;
			if (i == start) {
				bytes_in_block -= table[argp->fd].fp % BLOCK_SIZE;
			}
			int bytes_to_read = argp->numbytes - bytes_read;
			if (bytes_to_read > bytes_in_block) {
				bytes_to_read = bytes_in_block;
			}
			int read_loc = lseek(mem, (fi.blocks[i] * BLOCK_SIZE)+table[argp->fd].fp, SEEK_SET);
			read(mem, &message+bytes_read, bytes_to_read);
		 	bytes_read += bytes_to_read;
			printf("read from fi.blocks[%d] = %d into message[%d]\n", i, fi.blocks[i], bytes_read);
			printf("read mem loc %d\n %d/%d bytes = %s\n", read_loc, bytes_to_read, bytes_read, message);
		}
		message_size = bytes_read;
		table[argp->fd].fp+=bytes_read;
		close(mem);
	}

	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_len=message_size;
	result.out_msg.out_msg_val=(char *) malloc(result.out_msg.out_msg_len);
	strcpy(result.out_msg.out_msg_val, message);

	return &result;
}

write_output * write_file_1_svc(write_input *argp, struct svc_req *rqstp)
{
	printf("\nIn server: %s writing %dB to fd:\n", argp->user_name, argp->numbytes, argp->fd);
	init_disk();
	static write_output result;
	int message_size = 100;
	char message[message_size];
	memset(message, ' ', message_size);

	struct file_info fi = get_open_file(argp->fd);
	// file is open
	if (fi.curr_size == -1) {
		sprintf(message, "ERROR: file with descriptor %d is not open\n", argp->fd);
	}
	else {
		int free_block = get_free_block();
		int mem = open(memory_filename, O_RDWR);
		int bytes_written = 0;
		while (bytes_written < argp->numbytes) {
			// get correct block from fi based on curr_size
			int curr_block = table[argp->fd].fp / BLOCK_SIZE;
			int idx = table[argp->fd].fp % BLOCK_SIZE;			// index into current block

			int bytes_to_write = argp->numbytes - bytes_written;
			int bytes_in_block = BLOCK_SIZE - idx;
			if (bytes_to_write > bytes_in_block) {
				bytes_to_write = bytes_in_block;
			}

			// if writing to a block which is not yet allocated, allocate it
			if (fi.blocks[curr_block] == -1) {
				int new_block = -1;
				if (free_block == -1) {
					// no free blocks
					new_block = add_block();
				}
				else {
					// use the free block, get next free block
					new_block = free_block;
					free_block = get_free_block();
				}

				fi.blocks[curr_block] = new_block;
				//curr_block = new_block;
			}
			printf("\twriting %d to fi.blocks[%d] = %d\n", bytes_to_write, curr_block, fi.blocks[curr_block]);
			// write to blocks 512 bytes at a time
			int mem_loc = lseek(mem, fi.blocks[curr_block]*BLOCK_SIZE+idx, SEEK_SET);
			char buf[bytes_to_write];
			memcpy(buf, &argp->buffer.buffer_val[bytes_written], bytes_to_write);
			printf("\twrote to addr: %d: %s\n", mem_loc, buf);
			write(mem, buf, bytes_to_write);
			bytes_written += bytes_to_write;
			table[argp->fd].fp += bytes_written;
		}
		close(mem);
		table[argp->fd].fp += bytes_written;
		sprintf(message, "successfully wrote %d bytes to fd %d", bytes_written, argp->fd);

		// update metadata with new info
		int meta = open(metadata_filename, O_RDWR);
		struct file_info info;
		for (; read(meta, &info, sizeof(info)) > 0;) {
			 if ((strcmp(fi.username, info.username) == 0) && (strcmp(fi.filename, info.filename) == 0)) {
				 lseek(meta, -sizeof(fi), SEEK_CUR);
				 write(meta, &fi, sizeof(fi));
			 }
		}
	}

	free(result.out_msg.out_msg_val);
	result.out_msg.out_msg_len = message_size;
	result.out_msg.out_msg_val = (char *)malloc(message_size);
	strcpy(result.out_msg.out_msg_val, message);

	return &result;
}

/*
lists all the files in the user's directory
*/
list_output * list_files_1_svc(list_input *argp, struct svc_req *rqstp)
{
	init_disk();
	printf("\nServer: listing files.\n");
	static list_output result;
	/*
	free(result.out_msg.out_msg_val);

	// append file names to the result
	int mem = open(memory_filename, O_RDONLY);
	struct file_info info;
	int n_files = 0;
	result.out_msg.out_msg_val = malloc(0);
	//char * files=malloc(0); // 10 for filename, 1 for newline
	int range = lseek(mem, 0, SEEK_END) / (FILE_SIZE*BLOCK_SIZE);
	printf("there are %d files in memory of size: %d\n", range, range*FILE_SIZE*BLOCK_SIZE);

	// check if the file is open in the file table
	for (int i = 0; i < range; i++) {
		lseek(mem, i*FILE_SIZE*BLOCK_SIZE, SEEK_SET);
		read(mem, &info, 20);
		if (strcmp(info.user, argp->user_name)==0) {
			// append filename
			char temp[n_files*11];
			memset(temp, ' ', n_files*11);
			strcpy(temp, result.out_msg.out_msg_val);

			// resize files array
			n_files += 1;
			free(result.out_msg.out_msg_val);
			result.out_msg.out_msg_val = malloc(n_files*11);
			memset(result.out_msg.out_msg_val, ' ', n_files*11);
			strcpy(result.out_msg.out_msg_val, temp);
			strcat(result.out_msg.out_msg_val, info.name);
			strcat(result.out_msg.out_msg_val, "\n");
		}
	}
	close(mem);
	printf("%d files owned by %s:\n%s", n_files, argp->user_name, result.out_msg.out_msg_val);
	result.out_msg.out_msg_len = n_files*11;
	*/
	return &result;
}

delete_output * delete_file_1_svc(delete_input *argp, struct svc_req *rqstp)
{
	static delete_output  result;
	// if file is open, close it
	// get the idx then replace it with a new file info (no specified user or name)


	return &result;
}

close_output * close_file_1_svc(close_input *argp, struct svc_req *rqstp)
{
	static close_output  result;

	// check if file is open
	// if so, remove it from the file table.

	return &result;
}
