### SSNFS

This project implements a simple stateful network file server (SSNFS) that supports remote file service model.
Server and client are implemented as Sun RPC.

### Build Instructions

Note: The GNU readline library is required for this project to build.

To build the programs, simply run "make".  Running rpcgen is not necessary, as it is run by the Makefile.

To delete all programs, build files, and virtual disk files, simply run "make clean".

### Usage Instructions

To run the server program, simply run "./server".  If your rpc binder is not running in insecure mode, you may need to run as "sudo ./server".

To run the client program, simply run "./client hostname", where hostname is the name of the host running the server, e.g., "./client localhost" if the server is running on the same host.

The client accepts the following commands: create, list, delete, write, read, copy.
create:  Requires a filename.  Create a file with given name.
list:  Lists all files owned by user.
delete: Requires a filename.  Deletes file with given name.
write: Requires a filename, offset, write length, and data.  Writes write length bytes of data to the file with given name at given offset.
read: Requires a filename, offset, and read length.  Reads read length bytes from file with given name from given offset.
copy: Requires a source filename and destination filename.  Copies data in source file to destination file.  Destination file must already exists.


### Notes
For this project, I made the following assumptions regarding the filesystem:  There will be a maximum of 100 files.  (This can be altered by changing the constant MAX_FILES in server.c.  Virtual disk files will have to be rebuilt for the change to take effect.)  There is no maximum number of user (except indirectly by the number of possible files).  Files will be at most 64 blocks long.  A file's size is implicitly defined by the number of blocks allocated to it: when a block is allocated to a file, it is filled with null bytes and the null bytes are assumed to be part of the file.  This may lead to unintuitive behaviors in the read and write methods.  Since blocks are always allocated in multiples of 8, I define a disk page as 8 blocks and all operations are defined in terms of pages instead of blocks.  The maximum number of pages can also by changed by altering the MAX_PAGES constant in server.c.  Again, virtual disk file will have to be rebuilt for the change to take effect.

The virtual disk is define by three files: a file table file (files.dat), a page table file (pages.dat), and a disk data file (disk.dat).  If they do not all exist, they will be created upon any rpc call.  Otherwise, the existing copies will be used.  Each entry in files.dat contains a username, a filename, the number of pages used, and a list of page numbers.  The file pages.dat is a list of bytes indicating if the page is currently used or not.  The file disk.dat contains the disk pages data in order of page number.  All files are set to all null bytes upon creation.  All rpc commands work by modifying these files.

For the client program, I assumed that the user would not enter invalid data, so there is only minimal error checking.  Because I use strtok to parse the input, I also assumed that the data given to the write command contains no spaces.

I used the GNU readline library for the client interface.  This allows the user to scroll through the session history and edit line, which greatly simplifies testing and use.

Wherever possible, I used the same reply and error messages as shown in the assignment 4 prompt, and elsewhere attempted to maintain the style of message illustrated in the prompt.  I modified the client prompt by adding a space to make the interface more readable.

A large amount of debugging information is output by the server process.  I have not attempted to remove any of this from my submission.
