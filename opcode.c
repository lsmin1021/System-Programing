/*
 * opcode.c
 * opcode 관련 함수들
 *
 * 2020/04/04 v.01
 *
 */

#include "20181666.h"


HASHLIST* opcode_table = NULL; //opcode list를 위한 링크드 리스트 헤드


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
	
	if(is_hex(code) == 0) //code 값이 16진수가 아닌 경우 예외 처리
		return 0;
	code_num = hex_to_dec(code);
	
	// key_num 설정 하기
	int key_num, len_inst;
	len_inst = strlen(instruction);

	//instruction의 가운데 문자 아스키값 + 문자열 길이를 KEY로 나눈 나머지값을
	//KEY_num으로 설정
	key_num = (instruction[len_inst/2]+len_inst)%KEY;
	
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
	return 1; 

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
	
	// 각 줄마다 정보를 저장해줄 변수들
	char code[3];
	char instruction[7]; 	
	char format[4];
	
	int op_flag=0, in_flag=0;//값을 저장하기 위해 flag
	while(fscanf(fp,"%s",line) != EOF){
		if(op_flag == 0){
			strcpy(code, line);
			op_flag = 1;
		}
		else if(in_flag == 0){
			strcpy(instruction, line);
			in_flag = 1;
		}
		else{
			strcpy(format, line);
			put_hash(code,instruction,format);
			op_flag = 0; in_flag=0;
		}
	}
	
	//파일닫기
	fclose(fp);

}
/*
 * opcode 관련 함수들
 *
 * mnemonic, opcodeList
 * */
int mnemonic(char str[]){ //instruction에 해당하는 opCode 출력
	int flag = -1; // opCode를 찾았으면 1로 바꾸는 flag
	int key = (str[strlen(str)/2]+strlen(str))%KEY; //key값 계산
	HASH* temp = opcode_table[key].link;
	while(temp!= NULL){
		if(strcmp(temp->instruction, str)==0){
			printf("opcode is %02X\n",temp->opCode);
			flag = 1; break;
		}
		temp = temp->link;
	}
	if(flag == -1)
		printf("error! There is no %s in opcodeList\n",str); //Hash 테이블에 없는 경우 에러 처리
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
void free_hash(){ //hash table의 메모리를 free해줌
	for(int i=0;i<20;i++){
		HASH *temp = opcode_table[i].link;
		HASH *del;
		while(temp != NULL){
			del = temp;
			temp = temp->link;			
			free(del);
		}
	}	
	
	free(opcode_table);

}

