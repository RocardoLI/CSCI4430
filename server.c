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
#include <dirent.h>
#include <malloc.h>
#include <pthread.h>
#include "myftp.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int client_count = 0;

void list_reply(int accept_fd){
	char payload[PAYLEN];
	memset(payload,0,PAYLEN);
	int len;
	DIR * dir;
	struct dirent * ptr;
	struct message_s header;
	int fn_len=1;
	header.length=0;
	// printf("i am in\n");
	if ((dir=opendir("data"))==NULL){
		perror("can not find the responding data");
	}
	while((ptr=readdir(dir))!=NULL){
		if(strlen(payload)+strlen(ptr->d_name)<=PAYLEN-1){
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			printf("name: %s\n",ptr->d_name);
		}
		else{
			fn_len+=1;
			memset(payload,0,PAYLEN);
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			printf("name: %s\n",ptr->d_name);
			
		}
	}
	// if(fn_len==0){
	// 	fn_len=1;
	// }
 
	//printf("what is wrong\n");
	// printf("the file list is: %s",filelist);
	closedir(dir);

	header.type=0xA2;
	// char tmpmessage[]="myftp";
	// header.protocol=(unsigned char *)tmpmessage ;
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';

	header.length=(fn_len*PAYLEN+10);

	header.length = htonl(header.length);


	printf("header: %d\n",header.length);
	memset(payload,0,PAYLEN);
	// strcpy(message_to_client.payload,filelist);
	if((len=(send(accept_fd,&header, sizeof(header),0)))<0){
		perror("can not send request to client");
	}
	else{
		while((ptr=readdir(dir))!=NULL){
		if(strlen(payload)+strlen(ptr->d_name)<=PAYLEN-1){
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			printf("name: %s\n",payload);
		}
		else{
			if((len=(send(accept_fd,payload,sizeof(payload),0)))<0){
				perror("can not send the file name to client\n");
			}
			memset(payload,0,PAYLEN);
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			printf("name: %s\n",ptr->d_name);
			
		}
		
	}
	if(fn_len==1){
			if((len=(send(accept_fd,payload,sizeof(payload),0)))<0){
				perror("can not send the file name to client\n");
			}
		}
	}
	printf("Done to send to client, len: %d\n",len);
	// free(message_to_client.payload);
}


void reply_request_file(int accept_fd, struct message_s buf, char payload[]){
	DIR * dir;
	struct dirent * ptr;
	struct message_s get_reply;
	int f_exist=0;
	int len;
	if ((dir=opendir("data"))==NULL){
		perror("can not find the responding data");
	}
	while((ptr=readdir(dir))!=NULL){
		// printf("file name:%s\n", buf.payload);
		if(strcmp(payload,ptr->d_name)==0){
			f_exist=1;
			get_reply.type=0xB2;
			get_reply.protocol[0]='m';
			get_reply.protocol[1]='y';
			get_reply.protocol[2]='f';
			get_reply.protocol[3]='t';
			get_reply.protocol[4]='p';
			get_reply.length=5+1+4;
			printf("find the file\n");
			break;
		}
	}

	get_reply.length = htonl(get_reply.length);

	if(f_exist==0){
		get_reply.type=0xB3;
		printf("cannot find the file\n");
	}
	if((len=(send(accept_fd,&get_reply,sizeof(get_reply),0)))<0){
		perror("can not send request to client");
	}
	if(f_exist==1){
		char path[]="data/";
		tranp_file_data( accept_fd, payload,path);
	}
	printf("Done to send to client\n"); 
}

void put_recv_file(int accept_fd){
	struct message_s header;
	int len;
	char payload[PAYLEN];
	char filename[PAYLEN];
	if((len=recv(accept_fd,payload,sizeof(payload),0))<0){
		perror("cannot send header to client\n");
	}
	strcpy(filename,payload);

	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';
	header.type=0xC2;
	header.length=10;

	header.length = htonl(header.length);

	if((len=send(accept_fd,&header,sizeof(header),0))<0){
		perror("cannot send header to client\n");
	}

	recv_file_data(accept_fd,filename,"data/");

}

void *pthread_loop(int* sDescriptor){
	int accept_fd = (int)sDescriptor;
	int len;
	struct message_s buf;
	 
		if ((len = (recv(accept_fd,&buf,sizeof(buf),0)))<0){
			perror("can not recv the commend");
			pthread_mutex_lock(&mutex);
				client_count--;
			pthread_mutex_unlock(&mutex);
		}
		
		if(buf.type==0xA1){
			list_reply(accept_fd);
		}
		if(buf.type==0xB1){
			unsigned char payload[PAYLEN];
			if ((len=(recv(accept_fd,payload,sizeof(payload),0)))<0){
				perror("can not recv the commend\n");
				pthread_mutex_lock(&mutex);
					client_count--;
				pthread_mutex_unlock(&mutex);
			}
			reply_request_file( accept_fd, buf,payload);
		}
		if(buf.type==0xC1){
			put_recv_file(accept_fd);
		}
		close(accept_fd);
	pthread_mutex_lock(&mutex);
		client_count--;
	pthread_mutex_unlock(&mutex);
	printf("%d\n",client_count);
}

void main_loop(unsigned short port){
	int fd, accept_fd, count;
	struct sockaddr_in addr;
	struct sockaddr_in tmp_addr;
	unsigned int addrlen= sizeof(struct sockaddr_in);
	fd=socket(AF_INET, SOCK_STREAM,0);
	if (fd==-1){
		perror("socket()");
		exit(1);
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family= AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(port);

	if(bind(fd, (struct sockaddr *) &addr, sizeof(addr))==-1)
	{
		perror("bind()");
		exit(-1);
	}
	if (listen(fd, 1024)==-1){
		perror("listen()");
		exit(1);
	}
	printf("[To stop the server: press Ctrl + C]\n");
	
	pthread_t thr;
	while(1){
		printf("%d\n",client_count);
		if(client_count<11){
			if((accept_fd=accept(fd,(struct sockaddr *) &tmp_addr, &addrlen))==-1){
				perror("accept()");
				exit(1);
			}
			pthread_mutex_lock(&mutex);
				client_count++;
			pthread_mutex_unlock(&mutex);
			if(pthread_create(&thr, NULL, pthread_loop, accept_fd)!=0){
				printf("fail to create thread\n");
				close(accept_fd);
				pthread_mutex_lock(&mutex);
					client_count--;
				pthread_mutex_unlock(&mutex);
			}
		}
	}
}
int main(int argc, char **argv){
	unsigned short port;
	if (argc!=2){
		fprintf(stderr, "Usage: %s [port]\n",argv[0]);
		exit(1);
	}
	port=atoi(argv[1]);
	main_loop(port);
	return 0;
} 
