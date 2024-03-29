#include "log.h"

static unsigned int terminal_log; // 로그 터미널 출력 여부 설정
static FILE* logging_fd; // 로그 출력할 파일 디스크립터
static int logging_level;

static time_t time_data_t; // 현재 시간 정보를 가지고 있는 값
static struct tm tm; // 시간 정보를 분석한 값

static unsigned int Logging_terminal_set(unsigned int data);
static unsigned int Logging_terminal_out(const char *message);
static unsigned int Logging_file_out(const char *message);
static void Logging_level_set(int level);


static char *Message_Level[] = { // Message Level 별 출력할 Level 문자열 값
    "[ System ]",
    "[  Error ]"
    "[ Notice ]",
    "[  INFO  ]",
    "[ DETAIL ]"
};

/**
 * @brief Log 기록에 필요한 변수들을 초기화 하는 함수
 * 
 * @param terminal 터미널 출력 여부를 설정값, 터미널 출력시 1, 터미널 출력 미사용시 0
 * @param log_level 로깅 레벨 설정, 1 : 필수 정보 출력, 2: 세부 내용 출력, 그 외의 수 : 출력하지 않음
 */
void Logging_init(int terminal, int log_level){
    time_data_t = time(NULL);
    tm = *localtime(&time_data_t);

    Logging_terminal_set(terminal);
    Logging_level_set(log_level);
}

/**
 * @brief 인자값으로 들어온 경로로 log 파일을 설정하는 함수
 * 
 * @param filepath Log 저장 위치 값
 * @return unsigned int 에러시 1, 정상시 0 반환
 */
unsigned int Logging_file_set(const char *filepath){
    
    if(!(logging_fd = fopen(filepath, "a"))){
        return 1;
    }
    return 0;
}

void Logging_file_close(){
    fclose(logging_fd);
    return;
}

/**
 * @brief 실질적으로 Log 기록에 사용되는 함수
 * 
 * @param level Log Message Level
 * @param message 출력할 메시지 내용
 * @return unsigned int 로그 레벨에 맞지 않으면 1, 정상적으로 출력 하면 0 반환
 */
unsigned int Logging_out(int level, char* message, ...){

    va_list ap;
    char buf[900];

    char now_time[20];
    char message_out[1024] = "\0";

    if(logging_level < level){
        return 1;
    }
    if(logging_level == -1){
        return 1;
    }
    
    time_data_t = time(NULL);
    tm = *localtime(&time_data_t);
    sprintf(now_time, "%4d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    va_start(ap, message);
    vsprintf(buf, message, ap);
    va_end(ap);
    sprintf(message_out, "%s : %s : %s\n", now_time, Message_Level[level], buf);
    
    Logging_file_out(message_out);

    if(terminal_log){
        Logging_terminal_out(message_out);
    }
    return 0;
}

/**
 * @brief 터미널 출력 여부를 설정하는 함수
 * 
 * @param data 0 : 터미널 출력 안함, 그외의 수 : 터미널 출력 사용
 * @return unsigned int 터미널 출력시 1, 터미널 출력 미사용시 0
 */
static unsigned int Logging_terminal_set(unsigned int data){

    if(data){
        terminal_log = 1;
        return 1;        
    }

    terminal_log = 0;
    return 0;
}

/**
 * @brief stdout 으로 데이터를 출력하는 함수
 * 
 * @param message Error 출력 Message
 * @return unsigned int 정상시 0, 에러시 1 반환
 */
static unsigned int Logging_terminal_out(const char *message){
    
    if(!fprintf(stdout,"%s", message)){
        return 1;
    }
    fflush(stdout);

    return 0;
}

/**
 * @brief 전역에 설정된 파일 디스크립터로 데이터를 출력하는 함수
 * 
 * @param message Error 출력 Message
 * @return unsigned int 정상시 0, 에러시 1 반환
 */
static unsigned int Logging_file_out(const char *message){
    
    if(!fprintf(logging_fd,"%s", message)){
        return 1;
    }
    fflush(logging_fd);

    return 0;
}

/**
 * @brief 기록할 로그 레벨을 설정하는 함수 
 * 
 * @param level 1 : 필수 정보 출력, 2: 세부 내용 출력, 그 외의 수 : 출력하지 않음
 */
static void Logging_level_set(int level){
    
    switch (level)
    {
    case 1:
        logging_level = SYSTEM;
        break;

   case 2:
        logging_level = ERROR;
        break;

    case 3:
        logging_level = INFO;
        break;

    case 4:
        logging_level = NOTICE;
        break;

    case 5:
        logging_level = DETAIL;
        break;
    
    default:
        logging_level = -1;
        break;
    }
}