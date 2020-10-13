#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

#include "Data.h"

#define SERVER_PORT 5657

static fd_set read_fds;
static int fd_max = 0;
static pthread_t threads[5];
int listenfd;

struct thread_arg
{
    int sock;
    struct sockaddr_in client_addr;
    int thread_num;
};

int WOL_PACK_SEND();

void *thread_work(void *arg_data);

void sigint_handler(){
    
    int i;
    fprintf(stdout, "-- SIGINT 종료 --\n");

    for(i=0 ;i<5; i++){
        if(threads[i]){
            fprintf(stdout, "%d thread cancel\n",i);
            pthread_cancel(threads[i]);
        }
    }

    close(listenfd);
    exit(0);
}

int main(int argc, char *argv[]){

    struct sockaddr_in serv_addr;

    signal(SIGINT, sigint_handler);

    listenfd = socket(AF_INET, SOCK_STREAM, 0); // 리스너 소켓 생성
    if(listenfd < 0){
        fprintf(stderr, "listener socket create Error");
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) < 0) {
        perror("Error on binding");
        exit(1);
    }

    if (listen(listenfd, 5)) { // 리스너 대기열 생성 
        perror("Error on listen");
        exit(1);
    }

    FD_ZERO(&read_fds); // read_fds 값을 초기화
    FD_SET(listenfd, &read_fds); // fds 값(파일 디스크립터의 집합)에 리스터 소켓 추가
    fd_max = listenfd;

    int newsockfd;
    struct sockaddr_in client_addr;
    fd_set tmp_fds;
    FD_ZERO(&tmp_fds);
    char temp[512];
    int temp_len;

    fprintf(stdout, "-- 소켓 생성 완료 --\n");

    while(1){
        tmp_fds = read_fds;
        if (select(fd_max + 1, &tmp_fds, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            close(listenfd);
            exit(1);
        }

        if(FD_ISSET(listenfd, &tmp_fds)) { // select 된 파일 디스크립터 get 
            uint32_t client_len = sizeof(client_addr);

            if ((newsockfd = accept(listenfd, (struct sockaddr *)&client_addr,&client_len)) < 0) {
                perror("Error in accept");
            }
            else{
                // thread work
                struct thread_arg *arg;
                arg = (struct thread_arg *)malloc(sizeof(struct thread_arg));
                arg->client_addr = client_addr;
                arg->sock = newsockfd;

                for(int i = 0; i < 5; i++){
                    arg->thread_num = i;
                    if(!threads[i]){
                        pthread_create(&threads[i], NULL, &thread_work, (void *)arg);
                        //pthread_join(threads[i], NULL);
                         pthread_detach(threads[i]);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}


void *thread_work(void *arg_data){
    struct thread_arg* arg = (struct thread_arg *)arg_data ;
    int temp_len;
    uint8_t temp[514];
    char clinet_data[514];
    struct Data* receive_data;

    inet_ntop(AF_INET, &arg->client_addr.sin_addr.s_addr, clinet_data, sizeof(clinet_data));
    printf("Server : %s client connected. \n", clinet_data);

    while(1){
        //temp_len = read(arg->sock, temp, 512);
        memset(temp, 0, sizeof(struct Data));
        temp_len = recv(arg->sock, temp, sizeof(struct Data), 0);
        receive_data = (struct Data*)temp;

        if(!strcasecmp(temp, "exit")){
            close(arg->sock);
            printf("Server : %s client close. \n", clinet_data);
            break;
        }
        printf("%s  %d : %s\n",clinet_data, receive_data->type, receive_data->data);

        switch(receive_data->type){
            case 1: // 유튜브
                break;
            case 4 : // WOL 패킷 
                WOL_PACK_SEND();
                break; 
        }
    }

   //free(threads[arg->thread_num]);
   //threads[arg->thread_num] = NULL;

    pthread_exit(NULL);
}

int WOL_PACK_SEND(){
    char buffer[BUF_LEN + 1];
    struct sockaddr_in server_addr;
	struct WOL_PACKET wol_packet;
	int server_fd;
	int i, j;

    char COMPUTER_IP[] = "192.168.150.255";
    //uint64_t MAC_ADDR = 0xF2FD21012211;
    uint64_t MAC_ADDR = 0x00D861C36D40;

	int len, msg_size;
	void *udp_ptr;

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = inet_addr(COMPUTER_IP);
    server_addr.sin_port = htons(7);

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

	// printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.Magic));
	// printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(wol_packet.MAC_ADDR));
	// printf("Debug : " MAC_ADDR_FMT "\n", MAC_ADDR_FMT_ARGS(MAC_ADDR));

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
	 	//exit(0);
	}

	close(server_fd);
    return 0;
}
