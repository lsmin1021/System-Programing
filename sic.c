#include "header.h"

#define LEN_COMMAND 30 //명령어 최대 길이
#define KEY 19 //hash table을 만들기 위한 key
#define MAX_MEMORY  1048576
#define ASCII_FIRST "20"
#define ASCII_LAST "7E"

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

HASHLIST* opcode_table = NULL;


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
		
		return 1;
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
		
		return 1;
	}

	return 0; //혹시 몰라서 예외 처리
}

void print_history(char input[]){ //명령어 기록 출력
	history(input); //history도 명령어 내역에 추가

	HISTORY* temp_node = h_head;
	while(temp_node!=NULL){
		printf("%-5d %s\n", temp_node->num, temp_node->command_name); //명령어 내역 출력
		temp_node = temp_node->link;
	}
	return;
}

/**
 * 16진수 변환 관련 함수들
 */
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
int is_hex(char hex[]){ //해당 문자열이 16진수가 맞는지 판단하는 함수
	for(int i=0;i<strlen(hex);i++){
		char ch = hex[i];
		if('0'<= ch && ch <= '9')
			continue;
		else if('A' <= ch && ch <= 'F')
			continue;
		else if('a'<= ch && ch <= 'f')
			continue;
		else
			return 0; //하나라도 범위에 해당하지 않는 문자가 있으면 0 리턴
	}

	return 1; //16진수가 맞으면 1 리턴

}

/*
 * hash table을 만들기 위한 함수들
 *
 * init_hash, put_hash, make_hash
 * */
void init_hash(){ //해시 테이블 초기화해주는 함수
	opcode_table = (HASHLIST*)malloc(sizeof(HASHLIST)*20);
	for(int i=0;i<20;i++){
		opcode_table[i].hash_num = i;
		opcode_table[i].link = NULL;
	}
}
int put_hash(char* code, char* instruction, char* format){ //해시 테이블에 넣기
	int code_num=0;
	code_num = hex_to_dec(code);
	
	// key_num 설정 하기
	/* 기준 다시 정하자아아ㅏㅇ아아아 TODO*/	
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
	return 1; //제대로 수행되었으면 1 리턴... 제대로 안됐을때는 아직 구현 x
	//TODO

}
/* *
 * opcode.txt로부터 정보를 읽어와서
 * 각 instruction에 대한 opcode를 
 * hash table 형태로 저장
 * */
void make_hash(){
	
	init_hash(); //맨 처음 hash table 초기화
	FILE* fp = fopen("opcode.txt","r"); //파일 열기
	char line[31];
	//맨 첫줄 읽어오기
	//char* temp = fgets(line,30,fp);
	// 각 줄마다 정보를 저장해줄 변수들
	char code[3];
	char instruction[7]; 	
	char format[4];
	//printf("%ste\n",temp);
//	printf("....");
	/*
	while(temp != NULL){ //파일로부터 한 줄씩 읽어보기
		line[strlen(line)-1]=0;
		//opCode 저장
		code[0] = line[0]; 
		code[1] = line[1];
	//	printf(">>%s",code);	
		int flag = 0, i;
		int index = 0;
		
		// instruction 저장
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
		//format 형태 저장
		for(;;i++){
			if((line[i] <'1' || line[i] >'4') && line[i]!='/') {
				if (flag == 0)continue;
				else break;
			}
			flag = 1; //숫자가 들어오면 flag를 세워서 거기부터 저장
			format[index++] = line[i];
		}
		//읽은 정보를 hash table에 넣는다
		put_hash(code, instruction, format);
		//다음 줄 읽어오기
		temp = fgets(line,30,fp);
		

		//변수 초기화
		code[0] = 0;
		code[1] = 0;
		for(int i=0;i<7;i++)
			instruction[i] = 0;
		for(int i=0;i<4;i++)
			format[i] = 0;
	}*/
	
	int flag1=0, flag2=0, flag3=0;
	while(fscanf(fp,"%s",line) != EOF){
		if(flag1 == 0){
			strcpy(code, line);
			flag1 = 1;
		}
		else if(flag2 == 0){
			strcpy(instruction, line);
			flag2 = 1;
		}
		else{
			strcpy(format, line);
			put_hash(code,instruction,format);
			flag1 = 0; flag2=0;
		}
	}
	
	
	fclose(fp);

}
/*
 * opcode 관련 함수들
 *
 * mnemonic, opcodeList
 * */
