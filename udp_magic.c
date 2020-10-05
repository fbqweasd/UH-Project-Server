#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>

#include "Data.h"

#define BUF_LEN 514
#define SERVER_PORT 5656

typedef struct WOL_PACKET{
	uint8_t Magic[6];
	uint8_t MAC_ADDR[6 * 16];
}

int main(int argc, char *argv[]){
    char buffer[BUF_LEN + 1];
    struct sockaddr_in server_addr;
	struct WOL_PACKET wol_packet;
	int server_fd, n;
	int i, j;
	uint8_t *MAC_ADDR;

	int len, msg_size;
	void *udp_ptr;

    memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

    // 입력 인자 값으로 서버 설정
	if(argc <= 1){
		printf("---- 기본 서버 주소로 설정 됨 ----\n");
		server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(SERVER_PORT);
	}
	else if(argc == 2){
		printf("---- 서버 주소 %s 설정 ----\n", argv[1]);
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = htons(SERVER_PORT);

	}
	else if(argc == 3){
		printf("---- 서버 주소 %s : %s 설정 ----\n", argv[1],argv[2]);
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = htons(atoi(argv[2]));
	}	
	else{
		printf("인자 값 오류 %s \n",argv[0]);
	}

    // 소켓 파일디스크립터 
	if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("sock");
		exit(0);
	}

	strcpy(MAC_ADDR, "FFFFFFFF", 6);	

	udp_ptr = &wol_packet
	memset(udp_ptr, 0xFF, 8 * 6 ); // magic Packet Start bit
	udp_ptr += 8 * 6;

	for(i = 0; i < 16; i++){
		for(j = 0; j < 6; j++){
			memset(udp_ptr, htonl(MAC_ADDR[j]), 8 * 2 ); // magic Packet Start bit
			udp_ptr += 8 * 2;
		}
	}
 
	if(sendto(server_fd, ,, (struct sockaddr *)&server_addr, sizeof(server_addr)){
		perror("send");
		exit(0);
	}

	close(server_fd);
    return 0;
}
