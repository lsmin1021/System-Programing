#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


#define LEN_COMMAND 30 //명령어 길이
#define KEY 19 //hash 생성 시 사용할 KEY 값
#define MAX_MEMORY 1048576 //메모리 크기
#define ASCII_FIRST "20" //아스키값 출력 시 출력되는 최소 값
#define ASCII_LAST "7E" //아스키갑 출력시 출력되는 최대값


int memory[16][65536]; //가상의 메모리 공간
int mem_addr = 0; //dump 명령어 입력 시 출력될 address

char command[10][LEN_COMMAND]={"h[elp]","d[ir]","q[uit]","hi[story]",
	"du[mp] [start, end]","e[dit] address, value", "f[ill] start, end, value",
	"reset", "opcode mnemonic", "opcodelist"}; //명령어 목록

//history를 저장하기 위한 구조체
typedef struct HISTORY_{
	int num;
	char command_name[LEN_COMMAND];
	struct HISTORY_* link;
}HISTORY;

HISTORY* h_head = NULL;


int history_num = 0; //현재 히스토리에 저장된 개수


//opcode hash table 만들기 위한 구조체
typedef struct HASH_{
	int opCode;
	char instruction[8];
	char format[4];
	struct HASH_* link;
}HASH;

typedef struct HASHLIST_{
	int hash_num;
	HASH* link;
}HASHLIST;

HASHLIST* opcode_table = NULL;

