/*
 * 20181666.h
 * 헤더파일
 *
 * 2020/04/04 v.01
 */

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



//history를 저장하기 위한 구조체
typedef struct HISTORY_{
	int num;
	char command_name[50];
	struct HISTORY_* link;
}HISTORY;

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


//fundamental function
int hex_to_dec(char hex[]);
int is_hex(char hex[]);
void ascii_print(int mem);

//help func
void help();

//directory func
void directory();

//history func
int history(char input[]);
void print_history(char input[]);
void free_history();

//opcode hash table
void init_hash();
int put_hash(char* code, char *instruction, char* format);
void make_hash();
int mnemonic(char str[]);
void opcodeList();
void free_hash();

//memory dump
void mem_reset();
int dump_print(char start[], char end[], int option);
int mem_edit(char address[], char value[]);
int mem_fill(char start[], char end[], char value[]);
