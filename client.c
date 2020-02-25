#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>	// "struct sockaddr_in"
#include <arpa/inet.h>	// "in_addr_t"
#include <malloc.h>
#include <dirent.h>
#include "myftp.h"
void list_file(int fd, struct message_s header){
	// memset(command,0,5);
	int len;
	int i;
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xA1;
	// buf.type=0xA1;
	header.length=sizeof(header);

	header.length = htonl(header.length);
	// buf=htonl(buf);
	if((len=(send(fd,&header,sizeof(header),0)))<0){
		perror("can not send request for list\n");
	}
	printf("send the command to server\n");
	struct message_s message_from_server;
	// printf("size : %d %d\n",sizeof(message_from_server),sizeof(message_from_server.payload));
	if((len=(recv(fd,&message_from_server,sizeof(message_from_server),0)))<0){
		perror("can not recv the message from server\n");
	}
	// printf("recv the message\n");
	else{
		char payload[PAYLEN];
		message_from_server.length = ntohl(message_from_server.length);
		printf("size:%d %d \n",message_from_server.length,(int)(message_from_server.length-10-1)/sizeof(payload));
		for( i=0;i<=(int)(message_from_server.length-10-1)/sizeof(payload);i++){
			printf("the %dth\n",i);
			if((len=(recv(fd,payload,sizeof(payload),0)))<0){
				perror("can not receive the file list from server\n");
			}
			else{
				printf("%s\n",payload);
			}
		}
	}

	printf("Done\n");
}

void get_file(int fd,   char * command){
	struct message_s header;
	int i;
	int len;
	if(command[3]!=' '){
		printf("c:%s\n",command[3]);
		perror("wrong format\n");
		exit(1);
	}
	for (i=3;i<strlen(command);i++){
		if(command[i]!=' '){
			break;
		}
	}
	const char *tmp=&command[i];
	char filename[32];
	char payload[PAYLEN]="";
	strcpy(filename,tmp);
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xB1;
	
	strcpy(payload,filename);
	header.length=10+strlen(filename);

	header.length = htonl(header.length);

	if((len=send(fd,&header,sizeof(header),0))<0){
		perror("can not send request for file\n");
	}
	else{
		// printf("Done to send request for file\n");

		// for(int i=0;i<=((header.length-10-1)/sizeof(payload));i++){
		// 	printf("%dth iteration\n",i);
			if((len=send(fd,payload,sizeof(payload),0))<0){
				perror("can not send the payload to server\n");
			}
		// }
		struct message_s result_of_get;
		if((len=(recv(fd,&result_of_get,sizeof(result_of_get),0)))<0){
			perror("cannot receive message from server\n");

		}
		if(result_of_get.type==0xB2){
			printf("successfully find the file\n");
			recv_file_data(fd, filename,"");
		}
		else{
			printf("fail to find the file\n");
		}
	}

}

void put_file(int fd, char command[]){
	struct message_s header;
	int i;
	int len;
	if(command[3]!=' '){
		printf("c:%s\n",command[3]);
		perror("wrong format\n");
		exit(1);
	}
	for (i=3;i<strlen(command);i++){
		if(command[i]!=' '){
			break;
		}
	}
	const char *tmp=&command[i];
	char filename[32];
	char payload[PAYLEN]="";
	int f_exist=0;
	strcpy(filename,tmp);
	strcpy(payload,filename);
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.type=0xC1;
	
	header.length = 10 + strlen(filename);
	header.length = htonl(header.length);

	DIR * dir;
	struct dirent * ptr;
	if ((dir=opendir("."))==NULL){
		perror("can not find the responding data\n");
	}
	while((ptr=readdir(dir))!=NULL){
		if(strcmp(payload,ptr->d_name)==0){
			f_exist=1;
			break;
		}
	}
	if(f_exist==0){
		// printf("can not find the file locally\n" );
		perror("can not find the file locally\n");
		exit(1);
	}
	else{
		printf("find the file\n");
		if((len=(send(fd,&header,sizeof(header),0)))<0){
			perror("can not send request to client");
		}
		printf("send the header\n");
		if((len=(send(fd,payload,sizeof(payload),0)))<0){
			perror("can not send request to client");
		}
		printf("send the file name\n");
		struct message_s message_from_server;
		if((len=(recv(fd,&message_from_server,sizeof(message_from_server),0)))<0){
			perror("can not recv request to client");
		}
		if(message_from_server.type==0xC2){
				tranp_file_data(fd,filename,"");
			}
	}
}
void main_task(in_addr_t ip, unsigned short port)
{
	struct message_s header;
	// struct all_data buf;
	int fd;
	char command[100];
	memset(command,0,100);
	struct sockaddr_in addr;
	unsigned int addrlen=sizeof (struct sockaddr_in);
	int len;
	fd=socket(AF_INET, SOCK_STREAM,0);
	if(fd== -1){
		perror("socket()");
		exit(1);
	}
	// scanf("%s", command);
	// printf("command: %s",command);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=ip;
	addr.sin_port=htons(port);
	if(connect(fd, (struct sockaddr *) &addr, addrlen)==-1){
		perror("connect()");
		exit(1);
	}
	while(strcmp(command,"list")!=0 && strncmp(command,"get",3)!=0&& strncmp(command,"put",3)!=0){
		printf("command: (1) send nothing, or (2) send 4 bytes\n");
		// scanf("%s", command);
		// gets(command);
		fgets(command,100,stdin);
		command[strlen(command)-1]='\0';
		printf("command: %s\n",command);
		// scanf("%s", command);
	}
	// printf("what is wrong\n");
	if (strcmp(command,"list")==0){
		// printf("what is wrong\n");
		list_file(fd,header);
	}
	if (strncmp(command,"get",3)==0){
		get_file(fd,command);
	}
	if (strncmp(command,"put",3)==0){
		put_file(fd,command);
		// printf("Sent nothing\n");
	}

	
	close(fd);
}
int main(int argc, char **argv){
	in_addr_t ip;
	unsigned short port;
	if (argc !=3 ){
		fprintf(stderr,"Usage: %s [IP address] [port]\n", argv[0]);
		exit(1);
	}
	if ((ip=inet_addr(argv[1]))==-1){
		perror("inet_addr()");
		exit(1);
	}
	port=atoi(argv[2]);
	main_task(ip,port);
	return 0;
}
