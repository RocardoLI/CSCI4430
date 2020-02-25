#define PAYLEN 1024
struct message_s{
	unsigned char protocol[5];
	unsigned char type;
	unsigned int length;

}__attribute__((packed));

// struct all_data
// {	struct message_s header;
// 	// char * payload;
// 	char payload[100];
	
// }__attribute__((packed));