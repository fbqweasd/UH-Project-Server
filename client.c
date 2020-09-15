#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>

#define BUF_LEN 514
#define SERVER_PORT 5656

int main(int argc, char *argv[]){
    char buffer[BUF_LEN + 1];
    struct sockaddr_in server_addr;
	int server_fd, n;

	char input_data[BUF_LEN];
	int len, msg_size;

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
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("sock");
		exit(0);
	}
	
	if(connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		perror("connect");
		exit(0); 
	}
    
    fgets(input_data, BUF_LEN, stdin);
    input_data[strlen(input_data)- 1] = '\0';
        
    if(ferror(stdin)){
        perror("stdio");
    }

    if(write(server_fd,input_data,strlen(input_data)) < 0){
        perror("write");
        exit(0);
    }
    return 0;
}