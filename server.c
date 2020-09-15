#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <pthread.h>

#include <arpa/inet.h>

#define SERVER_PORT 5656

static fd_set read_fds;
static int fd_max = 0;
pthread_t threads[5];

struct thread_arg
{
    int sock;
    struct sockaddr_in client_addr;
    int thread_num;
};


void *thread_work(void *arg_data){
    struct thread_arg* arg = (struct thread_arg *)arg_data ;
    int temp_len;
    char temp[514];

    inet_ntop(AF_INET, &arg->client_addr.sin_addr.s_addr, temp, sizeof(temp));
    printf("Server : %s client connected. \n", temp);

    while(1){
        temp_len = read(arg->sock, temp, 512);
        temp[temp_len] = '\0';

        if(!strcasecmp(temp, "exit")){
            close(arg->sock);
            printf("Server : %s client close. \n", temp);
            break;
        }
        printf("read data : %s\n", temp);
    }

   //  free(threads[arg->thread_num]);
   // threads[arg->thread_num] = NULL;

    pthread_exit(NULL);
}

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
                        pthread_join(threads[i], NULL);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}