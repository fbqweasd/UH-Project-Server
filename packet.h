#include <stdint.h>

#define MAC_ADDR_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR_FMT_ARGS(addr) addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define BUF_LEN 1024

struct Data{
    uint8_t type; // 데이터 타입
    uint8_t len;
    char Data[];
};

/*
    0 : 기능 없음
    1 : 유튜브관련 
    2 : 에어컨 관련
    3 : 서버 관련
    4 : WOL 데이터
    5 : 컴퓨터 종료
*/

struct WOL_PACKET{
	uint8_t Magic[6];
	uint8_t MAC_ADDR[6 * 16];
};
