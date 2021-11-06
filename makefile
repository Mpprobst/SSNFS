all: client server
client : client.o ssnfs_clnt.o  ssnfs_xdr.o  
	gcc -o client client.o ssnfs_clnt.o ssnfs_xdr.o
server: ssnfs_xdr.o  ssnfs_svc.o server.o
	gcc -o server server.o ssnfs_svc.o ssnfs_xdr.o 
client.o: ssnfs.h client.c 
	gcc -c client.c
server.o: ssnfs.h server.c
	gcc -c server.c
ssnfs_svc.o: ssnfs.h ssnfs_svc.c 
	gcc -c ssnfs_svc.c
ssnfs_clnt.o: ssnfs_clnt.c ssnfs.h
	gcc -c ssnfs_clnt.c 
ssnfs_xdr.o: ssnfs_xdr.c ssnfs.h
	gcc -c ssnfs_xdr.c

clean :
	rm  *.o *~

 
