#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>

#include <arpa/inet.h>

#define SERVER_PORT 5656

static fd_set read_fds;
static int fd_max = 0;

int main(int argc, char *argv[]){

    int listenfd;
    struct sockaddr_in serv_addr;

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
    struct sockaddr_in6 client_addr6;
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
                inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
		        printf("Server : %s client connected. \n", temp);

                temp_len = read(newsockfd, temp, 512);
                temp[temp_len] = '\0';

                printf("read data : %s\n", temp);
            }
        }
    }

    return 0;
}