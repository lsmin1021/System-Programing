/*
 * sp project 1
 * 20181666 이승민
 *
 * 20181666.c
 * 메인 함수 및 기본 함수들
 *
 * 2020/04/04 v.01
 *
 */


#include "20181666.h"

char command[CNT_COMMAND][LEN_COMMAND]={"h[elp]","d[ir]","q[uit]","hi[story]",
	"du[mp] [start, end]","e[dit] address, value", "f[ill] start, end, value",
	"reset", "opcode mnemonic", "opcodelist",
	"assemble filename", "type filename", "symbol"}; //명령어 목록


HISTORY* h_head = NULL; //히스토리를 저장하는 링크드리스트 헤더
int history_num = 0 ; //현재 히스토리에 저장된 개수

void help(){ //명령어 목록 출력
	for(int i=0;i< CNT_COMMAND ;i++){
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

void free_history(){ //history의 linked list를 free해줌

	HISTORY *temp = h_head;
	HISTORY *del;
	while(temp != NULL){
		del = temp;
		temp = temp -> link;
		free(del);
	}

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
		else if('a' <= ch && ch <= 'f')
			temp = ch - ('a' -10);
		else
			return -1; //잘못된 값이 들어온 경우 -1 리턴
		
		num += temp * mul;
		mul *= 16;
	}
	return num; //십진수로 변환한 값 반환
}
int is_hex(char hex[]){ //해당 문자열이 16진수가 맞는지 판단하는 함수
	int len = strlen(hex);
	for(int i=0; i<len; i++){
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
void ascii_print(int mem){ //memory에 값을 ascii 값으로 출력해주는 함수	
	if(hex_to_dec(ASCII_FIRST) <= mem && mem <= hex_to_dec(ASCII_LAST))
		printf("%c",mem);
	else
		printf(".");
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
				printf("%15s/\t",ent->d_name);
			}
			else if(S_ISREG(buf.st_mode)){
				if(buf.st_mode & 01001001)
					printf("%15s*\t",ent->d_name);
				else
					printf("%15s \t",ent->d_name);
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
/*
 * filename을 읽어 해당 파일의 내용 출력
 *
 * 해당 파일이 없는 경우 에러 메세지
 */
int type_func(char* filename){
	FILE *fp = fopen(filename, "r");//filename에 해당하는 파일 열기
	if( fp == NULL){ //파일이 없는 경우 에러처리
		printf("error! There is no %s in this directory\n", filename);
		return -1;
	}

	char temp[10000];
	while( fgets(temp,10000,fp) != NULL){
		printf("%s",temp);
	}

	fclose(fp); //파일 닫기
	return 1;

}
/*
* 동적 할당한 메모리들을 free 해줌
* */
void free_memory(){
	free_hash(); //hash table free
	free_history(); //history 내역 free
}	
int main(void){
	char input[50]; //사용자로 부터의 입력
	mem_reset();
	make_hash();
	int valid_flag;
	while(1){ //종료 시 까지 무한 반복
		printf("sicsim> "); //입력 프롬프트 상태
		if(fgets(input,50,stdin)==NULL)
			break;//입력
		if(strlen(input) == 1 || input[0] == ' ') { //엔터만 입력 시 continue
			continue;
		}
		input[strlen(input)-1]=0; //입력의 마지막 엔터 값을 null 값으로 변경
		valid_flag = 0; //명령어 오류 처리를 위한 flag
		if(strcmp(input,"q") * strcmp(input,"quit") == 0) break; //quit 혹은 q 입력 시 종료
		
		//history 혹은 hi 입력 시 히스토리 출력
		if(strcmp(input,"hi") * strcmp(input,"history")==0) {
			print_history(input);
			valid_flag = 1;
			continue;
		}
		//help 혹은 h 입력 시 명령어 목록 출력
		if(strcmp(input,"h") * strcmp(input,"help") == 0) {
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
		
		//인자가 하나인 경우 history에 넣고 continue
		if(valid_flag == 1){
			history(input);
			continue;
		}

		//인자가 여러개인 경우 처리
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
	
		//du 혹은 dump 입력 시 메로리 정보 출력
		if(strcmp(inArr[0], "du") * strcmp(inArr[0], "dump") == 0 && index <= 3){
			//',' 입력 안했을 경우 대비
			if(index == 3 && (inArr[1])[strlen(inArr[1])-1] == ','){
				(inArr[1])[strlen(inArr[1])-1] = '\0';
			}
			valid_flag = dump_print(inArr[1],inArr[2], index);	
		}
		//f 또는 fill 입력 시 start 부터 end 까지 value 값으로 채움
		if(strcmp(inArr[0], "f") * strcmp(inArr[0],"fill")==0 && index == 4){	
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			(inArr[2])[strlen(inArr[2])-1] = '\0';
			valid_flag = mem_fill(inArr[1], inArr[2], inArr[3]);
		}
		//e 또는 edit 입력 시 주소에 해당ㅇ하는 값을 value로 바꿈
		if(strcmp(inArr[0],"e") * strcmp(inArr[0],"edit") == 0 && index == 3){
			(inArr[1])[strlen(inArr[1])-1] = '\0';
			valid_flag = mem_edit(inArr[1], inArr[2]);
		}
		//opcode 입력 시 mnemonic에 해당하는 opcode 출력
		if(strcmp(inArr[0], "opcode") == 0 && index == 2){
			valid_flag = mnemonic(inArr[1]);	
		}

		//type filename 입력 시 file의 내용 출력
		if(strcmp(inArr[0], "type") == 0 && index == 2){
			valid_flag = type_func(inArr[1]);
		}
		

		if(valid_flag == 1) //valid한 명령어가 들어온 경우 history에 저장
			history(tmpinput);
		else if(valid_flag == 0) //invalid한 경우 예외 처리
			printf("error! invalid command\n");

	}
	//FILE *fp = fopen("aa","r");
	make_lst();
	read_file("2_5.asm");
	print_asm();
	free_memory();
	
	return 0;
}
