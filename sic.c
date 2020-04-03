#include "header.h"

#define LEN_COMMAND 30 //명령어 최대 길이
#define KEY 19 //hash table을 만들기 위한 key


int memory[16][65536]; //가상의 메모리 공간
int mem_addr = 0;


char command[10][LEN_COMMAND]={"h[elp]","d[ir]","q[uit]","hi[story]",
	"du[mp] [start, end]","e[dit] address, value", "f[ill] start, end, value",
	"reset", "opcode mnemonic", "opcodelist"}; //명령어 목록

typedef struct HISTORY_{
	int num;
	char command_name[LEN_COMMAND];
	struct HISTORY_* link;
}HISTORY;

HISTORY* h_head = NULL;
int history_num = 0; //현재 히스토리에 저장된 개수

//opcode hash table 만들기 위한 변수
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


void help(){ //명령어 목록 출력
	for(int i=0;i<10;i++){
		printf("%s\n",command[i]);
	}
	
}

int history(char input[]){ //입력한 명령어 기록 
	HISTORY* new_node;

	if(h_head == NULL){
		new_node = (HISTORY*)malloc(sizeof(HISTORY)); //첫 기록 초기화
		new_node->num = ++history_num;
		strcpy(new_node->command_name, input);
		new_node->link = NULL;
		h_head = new_node;
	}
	else{
		HISTORY* temp_node = h_head;
		while(temp_node->link != NULL){
			temp_node = temp_node->link;
		}
		new_node = (HISTORY*)malloc(sizeof(HISTORY)); //새 기록 초기화
		new_node->num = ++history_num;
		strcpy(new_node->command_name, input);
		new_node->link = NULL;
		temp_node->link = new_node;
	}

	return 1;
}

void print_history(char input[]){ //명령어 기록 출력
	HISTORY* temp_node = h_head;
	history(input); //history도 명령어 내역에 추가
	while(temp_node!=NULL){
		printf("%-5d %s\n", temp_node->num, temp_node->command_name); //명령어 내역 출력
		temp_node = temp_node->link;
	}
	return;
}

void init_hash(){ //해시 테이블 초기화해주는 함수
	opcode_table = (HASHLIST*)malloc(sizeof(HASHLIST)*20);
	for(int i=0;i<20;i++){
		opcode_table[i].hash_num = i;
		opcode_table[i].link = NULL;
	}
}
int hex_to_dec(char hex[]){ //16진수를 10진수로 바꿔주는 함수
	int num = 0; //10진수를 저장할 변수
	int mul = 1;
	for(int i = strlen(hex)-1;i >= 0;i--){
		char ch = hex[i];
		int temp;
		if('0' <= ch && ch <= '9'){
			temp = ch-'0';
		}
		else if('A' <= ch && ch <= 'F'){
			temp = ch - ('A'-10);
		}
		else 
			temp = ch - ('a' -10);
		num += temp * mul;
		mul *= 16;
	}
	return num; //십진수로 변환한 값 반환
}
int put_hash(char code[2], char instruction[7], char format[4]){ //해시 테이블에 넣기
	int code_num=0;
	code_num = hex_to_dec(code);
	
	//int key_num = code_num%KEY; //opCode의 십진수 값을 KEY 값으로 나눈 값 : HASH
	
	// key_num 설정 하기
	/* 기준 다시 정하자아아ㅏㅇ아아아*/	
	int key_num, len_inst;
	len_inst = strlen(instruction);
	if(len_inst >= 3 )
		key_num = (instruction[len_inst-1]+instruction[len_inst-2])%19;
	else if(len_inst == 2)
		key_num = (instruction[len_inst-1] + instruction[len_inst-2])%21;
	else
		key_num = instruction[len_inst-1]%19;
	if(key_num >= 20)
		key_num -= 20;
	//새로운 노드 생성
	HASH* new_node = (HASH*)malloc(sizeof(HASH));
	new_node->opCode = code_num;
	strcpy(new_node->instruction, instruction);
	strcpy(new_node->format,format);
	
	//해시 값에 해당하는 링크에 넣기
	HASH* temp = opcode_table[key_num].link;
	if(temp == NULL) opcode_table[key_num].link = new_node;
	else{
		while(temp->link != NULL){
			temp = temp->link;
		}
		temp->link = new_node;
	}
//	printf("%d !!%s!!\n",key_num,new_node->instruction);
	return 1; //제대로 수행되었으면 1 리턴... 제대로 안됐을때는 아직 구현 x


}
void make_hash(){
	init_hash();
	FILE* fp = fopen("opcode.txt","r");
	char line[30];
	char* temp = fgets(line,30,fp);
	char code[2];
	char instruction[7]; 	
	char format[4];
	while(temp != NULL){ //파일로부터 한 줄씩 읽어보기
		line[strlen(line)-1]=0;
		code[0] = line[0];
		code[1] = line[1];

		int flag = 0, i;
		int index = 0;

		for(i=2;;i++){
			if(line[i] <'A' || line[i]>'Z' ) {
				if(flag==0) continue;
				else break;
			}
			flag=1;
			instruction[index++] = line[i];			
		}
		flag = 0;
		index = 0;
		for(;;i++){
			if((line[i] <'1' || line[i] >'4') && line[i]!='/') {
				if (flag == 0)continue;
				else break;
			}
			flag = 1;
			format[index++] = line[i];
		}
		put_hash(code, instruction, format);
		temp = fgets(line,30,fp);
		

		//변수 초기화
		code[0] = 0; code[1] = 0;
		for(int i=0;i<7;i++)
			instruction[i] = 0;
		for(int i=0;i<4;i++)
			format[i] = 0;
	}


	
}
void mnemonic(char str[]){ //instruction에 해당하는 opCode 출력
	for(int i=0;i<20;i++){
		HASH* temp = opcode_table[i].link;
		int flag = 0;
		while(temp!= NULL){
			if(strcmp(temp->instruction, str)==0)
				printf("opcode is %02X\n",temp->opCode);
			temp = temp->link;
		}
	}
}
void opcodeList(){ //opcode Hash Table의 내용을 출력해줌
	for(int i=0;i<20;i++){
		HASH* temp = opcode_table[i].link;
		printf("%d : ",i);
		int flag = 0;
		while(temp!= NULL){
			if(flag != 0) printf(" -> ");
			printf("[%s,%02X]",temp->instruction,temp->opCode);
			flag = 1;
			temp = temp->link;
		}
		printf("\n");
	}
}

