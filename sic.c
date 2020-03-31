#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LEN_COMMAND 30 //명령어 최대 길이


char command[10][LEN_COMMAND]={"h[elp]","d[ir]","q[uit]","hi[story]",
	"du[mp] [start, end]","e[dit] address, value", "f[ill] start, end, value",
	"reset", "opcode mnemonic", "opcodelist"}; //명령어 목록

typedef struct HISTORY_{
	int num;
	char command_name[LEN_COMMAND];
	struct HISTORY_* link;
}HISTORY;

HISTORY* h_head = null;
int history_num = 0; //현재 히스토리에 저장된 개수

void help(){ //명령어 목록 출력
	for(int i=0;i<10;i++){
		printf("%s\n",command[i]);
	}
	
}

int history(){ //입력한 명령어 기록 
	if(h_head == null){
		
	}



}

void print_history(char input[]){ //명령어 기록 출력
	



}

int main(void){
	char input[50]; //사용자로 부터의 입력

	while(1){
		printf("sicsim> "); //입력 프롬프트 상태
		fgets(input,50,stdin); //입력
		input[strlen(input)-1]=0; //입력의 마지막 엔터 값을 null 값으로 변경


		if(strcmp(input,"q") * strcmp(input,"quit") == 0) break; //quit 혹은 q 입력 시 종료
		
		history(input);

//		printf("\n%s\n",input);

	}


	help();

	return 0;
}
