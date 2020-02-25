.PHONY:  client mserver
mserver: mserver.o myftp.o client
	gcc -o mserver mserver.o myftp.o -lpthread
mserver.o: mserver.c myftp.h 
	gcc -c mserver.c -lpthread
client: client.o myftp.o
	gcc -o client client.o myftp.o
client.o: client.c myftp.h
	gcc -c client.c
myftp.o: myftp.c myftp.h
	gcc -c myftp.c

clean:
	rm mserver client mserver.o client.o myftp.o
