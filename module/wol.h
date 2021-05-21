#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

#include "log.h"
#include "packet.h"

int WOL_PACK_SEND(uint64_t mac_arg){
    struct sockaddr_in server_addr;
    struct WOL_PACKET wol_packet;

    char BroadCast_IP[] = "192.168.150.255";

    void *udp_ptr;
    int server_fd, i;
    uint64_t MAC_ADDR = mac_arg;
    int broadcastEnable=1;

    if(!mac_arg){ // 인자값으로 MAC 주소를 넘기면 할당
        Logging_out(ERROR, "WOL arg Error");
        return 0;
    }

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(BroadCast_IP);
    server_addr.sin_port = htons(7);

    // 소켓 파일디스크립터 
	if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
		perror("sock");
		exit(0);
	}
	setsockopt(server_fd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

	udp_ptr = &wol_packet;
	memset(udp_ptr, 0xFF, 6); // magic Packet Start bit
	// udp_ptr += 6;

	for(i = 0; i < 16; i++){ // MAC_ADDR
		memset(udp_ptr++, (MAC_ADDR & 0xFF0000000000) >> 40, 1);
		memset(udp_ptr++, (MAC_ADDR & 0xFF00000000) >> 32, 1);
		memset(udp_ptr++, (MAC_ADDR & 0xFF000000) >> 24, 1);
		memset(udp_ptr++, (MAC_ADDR & 0xFF0000) >> 16, 1);
		memset(udp_ptr++, (MAC_ADDR & 0xFF00) >> 8, 1);
		memset(udp_ptr++, MAC_ADDR & 0xFF, 1);
	}
    Logging_out(INFO,"WOL Pack : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.MAC_ADDR));

	if(sendto(server_fd, &wol_packet, sizeof(wol_packet), 0,(struct sockaddr *)&server_addr, sizeof(server_addr))){
	 	perror("send");
	}

	close(server_fd);
    return 0;
}