void directory(){ //현재 디렉터리에 있는 파일들을 출력한다
	DIR* dp = NULL;
	struct dirent *ent;
	struct stat buf;
	dp = opendir("./");
	int printcnt = 0; // 적절히 엔터를 넣어주기 위한 변수
	if(dp!= NULL){
		printf("\t");
		while((ent = readdir(dp)) != NULL){
			lstat(ent->d_name, &buf);
			if(S_ISDIR(buf.st_mode)){
				printf("%s/\t",ent->d_name);
			}
			else if(S_ISREG(buf.st_mode)){
				if(buf.st_mode & 01001001)
					printf("%10s*\t",ent->d_name);
				else
					printf("%10s\t",ent->d_name);
			}
			printcnt++;
			if(printcnt==3){
				printf("\n\t");
				printcnt=0;
			}
		}
		printf("\n");
		closedir(dp);
	}
	else{
		perror("");
//		return EXIT_FAILUARE;
	}


}
void mem_reset(){ //메모리를 리셋해주는 함수
	// memory 리셋
	for(int i=0;i<16;i++){
		for(int j=0;j<65536;j++){
			memory[i][j] = 0;
		}
	}
}
void ascii_print(int mem){ //memory에 값을 ascii 값으로 출력해주는 함수	
	if(hex_to_dec("20") <= mem && mem <= hex_to_dec("7E"))
		printf("%c",mem);
	else
		printf(".");
}
void dump_print(char start[], char end[], int option){
	int i_start; //시작 인덱스
	int i_end;
	printf("ss %s, ee %s\n",start, end);
//	if( strcmp(end, "-2") == 0){ //dump 
	if(option == 1){
		i_start = mem_addr;
		i_end = mem_addr + 160 - 1;

		mem_addr = mem_addr + 160;
		if( mem_addr >= 16*65536) //끝까지 간 경우 address를 다시 0으로 초기화
			mem_addr = 0;
	}
//	else if(strcmp(end,"-1") == 0){ //dump start
	else if(option == 2){
		i_start = hex_to_dec(start);
		i_end = i_start+160-1;
		if(i_end >= 16*65536)
			i_end = 16*65536-1;
	}
	else{ //dump start end
		i_start = hex_to_dec(start);
		i_end = hex_to_dec(end);
	}
	//에러 처리
	if(i_start >= 16*65536 || i_end >= 16*65536){ //메모리 범위 벗어난 경우 에러메세지
		printf("error! index must be less than memory size\n");
		return ;
	}
	else if(i_start > i_end){
		printf("error! start must be less than end\n");
		return;
	}

	int	first = i_start %16; //출력할 때 앞에 비워지는 칸 수
	int last = i_end % 16; //마지막 줄에 출력되는 수
	int cnt = i_end-i_start + 1, index = i_start;
	int flag = 0;
	int line_s, line_e; //해당 줄에 시작 인덱스와 끝 인덱스
	int line_ss;
	printf("%05X ",(i_start/16)*16);
	flag = 1;
	line_s = i_start;
	int i;
	for(i=0;i<first;i++) //앞 부분 공백 출력
		printf("   ");
	for (i=i_start; i<=i_end;i++){
		if(flag == 0){
			printf("%05X ",(i/16)*16);
			line_s = i;
			flag = 1;
		}
		printf("%02X ", memory[i%16][i/16]);
		line_e = i;
		if(i % 16 == 15){
			printf("; ");
			for(int j = (i/16)*16;j<line_s;j++){
				ascii_print(0);
			}
			for(int j=line_s; j<= line_e; j++){ //해당 줄의 시작 인덱스부터 끝까지 출력
				ascii_print(memory[j%16][j/16]);
			}
			for(int j=line_e+1;j< (i/16)*16+15;j++)
				ascii_print(0);
			flag = 0;
			printf("\n");
		}
	}
	i--; //맨 마지막으로 출력된 값의 인덱스 저장
	for(int k=0;k<16-last-1;k++) //뒷 부분 공백 출력
		printf("   ");
	if(i%16!=15){ //줄 중간에서 끝난 경우 아스키값 출력
		printf("; ");
		for(int j=line_s; j<= line_e; j++){ //해당 줄의 시작 인덱스부터 끝까지 출력
			ascii_print(memory[j%16][j/16]);
		}
		for(int j=line_e+1;j<=((i-1)/16)*16+15;j++)
			ascii_print(0);
		printf("\n");
	}

}
void mem_edit(char address[], char value[]){
	int addr = hex_to_dec(address); //10진수로 바꿔줌
	int val = hex_to_dec(value);
	if(addr >= 16 * 65536 || addr < 0){
		printf("error! address error");
		return;
	}
	if(val > hex_to_dec("FF")){
		printf("error! value error");
		return;
	}

	memory[addr%16][addr/16] = val;

}
void mem_fill(char start[], char end[], char value[]){
	int i_start = hex_to_dec(start);
	int i_end = hex_to_dec(end);
	int val = hex_to_dec(value);
	
	if(i_start >= 16*65536 || i_end >= 16*65536){ //메모리 범위 벗어난 경우 에러메세지
		printf("error! index must be less than memory size\n");
		return ;
	}
	else if(i_start > i_end){
		printf("error! start must be less than end\n");
	printf("/");
		return;
	}
	if(val > hex_to_dec("FF")){
		printf("error! value error");
		return;
	}
	
	for (int i=i_start; i<=i_end;i++){
		memory[i%16][i/16] = val;
	}
}

