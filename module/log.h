#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

enum log_level { 
    SYSTEM = 0,
    ERROR,
    NOTICE,
    INFO,
    DETAIL
};

enum output_level { // 출력 level 
    DAEMON = 0, // 터미널 출력 없이 Log 파일에만 출력
    TERMINAL // 터미널, Log 파일 둘 다 출력
};

void Logging_init(int terminal, int log_level);

unsigned int Logging_file_set(const char *filepath);
unsigned int Logging_out(int level, char *message, ...);
void Logging_file_close();

#endif