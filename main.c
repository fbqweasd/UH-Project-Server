#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>

#include <json.h>

#include "wol.h"
#include "packet.h"
#include "log.h"

int SERVER_PORT = 5657;

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

int WOL_PACK_SEND(uint64_t mac_arg);

void *thread_work(void *arg_data);
void Send_TCP(struct Data* data);

void sigint_handler(){
    int i;
    Logging_out(SYSTEM, "-- signal 입력으로 종료--");

    for(i=0 ;i<5; i++){
        if(threads[i]){
            Logging_out(SYSTEM, "%d thread cancel\n", i);
    	
            pthread_cancel(threads[i]);
	        pthread_join(threads[i], NULL);
        }
    }
    close(listenfd);
    exit(0);
}

int main(){
    struct sockaddr_in serv_addr;
  
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

{   // 설정 파일에서 값 설정
    FILE* config_fp;
    char c;
    char filedata[100], logpath[100];
    int cunt = 0;
    json_object *fileobj;
    json_object *jsonval;

    if(!(config_fp = fopen("/opt/UH-Project-Server/config.json", "r"))){
		fprintf(stderr, "config file open Erorr\n");
		return -1;
	}

    do{
        c = fgetc(config_fp);

        if(c != '\n' && c != '\t' && c != 32){
            filedata[cunt] = c;
            cunt++;
        }
    }while (c != 255);
    
    filedata[cunt - 1] = '\0';
	fclose(config_fp);

    fileobj = json_tokener_parse(filedata);

    // Port 설정
    if(json_object_object_get_ex(fileobj, "ServerPort", &jsonval)){
        SERVER_PORT = json_object_get_int(jsonval);
    }
    else{
        SERVER_PORT = 5657;
    }
    
    // *** Logging  설정 부분 ***
    Logging_init(TERMINAL, 3);
	
    if(json_object_object_get_ex(fileobj, "LogPath", &jsonval)){
        strcpy(logpath, json_object_get_string(jsonval));
    }
    else{
        strcpy(logpath, "./UH-Server.log");
    }
    
    if(Logging_file_set(logpath)){
        fprintf(stderr, "Log file open Error\n");
        return 1;
    }
    // *** 설정 종료 ***

    json_object_put(fileobj);
}
    
    Logging_out(SYSTEM, "");
    Logging_out(SYSTEM, "== Server Start ==");

    // *** Server Sock init  ***
    listenfd = socket(AF_INET, SOCK_STREAM, 0); // 리스너 소켓 생성
    if(listenfd < 0){
        Logging_out(SYSTEM, "listener socket create Error");
        exit(EXIT_FAILURE);
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

    Logging_out(SYSTEM, "-- 소켓 생성 완료 --");

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
    uint8_t temp[10];
    struct Data* receive_data;
    struct sockaddr_in addr;
    char client_data[64];
    struct msghdr *message = malloc(sizeof(struct msghdr));

    inet_ntop(AF_INET, &arg->client_addr.sin_addr.s_addr, client_data, sizeof(client_data));
    Logging_out(INFO, "Server : %s client connected.", client_data);

    memset(temp, 0, sizeof(struct Data) + 6);
    memset(message, 0, sizeof(*message));
    message->msg_name = (void*)&addr;
    message->msg_namelen = sizeof(struct sockaddr_in);
    message->msg_iov = malloc(sizeof(struct iovec));
    message->msg_iov->iov_base = temp;
    message->msg_iov->iov_len = sizeof(struct Data) + sizeof(uint8_t) * 6;
    message->msg_iovlen = 1;

    recvmsg(arg->sock, message, 0);
    free(message->msg_iov);
    free(message);

    receive_data = (void*)temp;
    Logging_out(INFO, "%s : type(%d) ", client_data, receive_data->type);

    switch(receive_data->type){
        case 0 : // Sock Error
            close(arg->sock);
            Logging_out(INFO, "Server : %s client close.", client_data);
            break;
        case 1: // 유튜브
            break;
        case 4 : // WOL 패킷 
        {
            uint64_t receive_mac;
            uint8_t* mac_ptr;
            
            mac_ptr = (void*)&receive_data->Data; 
            receive_mac =  ((long long)mac_ptr[0] << 40);
            receive_mac += ((long long)mac_ptr[1] << 32);
            receive_mac += (mac_ptr[2] << 24);
            receive_mac += (mac_ptr[3] << 16);
            receive_mac += (mac_ptr[4] << 8);
            receive_mac += (mac_ptr[5]);

            printf("Debug : Len : %d \n", ntohs(receive_data->len));
            printf("Debug : Len : %d \n", receive_data->len);
            printf("Debug : MAC : %x:%x:%x:%x:%x:%x \n", receive_data->Data[0], receive_data->Data[1], receive_data->Data[2], receive_data->Data[3], receive_data->Data[4], receive_data->Data[5]);
            
            WOL_PACK_SEND(receive_mac); // 인자값은 MAC 주소의 값

            // Server -> App
            sendto(arg->sock, receive_data, sizeof(struct Data) + sizeof(uint8_t) * 6, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

            close(arg->sock);
            break;
        }
        case 5 : // 컴퓨터 종료
        {
            sendto(arg->sock, receive_data, sizeof(struct Data) + sizeof(uint8_t) * 6, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
            break;
        }


	    if(send(arg->sock, receive_data, sizeof(struct Data), 0) == -1){
            Logging_out(ERROR, "%s WOL sock Error", client_data);
	    }

        break; 
    }

    close(arg->sock);
    threads[arg->thread_num] = (pthread_t)NULL;
    pthread_exit(NULL);
}