int main(void){
	char input[50]; //사용자로 부터의 입력
	mem_reset();	
	printf("durl\n");
	make_hash();
	while(1){
		printf("sicsim> "); //입력 프롬프트 상태
		fgets(input,50,stdin); //입력
		input[strlen(input)-1]=0; //입력의 마지막 엔터 값을 null 값으로 변경
		

		if(strcmp(input,"q") * strcmp(input,"quit") == 0) break; //quit 혹은 q 입력 시 종료
		
		//history 혹은 h 입력 시 히스토리 출력
		if(strcmp(input,"h") * strcmp(input,"history")==0) {
			print_history(input);
			continue;
		}
		else history(input);
		
		if(strcmp(input, "opcodelist")==0) {
			opcodeList(); 
			continue;
		}
		if(strcmp(input, "d") * strcmp(input, "dir") == 0) {
			directory(); // dir 명령어
			continue;
		}
		if(strcmp(input, "reset") == 0){
			mem_reset();
			continue;
		}
		char *inArr[10] = {NULL, };
		int index = 0;

		char *ptr = strtok(input," ");
		while(ptr != NULL){
			inArr[index] = ptr;
			index++;
			ptr = strtok(NULL, " ");
		}
		for(int i=0;i<10;i++){
			if(inArr[i] != NULL)
				printf("%s ",inArr[i]);
		}
		//opcodelist 입력 시 list 출력
		if(inArr[0][0] == 'd' && inArr[0][1] == 'u'){
			if(index == 3)
				(inArr[1])[strlen(inArr[1])-1] = '\0';
			dump_print(inArr[1],inArr[2], index);	
		}
		if(inArr[0][0] == 'f'){
			
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			(inArr[2])[strlen(inArr[2])-1] = '\0';
			mem_fill(inArr[1], inArr[2], inArr[3]);
		}
		if(inArr[0][0] == 'e'){
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			mem_edit(inArr[1], inArr[2]);
		}



		/* TODO 명령어 처리, 에러처리
		 * dump, edit, fill, reset
		 * opcode mnemonic
		 * dir 형식 정리
		 * 변수 이름 바꾸기
		 *
		 * */


	}

//	make_hash();
//	opcodeList();
//	mnemonic("LDB");
//	memory[0][0] = 255;
//	dump_print("4", "37");
//	help();
//	directory();	
	return 0;
}
