#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifndef __LOG_H__
#define __LOG_H__

unsigned int terminal_log; // 로그 터미널 출력 여부 설정
FILE* logging_fd; // 로그 출력할 파일 디스크립터
int logging_level;

time_t time_data_t; // 현재 시간 정보를 가지고 있는 값
struct tm tm; // 시간 정보를 분석한 값

enum log_level { 
    SYSTEM = 0,
    ERROR,
    WARNING,
    INFO,
    DEBUG
};

enum output_level { // 출력 level 
    DAEMON = 0, // 터미널 출력 없이 Log 파일에만 출력
    TERMINAL // 터미널, Log 파일 둘 다 출력
};

void Logging_init(int terminal, int log_level);

unsigned int Logging_file_set(char *filepath);
unsigned int Logging_out(enum log_level level, char *message, ...);

unsigned int Logging_terminal_set(unsigned int data);
unsigned int Logging_terminal_out(char *message);
unsigned int Logging_file_out(char *message);
void Logging_level_set(int level);

#endif