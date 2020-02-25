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
#include <sys/malloc.h>
#include "myftp.h"

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
			if((len=(send(accept_fd,&payload,sizeof(payload),0)))<0){
				perror("can not send the file name to client\n");
			}
			memset(payload,0,PAYLEN);
			strcat(payload,ptr->d_name);
			strcat(payload,"\n");
			printf("name: %s\n",ptr->d_name);
			
		}
		
	}
	if(fn_len==1){
			if((len=(send(accept_fd,&payload,sizeof(payload),0)))<0){
				perror("can not send the file name to client\n");
			}
		}
	}
	printf("Done to send to client, len: %d\n",len);
	// free(message_to_client.payload);
}

void tranp_file_data(int accept_fd, char filename[]){
	// printf("can i reach here\n");
	char filepath[40]="data/";
		// 
	strcat(filepath,filename);
		// printf("can i reach here\n");
	FILE * file=NULL;
	// printf("can i reach here\n");
	if((file=fopen(filepath,"rb"))<0){
		perror("fail to open the file\n");
	}
	// printf("can i reach here\n");
	int filelength=0;
	char c;
	while((int)(c=fgetc(file))>=0){
		filelength+=1;
		printf("char is %c\n",c);
	}
	struct message_s header;
	int len;
	 char payload[PAYLEN];
	header.protocol[0]='m';
	header.protocol[1]='y';
	header.protocol[2]='f';
	header.protocol[3]='t';
	header.protocol[4]='p';
	header.type=0xFF;
	header.length=filelength+10;

	header.length = htonl(header.length);
	printf("file length is %d\n",filelength );
	close(file);

	if((file=fopen(filepath,"r"))<0){
		perror("fail to open the file\n");
	}
	if((len=send(accept_fd,&header,sizeof(header),0))<0){
		perror("cannot send header to client\n");
	}
	 char character[2]="";
	memset(payload,0,PAYLEN);
	int if_finish=0;

	header.length = ntohl(header.length);

	for(int i=0;i<=(header.length-10-1)/sizeof(payload);i++){
		for(int j=0;j<sizeof(payload)-1;j++){
			memset(character,0,sizeof(character));
			if((int)(character[0]=fgetc(file))>=0){
				// printf("can i reach here,%d %d %s\n",j,strlen(payload), character);
				strcat(payload,character);
				// printf("can i reach there,\n");
			}
			else{
				if((len=send(accept_fd,payload,sizeof(payload),0))<0){
					perror("can not send payload to client\n");
				}
				memset(payload,0,PAYLEN);
				if_finish=1;
				break;
			}
		}
		if(if_finish!=0){
			if((len=send(accept_fd,payload,sizeof(payload),0))<0){
					perror("can not send payload to client\n");
				}
			}
		memset(payload,0,PAYLEN);

	}
	close(file);
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
		tranp_file_data( accept_fd, payload);
	}
	printf("Done to send to client\n"); 
}
// void send_file_data(int accept_fd, struct all_data buf){

// }
void recv_file_data(int fd, char filename[], char path[]){
	int len;
	struct message_s header;
	 char payload[PAYLEN];
	if((len=recv(fd,&header,sizeof(header),0))<0){
		perror("cannot recv the header from server\n");
	}

	header.length = ntohl(header.length);

	 printf("length: %d\n",header.length);
	if(header.type!=0xFF){
		
		perror("type is wrong \n");

	}
	FILE * downfile=NULL;
	char filepath[100]="";
	strcat(filepath,path);
	strcat(filepath,filename);
	printf("cat the filename\n");
	if((downfile=fopen(filepath,"wb"))==NULL){
		printf("open file: %s\n",filepath);
	}

	for(int i=0; i<=(header.length-10-1)/sizeof(payload);i++){
		if((len=recv(fd,payload,sizeof(payload),0))<0){
			perror("can not recv the payload from server\n");
		}
		printf("receive %d %s \n",strlen(payload),payload);
		if(fwrite(payload,strlen(payload),1, downfile)<0)
		{
			perror("can not write the payload into file\n");
		}

		memset(payload,0,PAYLEN);
	}
	fflush(downfile);
	close(downfile);
	printf("Done\n");
}

void put_recv_file(int accept_fd){
	struct message_s header;
	int len;
	char payload[PAYLEN];
	char filename[PAYLEN];
	if((len=recv(accept_fd,&payload,sizeof(payload),0))<0){
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

void main_loop(unsigned short port){
	int fd, accept_fd, count, client_count;
	struct sockaddr_in addr, tmp_addr;
	unsigned int addrlen= sizeof(struct sockaddr_in);
	// struct all_data buf;
	struct message_s buf;
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
	client_count=0;
	while(1){
		int len;
		if((accept_fd=accept(fd,(struct sockaddr *) &tmp_addr, &addrlen))==-1){
			perror("accept()");
			exit(1);

		}
		client_count++;
		if ((len=(recv(accept_fd,&buf,sizeof(buf),0)))<0){
			perror("can not recv the commend");
		}
		if(buf.type==0xA1){
			list_reply(accept_fd);
		}
		// count=rev(accept_fd,&buf,sizeof(buf));
		// if(count==-1){
		// 	perror("read...");
		// 	exit(1);

		// }
		if(buf.type==0xB1){
			unsigned char payload[PAYLEN];
			// printf("get command");
			if ((len=(recv(accept_fd,&payload,sizeof(payload),0)))<0){
				perror("can not recv the commend\n");
			}
			reply_request_file( accept_fd, buf,payload);
		}
		if(buf.type==0xC1){
			put_recv_file(accept_fd);
		}
		// printf("why\n");
		close(accept_fd);
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
