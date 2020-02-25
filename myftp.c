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
void tranp_file_data(int accept_fd, char filename[], char path[]){
	// printf("can i reach here\n");
	char filepath[100]="";
	strcat(filepath,path);
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
	c=fgetc(file);
	while(!feof(file)){
		filelength+=1;
		// printf("char is %X\n",c);
		c=fgetc(file);
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

	if((file=fopen(filepath,"rb"))<0){
		perror("fail to open the file\n");
	}
	if((len=send(accept_fd,&header,sizeof(header),0))<0){
		perror("cannot send header to client\n");
	}
	 // char character[2]="";
	memset(payload,0,PAYLEN);
	int if_finish=0;
	int file_length=0;

	header.length = ntohl(header.length);

	for(int i=0;i<=(header.length-10-1)/(sizeof(payload)-1);i++){
		for(int j=0;j<sizeof(payload)-1;j++){
			// memset(character,0,sizeof(character));
			// character[0]=fgetc(file);
			payload[j]=fgetc(file);
			printf("char: %c",payload[j]);
			if(!feof(file)){
				// printf("can i reach here,%d %d %s\n",j,strlen(payload), character);
				// printf("char is %X\n",character[0]);
				// strcat(payload,character);
				file_length+=1;
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

		for(int k=0;k<sizeof(payload)-1;k++){
			printf("i : %dth, %X\n",k, payload[k] );
		}
		// exit(1);
		if(if_finish==0){
			if((len=send(accept_fd,payload,sizeof(payload),0))<0){
					perror("can not send payload to client\n");
				}
			}

		memset(payload,0,PAYLEN);

	}
	printf("file_length: %d\n",file_length);
	close(file);

}

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
	int file_length=0;
	int i;
	for( i=0; i<=(header.length-10-1)/(sizeof(payload)-1);i++){
		printf("i: %d\n",i);
		if((len=recv(fd,payload,sizeof(payload),0))<0){
			perror("can not recv the payload from server\n");
		}
		/*
		for (int j=0; j<sizeof(payload);j++){
			// printf("payload: %di, %X\n",j,payload[j]);
		}
		*/
		printf("i: %d %d\n",i,(header.length-10-1)/(sizeof(payload)-1));
		if (i!=(header.length-10-1)/(sizeof(payload))){
			printf("not last step i: %d\n",i);
			if(fwrite(payload,sizeof(payload)-1,1, downfile)<0)
			{
				perror("can not write the payload into file\n");
			}
			file_length+=sizeof(payload)-1;
		}
		else{
			if ( (header.length-10)%(sizeof(payload)-1)!=0)
			{
				printf("last step i: %d %d\n",i, (header.length-10)%(sizeof(payload)-1));
				if(fwrite(payload,(header.length-10)%(sizeof(payload)-1),1, downfile)<0)
				{
					perror("can not write the payload into file\n");
				}
				file_length+=(header.length-10)%(sizeof(payload)-1);
			}
			else{
				printf("last step i: %d %d\n",i, (sizeof(payload)-1));
				if(fwrite(payload,(sizeof(payload)-1),1, downfile)<0)
				{
					perror("can not write the payload into file\n");
				}
				file_length+=(sizeof(payload)-1);

			}
		}
		// exit(1);
		// if(fwrite(payload,strlen(payload),1, downfile)<0)
		// {
		// 	perror("can not write the payload into file\n");
		// }
		memset(payload,0,PAYLEN);
	}
	printf("file_length: %d %d\n",file_length, header.length-10);
	fflush(downfile);
	close(downfile);
}
