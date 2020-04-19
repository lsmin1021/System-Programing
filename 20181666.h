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

#define CNT_COMMAND 13 //명령어 개수
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

HASHLIST* opcode_table;


//asm 코드의 각 줄의 정보를 저장하는 구조체
typedef struct LINE_{
	char comment[50]; //주석 이면 저장. 다른건 다 0 혹은 NULL로 처리
	int line; //줄 번호 
	int loc; //주소 저장
	
	char label[10];
	char str[10];
	int opCode; //str이 mnmonic이면 opcode, 아니면 -1 저장
	int format; //format이 뭔지 저장(상수인 경우 0)
	
	char operand[15];
	char operand2[10]; //operand가 2개인 경우 사용
	int indexed; //indexed이면 1, 아니면 0
	int addressing; //addressing mode저장. 

	int modified; //modify 해야하면 1 아니면 0
	//0이면 SIC, 1이면 simle, 2면 immediate, 3이면 inderect
	char obj[9];	
	struct LINE_* link; //다음 줄을 가리키는 link
}LINE;
//asm 파일의 정보를 저장하는 line의 헤드역할
typedef struct ASM_{
	char name[20]; //프로그램 이름
	int start; //시작 메모리 주소
	int end; //끝 메모리 주소 (길이 구하기 위해)
	int modify_cnt;
	LINE* link;
}ASM;
ASM* asm_head;
int base, pc; //base와 pc를 저장하기 위한 변수

int* modify; //modification을 해야하는 location 저장하는 배열

typedef struct SYMBOL_{
	char symbol[10];
	int loc; //location
	struct SYMBOL_* link;
}SYMBOL;
SYMBOL* symbol_head;
SYMBOL* save_symtab;
int symbol_flag;


//fundamental function
int hex_to_dec(char hex[]);
int is_hex(char hex[]);
void ascii_print(int mem);

//help func
void help();

//directory func
void directory();
int type_func(char* filename);

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

//assembler

int is_directive(char str[]);
int is_mnemonic(char str[]);
int is_valid_inst(char str[]);
int find_format(char *str);
int put_symbol(char label[], int loc);
int manage_line(char* line, int lineCnt, int *loc);
int read_file(char *filename);

int reg_num(char* reg);
int find_sym_loc(char* symbol);

char* line_objectcode(LINE* node);
int make_objectcode();

void makefile_lst(char* filename);
void makefile_obj(char* filename);

void make_lst();

void free_asm();

//test function
void free_symbol(SYMBOL* node);
void save_symbol();
void print_symbol();