int mnemonic(char str[]){ //instruction에 해당하는 opCode 출력
	int flag = -1; // opCode를 찾았으면 1로 바꾸는 flag
	for(int i=0;i<20;i++){
		HASH* temp = opcode_table[i].link;
		while(temp!= NULL){
			if(strcmp(temp->instruction, str)==0){
				printf("opcode is %02X\n",temp->opCode);
				flag = 1; break;
			}
			temp = temp->link;
		}
		if(flag == 1) break;
	}
	if(flag == -1)
		printf("there is no %s in opcodeList\n",str); //Hash 테이블에 없는 경우 에러 처리
	return flag; //정상적으로 찾은 경우 1 리턴, 아닌 경우 0 리턴
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

/*
 * 디렉토리 정보를 출력하기 위한 함수
 * directory()
 * */
void directory(){ //현재 디렉토리에 있는 파일들을 출력한다
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
	//printf("메모리 리셋 완료!\n");
}
void ascii_print(int mem){ //memory에 값을 ascii 값으로 출력해주는 함수	
	if(hex_to_dec(ASCII_FIRST) <= mem && mem <= hex_to_dec(ASCII_LAST))
		printf("%c",mem);
	else
		printf(".");
}
int dump_print(char start[], char end[], int option){
	int i_start; //시작 인덱스
	int i_end;

	//dump에 입력한 인자 수에 따른 옵션 분리
	if(option == 1){ //dump만 입력한 경우
		i_start = mem_addr;
		i_end = mem_addr + 160 - 1;

		mem_addr = mem_addr + 160;
		if( mem_addr >= MAX_MEMORY) //끝까지 간 경우 address를 다시 0으로 초기화
			mem_addr = 0;
	}
	else if(option == 2){ //dump start
		if(is_hex(start) == 0) //start 값이 16진수가 아니면 에러. 0 리턴
			return 0;
		i_start = hex_to_dec(start);
		i_end = i_start+160-1;
		if(i_end >= MAX_MEMORY)
			i_end = MAX_MEMORY-1;
	}
	else{ //dump start end
		if(is_hex(start) == 0 || is_hex(end) == 0) //start와 end 확인해서 16진수 아니면 0리턴
			return 0;
		i_start = hex_to_dec(start);
		i_end = hex_to_dec(end);
	}

	//에러 처리
	if(i_start >= MAX_MEMORY || i_end >= MAX_MEMORY){ //메모리 범위 벗어난 경우 에러메세지
		printf("error! index must be less than memory size\n");
		return -1;
	}
	else if(i_start > i_end){
		printf("error! start must be less than end\n");
		return -1;
	}

	int	first = i_start %16; //출력할 때 앞에 비워지는 칸 수
	int last = i_end % 16; //마지막 줄에 출력되는 수
//	int cnt = i_end-i_start + 1, index = i_start;
	int flag = 0;
	int line_s, line_e; //해당 줄에 시작 인덱스와 끝 인덱스
	int line_ss;

	// 맨 처음 주소 출력
	printf("%05X ",(i_start/16)*16);
	flag = 1;
	line_s = i_start;
	int i;

	for(i=0;i<first;i++) //앞 부분 공백 출력
		printf("   ");

	for (i=i_start; i<=i_end;i++){ //데이터 출력
		if(flag == 0){
			printf("%05X ",(i/16)*16); //주소 출력
			line_s = i;
			flag = 1;
		}
		printf("%02X ", memory[i%16][i/16]); //메모리 값 출력
		line_e = i;
		if(i % 16 == 15){ //줄의 맨 끝에 도착했을 때
			printf("; "); //구분자 출력
			for(int j = (i/16)*16;j<line_s;j++){ //범위에서 벗어난 부분 . 출력
				ascii_print(0);
			}
			for(int j=line_s; j<= line_e; j++){ //해당 줄의 시작 인덱스부터 끝까지 출력
				ascii_print(memory[j%16][j/16]);
			}
			for(int j=line_e+1;j< (i/16)*16+15;j++) //범위에서 벗어난 부분 . 출력
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
		for(int j=line_e+1 ;j<=(i/16)*16+15; j++) //끝에 남은 부분 . 출력
			ascii_print(0);
		printf("\n");
	}

	return 1;
}

/*
 * edit하는 함수. 주소에 해당하는 메모리의 값을 value로 바꿈
 * */
int mem_edit(char address[], char value[]){
	if(is_hex(address) * is_hex(value) == 0) //16진수가 아닌 경우 0 리턴
		return 0;
	
	int addr = hex_to_dec(address); //10진수로 바꿔줌
	int val = hex_to_dec(value);
	if(addr >= MAX_MEMORY || addr < 0){
		printf("error! address error\n");
		return -1;
	}
	if(val > hex_to_dec("FF")){
		printf("error! value error\n");
		return -1;
	}

	memory[addr%16][addr/16] = val;
	return 1;
}
/*
 * start부터 end까지의 주소에 해당하는 메모리를
 * value값으로 채움
 * */
int mem_fill(char start[], char end[], char value[]){
	if(is_hex(start)  * is_hex(end) * is_hex(value) == 0){ //16진수가 아닌 경우 0 리턴
		return 0;
	}
	int i_start = hex_to_dec(start);
	int i_end = hex_to_dec(end);
	int val = hex_to_dec(value);
	
	if(i_start >= MAX_MEMORY || i_end >= MAX_MEMORY){ //메모리 범위 벗어난 경우 에러메세지
		printf("error! index must be less than memory size\n");
		return -1;
	}
	else if(i_start > i_end){
		printf("error! start must be less than end\n");
		return -1;
	}
	if(val > hex_to_dec("FF")){
		printf("error! value error\n");
		return -1;
	}
	
	for (int i=i_start; i<=i_end;i++){
		memory[i%16][i/16] = val;
	}
	return 1;
}

int main(void){
	char input[50]; //사용자로 부터의 입력
	mem_reset();	
	make_hash();
	int valid_flag;
	while(1){
		printf("sicsim> "); //입력 프롬프트 상태
		fgets(input,50,stdin); //입력
		input[strlen(input)-1]=0; //입력의 마지막 엔터 값을 null 값으로 변경
		valid_flag = 0; //명령어 오류 처리를 위한 flag
		if(strcmp(input,"q") * strcmp(input,"quit") == 0) break; //quit 혹은 q 입력 시 종료
		
		//history 혹은 hi 입력 시 히스토리 출력
		if(strcmp(input,"hi") * strcmp(input,"history")==0) {
			print_history(input);
			valid_flag = 1;
			continue;
		}
		if(strcmp(input,"help") == 0) {
			help(); 	
			valid_flag = 1;
		}
		//opcodelist 입력시 opcodeList 출력
		if(strcmp(input, "opcodelist")==0) {
			opcodeList(); 
			valid_flag = 1;
		}
		//d 또는 dir 입력 시 디렉토리 출력
		if(strcmp(input, "d") * strcmp(input, "dir") == 0) { 
			directory(); // dir 명령어
			valid_flag = 1;
		}
		if(strcmp(input, "reset") == 0){ //reset 입력시 메모리 초기화
			mem_reset();
			valid_flag = 1;
		}
		//인자가 여러개인 경우 처리
		if(valid_flag == 1){
			history(input);
			continue;
		}

		char *inArr[10] = {NULL, };
		int index = 0;
		char tmpinput[30];
		strcpy(tmpinput, input); //tmpinput에 input값 임시 저장. history를 위해서
		char *ptr = strtok(input," ");
		while(ptr != NULL){
			inArr[index] = ptr;
			index++;
			ptr = strtok(NULL, " ");
		}
	
		//opcodelist 입력 시 list 출력
		if(strcmp(inArr[0], "du") * strcmp(inArr[0], "dump") == 0 && index <= 3){
			if(index == 3)
				(inArr[1])[strlen(inArr[1])-1] = '\0';
			valid_flag = dump_print(inArr[1],inArr[2], index);	
		}
		if(strcmp(inArr[0], "f") * strcmp(inArr[0],"fill")==0 && index == 4){	
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			(inArr[2])[strlen(inArr[2])-1] = '\0';
			valid_flag = mem_fill(inArr[1], inArr[2], inArr[3]);
		}
		if(strcmp(inArr[0],"e") * strcmp(inArr[0],"edit") == 0 && index == 3){
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			valid_flag = mem_edit(inArr[1], inArr[2]);
		}
		if(strcmp(inArr[0], "opcode") == 0 && index == 2){
			valid_flag = mnemonic(inArr[1]);	
		}

		if(valid_flag == 1) //valid한 명령어가 들어온 경우 history에 저장
			history(tmpinput);
		else if(valid_flag == 0)
			printf("error! invalid command\n");
		/* TODO 명령어 처리, 에러처리
		 * dump, edit, fill, reset
		 * opcode mnemonic
		 * dir 형식 정리
		 * 변수 이름 바꾸기
		 *
		 * */


	}


	return 0;
}
