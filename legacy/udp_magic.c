#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>

#include "Data.h"

#define SERVER_PORT 7

#define MAC_ADDR_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR_FMT_ARGS(addr) addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

int main(int argc, char *argv[]){
    char buffer[BUF_LEN + 1];
    struct sockaddr_in server_addr;
	struct WOL_PACKET wol_packet;
	int server_fd, n;
	int i, j;
	uint64_t MAC_ADDR = 0x00D861C36D40;

	int len, msg_size;
	void *udp_ptr;

    memset(&server_addr, 0x00, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

    // 입력 인자 값으로 서버 설정
	if(argc <= 1){
		printf("---- 기본 서버 주소로 설정 됨 ----\n");
		server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
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

	int broadcastEnable=1;
	int ret=setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	udp_ptr = &wol_packet;
	memset(udp_ptr, 0xFF, 6); // magic Packet Start bit
	udp_ptr += 6;

	printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.Magic));
	printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.MAC_ADDR));
	//printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(MAC_ADDR));

	for(i = 0; i < 16; i++){
		// MAC_ADDR
		memset(udp_ptr++, (MAC_ADDR & 0xFF0000000000) >> 40, 1);
		printf("%x\t", (MAC_ADDR & 0xFF0000000000) >> 40);

		memset(udp_ptr++, (MAC_ADDR & 0xFF00000000) >> 32, 1);
		printf("%x\t", (MAC_ADDR & 0xFF00000000) >> 32);

		memset(udp_ptr++, (MAC_ADDR & 0xFF000000) >> 24, 1);
		printf("%x\t", (MAC_ADDR & 0xFF000000) >> 24);

		memset(udp_ptr++, (MAC_ADDR & 0xFF0000) >> 16, 1);
		printf("%x\t", (MAC_ADDR & 0xFF0000) >> 16);

		memset(udp_ptr++, (MAC_ADDR & 0xFF00) >> 8, 1);
		printf("%x\t", (MAC_ADDR & 0xFF00) >> 8);

		memset(udp_ptr++, MAC_ADDR & 0xFF, 1);
		printf("%x\t", MAC_ADDR & 0xFF); 
		printf("\n");
	}
	printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.MAC_ADDR));

	if(sendto(server_fd, &wol_packet, sizeof(wol_packet), 0,(struct sockaddr *)&server_addr, sizeof(server_addr))){
	 	perror("send");
	 	exit(0);
	}

	close(server_fd);
    return 0;
}
