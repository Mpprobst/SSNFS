# SIMPLE STATEFUL NETWORK FILE SERVER
This is a solution to HW4 for CS570 instructed by Dr. Manivannan at the University of Kentucky.
The goal of this project is to implement a simple stateful network file server - that is the server will host files created by clients.
## USAGE
Compiling the server and client can be done using the makefile. Simply enter the command `make`.
Because the files are saved, between sessions, to clear memory you must delete the memory and metadata file. Do so with the `make clean` command.

Next, run the server with command `./server`

On another terminal, connect to the server with a client using command `./client <hostname> <request>` where the hostname is the name of the host running the server, and the request is one of the following commands:
The following requests can be made:
- `open <filename>`: opens existing file and tracks it in the file table. if the file does not exist, it is created. when a file is opened, its file pointer is set to 0. returns a file descriptor that is used to do further operations to the file.
- `close <file descriptor>`: closes a file using its given file descriptor, removing it from the file table.
- `write <file descriptor> <buffer> <bytes to write>`: writes specified number of bytes of given information to a file using its file descriptor. moves file pointer the same number of bytes written.
- `read <file descriptor> <bytes to read>`: reads a certain number of bytes into a buffer from a designated file. moves file pointer the same number of bytes read.
- `list`: returns a list of all files owned by the user.
- `delete <filename>`: deletes a file from memory, clearing the blocks of memory it was using and removes it from the metadata file.

### TEST SUITES
Optionally, you may enter commands that run test suites specific to each command which were used to test this program.
Do so with the following commands:
- `./client <hostname> opentest`
- `./client <hostname> closetest`
- `./client <hostname> writetest`
- `./client <hostname> readtest`
- `./client <hostname> listtest`
- `./client <hostname> deletetest`

## SPECIFICATIONS
These are requirements laid out by the assignment:
- The server and client are implemented as Sun RPC client and server.
- File data are stored in a linux style file segmented into blocks of 512 bytes.
- files may allocate a maximum of 64 blocks
- the maximum size of memory is 16MB
- server must be stateful containing file table tracking which files are open and the value of the file pointers of the open files.
## IMPLEMENTATION DETAILS
These are some decisions I made that were not specified by the assignment handout:
- users may create any number of files they want.
- file information are stored in a different metadata file that records the file's name, the user who owns the file, the file size, and which blocks in the data file belong to the file.
- files dynamically allocate memory as needed. that is, when writing to a file, if the user is writing more bytes than the file has allocated, the file will take on the next available block of memory.
- when a file is deleted, it frees the blocks of memory which it had been holding on to. those blocks which are freed have their data cleared and remain so until some other file requires it. that being said, blocks of memory held by a file do not have to be sequential.
- filename can be a maximum of 25 characters long.
- username can be a maximum of 15 characters long.

## LIMITATIONS
- the client assumes input will always be correct. it assumes that when an operation requires a file descriptor, that an integer will be entered and not some character or other variable type. if incorrect variable types are given, the request will likely not work as intended, but undesirable side effects may occur as these cases are not handled.
