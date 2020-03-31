#include "header.h"

#define LEN_COMMAND 30 //명령어 최대 길이
#define KEY 19 //hash table을 만들기 위한 key
int memory[16][65536]; //가상의 메모리 공간



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

void dump(int start, int end){




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
	
	int key_num = code_num%KEY; //opCode의 십진수 값을 KEY 값으로 나눈 값 : HASH
	
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
void mnemonic(char str[]){
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
int main(void){
	char input[50]; //사용자로 부터의 입력
	for(int i=0;i<16;i++){
		for(int j=0;j<65536;j++){
			memory[i][j] = 0;
		}
	}
	while(1){
		printf("sicsim> "); //입력 프롬프트 상태
		fgets(input,50,stdin); //입력
		input[strlen(input)-1]=0; //입력의 마지막 엔터 값을 null 값으로 변경


		if(strcmp(input,"q") * strcmp(input,"quit") == 0) break; //quit 혹은 q 입력 시 종료
		
		//history 혹은 h 입력 시 히스토리 출력
		if(strcmp(input,"h") * strcmp(input,"history")==0) print_history(input);
		else history(input);
		
		

//		printf("\n%s\n",input);

	}

	make_hash();
//	opcodeList();
	printf("\n");
//	mnemonic("LDB");
//	memory[0][0] = 255;
//	printf("hehe %X\n",memory[0][0]);

//	help();
	directory();	
	return 0;
}
