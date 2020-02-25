#ifndef MYFTP_H
#define MYFTP_H
#define PAYLEN 1025
struct message_s{
	unsigned char protocol[5];
	unsigned char type;
	unsigned int length;

}__attribute__((packed));


void tranp_file_data(int accept_fd, char filename[], char path[]);
void recv_file_data(int fd, char filename[],char path[]);
#endif MYFTP_H
