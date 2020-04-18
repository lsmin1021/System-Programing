#include "20181666.h"

char directives[6][6] =
	{"START", "END", "BYTE",
	"WORD", "RESB", "RESW"}; //assembler directives의 목록
ASM *asm_head = NULL;


//str이 directives이면 1리턴, 아니면 0 리턴
int is_directive(char str[]){
	for(int i=0;i<6;i++){
		if(strcmp(directives[i], str) == 0)
			return 1;
	}
	return 0;
}
//str이 mnemonic이 맞으면 opcode 리턴, 아니면 -1 리턴
int is_mnemonic(char str[]){
	int flag = 0; //opcode를 찾으면 1로 바뀌는 flag
	int key = (str[strlen(str)/2]+strlen(str))%KEY;
	HASH* temp = opcode_table[key].link;
	while(temp != NULL){
		if(strcmp(temp->instruction, str)==0){
			flag = 1; break;
		}
		temp = temp->link;
	}
	if(flag == 1)
		return temp->opCode;
	else return -1;
}
//str이 적절한 instruction인지 판단하는 함수 아니면 0 리턴
int is_valid_inst(char str[]){
	if(str[0] == '+'){
		char dest[10];
		strncpy(dest, str+1, strlen(str)-1);
		if(is_mnemonic(dest) != -1)
			return 4; //format이 4인 경우
	}
	else if(is_directive(str) == 1) //directives면 1 return
		return 1;
	else if(is_mnemonic(str) != -1) //mnemonic이면 3 return format이 3
		return 3; 
	else if(strcmp(str, "BASE") == 0) //BASE이면 2 return
		return 2;
	return 0;
}
//제대로 넣으면 1 리턴, 중복됐거나 오류나면 0 리턴
int put_symbol(char label[],int loc){
	SYMBOL* new_node;
	if(symbol_head == NULL){
		new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
		strcpy(new_node->symbol, label);
		new_node->loc = loc;
		new_node->link = NULL;
		symbol_head = new_node;

		return 1;
	}
	else{
		SYMBOL* temp = symbol_head;
		if(strcmp(temp->symbol, label) == 0){ //맨 처음 중복인 경우
			printf("error! duplicate symbol\n");
			return 0;
		}
		while(temp->link != NULL){
			if(strcmp(temp->symbol, label) == 0){//중복
				printf("error! duplicate symbol\n");
				return 0;
			}
			temp = temp->link;
		}
		new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
		strcpy(new_node->symbol, label);
		new_node->loc = loc;
		new_node->link = NULL;
		temp->link = new_node;

		return 1;
	}
	
	return 0;
}
//해당 mnemonic의 format이 1 혹은 2면 해당 값 리턴 아니면 0 리턴
int find_format(char* str){
	int flag = 0; //opcode를 찾으면 1로 바뀌는 flag
	int key = (str[strlen(str)/2]+strlen(str))%KEY;
	HASH* temp = opcode_table[key].link;
	while(temp != NULL){
		if(strcmp(temp->instruction, str)==0){
			flag = 1; break;
		}
		temp = temp->link;
	}
	if(flag == 1){
		if(strcmp(temp->format,"3/4")!=0)
			return atoi(temp->format);
		return 0;
	}
	else return -1;


}
//asm 파일에서 한 줄씩 가져와 처리
int manage_line(char* line,int lineCnt, int* loc){

	//줄 맨 끝에 엔터 처리
	if(line[strlen(line)-1] == '\n')
		line[strlen(line)-1] = '\0';
	
	//새로운 줄 초기화
	LINE* new_line;
	new_line = (LINE*)malloc(sizeof(LINE));
	//빈 문자열로 초기화
	strcpy(new_line->comment, "");
	strcpy(new_line->label, "");
	strcpy(new_line->str, "");
	strcpy(new_line->operand, "");
	strcpy(new_line->operand2,"");
	new_line->line = lineCnt*5; //줄번호 저장
	new_line->loc = -1; //location 
	new_line->opCode = -1; //mnemonic인 경우 opCode값
	new_line->format = -1; //mnemonic인 경우 format
	new_line->indexed = 0; //mnemonic 중 indexed인 경우 1
	new_line->addressing = -1; //addressing 모드
	new_line->link = NULL; //다음 노드 link

	if(line[0] == '.') {//코멘트 처리i
		strcpy(new_line->comment,line);
		//link 처리
		if(asm_head->link == NULL){
			asm_head->link = new_line;
		}
		else{
			LINE* tmp = asm_head->link;
			while(tmp->link != NULL){
				tmp = tmp->link;
			}
			tmp->link = new_line;
		}
		return 1;
	}
	
	char *Arr[5]={NULL,}; //띄어쓰기로 구분
	int cnt = 0; //개수
	char tmp[31];
	
	int valid_flag = -1; //error나는지 확인
	strcpy(tmp, line);
	char *ptr = strtok(tmp," ");
	while(ptr != NULL){
		Arr[cnt] = ptr;
		cnt++;
		ptr = strtok(NULL, " ");
	}
	if(cnt == 3){ //label이 있는 경우 혹은 operand가 2개인 경우
		if(is_valid_inst(Arr[0]) == 3 || is_valid_inst(Arr[0]) == 4){//첫번 문자열검사
			if(Arr[1][strlen(Arr[1])-1] == ',') //쉼표 처리
				Arr[1][strlen(Arr[1])-1] = '\0';
			
			//정보 저장
			strcpy(new_line->str, Arr[0]);
			new_line->opCode = is_mnemonic(Arr[0]);
			new_line->loc = *loc;
			valid_flag = is_valid_inst(Arr[0]);	
			if(valid_flag == 3){
				if(find_format(Arr[0]) == 1||find_format(Arr[0]) == 2)
					 valid_flag = find_format(Arr[0]); //format에 맞게 조정
			}
			new_line->format = valid_flag;
			*loc += valid_flag;

			char operandTmp[10];
			for(int i=0;i<10;i++)
				operandTmp[i] = 0;
			if(Arr[1][0] == '#'){ //intermediate addressing
				new_line->addressing = 2;
				strncpy(operandTmp, Arr[1]+1, strlen(Arr[1])-1); 
			}
			else if(Arr[1][0] == '@'){//indirect addressing
				new_line->addressing = 3;
				strncpy(operandTmp, Arr[1]+1, strlen(Arr[1])-1); 
			}
			else{
				new_line->addressing = 0; //single addressing
				strcpy(operandTmp, Arr[1]); 
			}
			strcpy(new_line->operand,operandTmp);
			

			for(int i=0;i<10;i++)
				operandTmp[i] = 0;
			if(Arr[2][0] == '#'){ //intermediate addressing
				new_line->addressing = 2;
				strncpy(operandTmp, Arr[2]+1, strlen(Arr[2])-1); 
			}
			else if(Arr[2][0] == '@'){//indirect addressing
				new_line->addressing = 3;
				strncpy(operandTmp, Arr[2]+1, strlen(Arr[2])-1); 
			}
			else{
				new_line->addressing = 0; //single addressing
				strcpy(operandTmp, Arr[2]); 
			}
			strcpy(new_line->operand2,operandTmp);
			
			if(strcmp(Arr[2], "X") == 0)
				new_line->indexed = 1; //indexed인 경우


		}
		else{	
			valid_flag = is_valid_inst(Arr[1]); //두번째 문자열 검사
			//label 처리
			if(put_symbol(Arr[0], *loc) == 1){ //symbol에러가 안나면 label 저장
				strcpy(new_line->label,Arr[0]);
			}
			else{
				free(new_line); //error난 경우 free
				return -1; //symbol이 error난 경우 -1 return
			}

			if(valid_flag == -1){
				printf("assembly error!\n"); //TODO error 메세지
				free(new_line); //error난 경우 free
				return -1; //제대로 안됐으면 -1 return
			}
			if(valid_flag != 2) //BASE인 경우 loc -1
				new_line->loc = *loc;
			else{
				strcpy(new_line->str, Arr[1]); //BASE인 경우 처리
				strcpy(new_line->operand, Arr[2]);
			}
			if(valid_flag == 1){ //directives
				strcpy(new_line->str, Arr[1]); //str에 저장
				if(strcmp(Arr[1],"START")==0 && asm_head->start == -1){ //TODO START 처리
					asm_head->start = atoi(Arr[2]); //맨 처음 시작 주소 저장
					*loc = asm_head->start;//location 저장
					new_line->loc = *loc; //START의 location은 시작주소
				}
				if(strcmp(Arr[1], "END") == 0){
					asm_head->end = *loc; //END의 경우 location 없음
					new_line->loc = -1;
					//TODO end 이면 끝나게?
				}
				if(strcmp(Arr[1], "WORD") == 0){ //WORD인 경우
					*loc += 3; //word 수 * 3
				}
				else if(strcmp(Arr[1], "BYTE") == 0){
					*loc += (strlen(Arr[2])-3); //TODO byte 길이 구하기
				}
				else if(strcmp(Arr[1], "RESW") == 0){
					*loc += 3 * atoi(Arr[2]); 
				}
				else if(strcmp(Arr[1], "RESB") == 0){
					*loc += atoi(Arr[2]);
				}
				strcpy(new_line->operand, Arr[2]); //operand에 값 저장
			}
			else if(valid_flag == 3 || valid_flag == 4){ //format 3 mnemonic
				if(valid_flag == 3){
					if(find_format(Arr[1]) == 1||find_format(Arr[1]) == 2)
						valid_flag = find_format(Arr[1]); //format에 맞게 조정

				}

				*loc += valid_flag; //format 에 따라 location 증가
				new_line->format = valid_flag;
				if(valid_flag == 4){ //format 4인 경우
					char dest[10];
					for(int i=0;i<10;i++){
						dest[i] = 0;
					}
					strncpy(dest, Arr[1]+1, strlen(Arr[1])-1);
					new_line->opCode = is_mnemonic(dest);
					strcpy(new_line->str, dest);
				}
				else{ //format 4인 경우 modification??? TODO
					new_line->opCode = is_mnemonic(Arr[1]);
					strcpy(new_line->str, Arr[1]);
				}
				char operandTmp[10]; //operand를 저장하기 위한 변수
				if(Arr[2][0] == '#'){ //intermediate addressing
					new_line->addressing = 2;
					strncpy(operandTmp, Arr[2]+1, strlen(Arr[2])-1); 
				}
				else if(Arr[2][0] == '@'){//indirect addressing
					new_line->addressing = 3;
					strncpy(operandTmp, Arr[2]+1, strlen(Arr[2])-1); 
				}
				else{
					new_line->addressing = 0; //single addressing
					strcpy(operandTmp, Arr[2]); 
				}
				strcpy(new_line->operand, operandTmp); //operand 저장
				//sic addressing 인 경우는 없나/..? TODO  확인해보기


			}
		}
	}
	else if(cnt == 2){ //label 없고 operand 하나인 경우
		valid_flag = is_valid_inst(Arr[0]);
		if(valid_flag == -1){
			printf("assembly error!11\n");
			free(new_line);
			return -1;
		}
		if(valid_flag != 2) //BASE인 경우 loc -1
			new_line->loc = *loc;
		else{
			strcpy(new_line->str, Arr[0]); //BASE인 경우 처리
			strcpy(new_line->operand, Arr[1]);
		}
		if(valid_flag == 1){ //directives인 경우 아마 END만
			strcpy(new_line->str, Arr[0]); //str에 저장
			if(strcmp(Arr[0],"START")==0 && asm_head->start == -1){ //TODO START 처리
				asm_head->start = atoi(Arr[1]); //맨 처음 시작 주소 저장
				*loc = asm_head->start;//location 저장
				new_line->loc = *loc; //START의 location은 시작주소
			}
			if(strcmp(Arr[0], "END") == 0){
				asm_head->end = *loc; //END의 경우 location 없음
				new_line->loc = -1;
				//TODO end 이면 끝나게?
			}
			
			strcpy(new_line->operand, Arr[1]); //operand에 값 저장

		}
		else if(valid_flag == 3 || valid_flag == 4){ //format 3 mnemonic
			if(valid_flag == 3){
				if(find_format(Arr[0]) == 1||find_format(Arr[0]) == 2)
					valid_flag = find_format(Arr[0]); //format에 맞게 조정
			}
			*loc += valid_flag; //format 3인 경우 location 3byte 증가
			new_line->format = valid_flag;
			
			//opCode, mnemonic 처리
			if(valid_flag == 4){ //format 4인 경우 + 제거
				char dest[10];
				for(int i=0;i<10;i++){
					dest[i] = 0;
				}
				strncpy(dest, Arr[0]+1, strlen(Arr[0])-1); 
				new_line->opCode = is_mnemonic(dest);
				strcpy(new_line->str, dest);
			}
			else{ //format 4인 경우 modification??? TODO
				new_line->opCode = is_mnemonic(Arr[0]);
				strcpy(new_line->str, Arr[0]);
			}

			//operand 처리
			char operandTmp[10]; //operand를 저장하기 위한 변수
			for(int i=0;i<10;i++){
				operandTmp[i] = 0;
			}
			if(Arr[1][0] == '#'){ //intermediate addressing
				new_line->addressing = 2;
				strncpy(operandTmp, Arr[1]+1, strlen(Arr[1])-1); 
			}
			else if(Arr[1][0] == '@'){//indirect addressing
				new_line->addressing = 3;
				strncpy(operandTmp, Arr[1]+1, strlen(Arr[1])-1); 
			}
			else{
				new_line->addressing = 0; //single addressing
				strcpy(operandTmp, Arr[1]); 
			}
			strcpy(new_line->operand, operandTmp); //operand 저장
			//sic addressing 인 경우는 없나/..? TODO  확인해보기


		}
	}
	else if(cnt == 1){
		valid_flag = is_valid_inst(Arr[0]);
		if(valid_flag == -1){
			printf("assembly error!\n");
			free(new_line);
			return -1;
		}
		if(valid_flag != 2) //BASE인 경우 loc -1
			new_line->loc = *loc;
		else
			strcpy(new_line->str, Arr[0]);

		if(valid_flag == 3 || valid_flag == 4){
			if(valid_flag == 3){
				if(find_format(Arr[0]) == 1||find_format(Arr[0]) == 2)
					valid_flag = find_format(Arr[0]); //format에 맞게 조정
			}
			*loc += valid_flag;
			new_line->format = valid_flag;
			if(valid_flag == 4){ //format 4인 경우 + 제거
				char dest[10];
				for(int i=0;i<10;i++){
					dest[i] = 0;
				}
				strncpy(dest, Arr[0]+1, strlen(Arr[0])-1); 
				new_line->opCode = is_mnemonic(dest);
				strcpy(new_line->str, dest);
			}
			else{ //format 4인 경우 modification??? TODO
				new_line->opCode = is_mnemonic(Arr[0]);
				strcpy(new_line->str, Arr[0]);
			}

		}

	}
	else if(cnt == 4){ //label있고 operand 2개인 경우
		//TODO + indexed 처리
	}
	
	//link 처리
	if(asm_head->link == NULL){
		asm_head->link = new_line;
	}
	else{
		LINE* tmp = asm_head->link;
		while(tmp->link != NULL){
			tmp = tmp->link;
		}
		tmp->link = new_line;
	}

	return 1;
}
//asm 파일을 읽어 링크드 리스트로 저장
int read_file(char* filename){

	FILE *assem = fopen(filename, "r");
	
	if (assem == NULL){
		printf("error! There is no %s\n", filename);
		return -1;
	}
	//TODO head의 head??
	//asm_head 초기화
	asm_head = (ASM*)malloc(sizeof(ASM));
	//asm_head->name = '\0'; //TODO start name 저장
	asm_head->start = -1;
	asm_head->end = -1; 
	asm_head->link = NULL;

	//symbol_head 초기화
//	symbol_head = (SYMBOL*) mallco(sizeof(SYMBOL));
	symbol_head = NULL;

	
	char temp[51];
	int lineCnt = 1;
	int loc = -1;
	while(fgets(temp,51,assem) != NULL){ //파일의 끝까지 읽기
		//printf("%s",temp);
		if(manage_line(temp, lineCnt, &loc) == -1){
			//printf("error!\n");
		}
	
		lineCnt++;
		strcpy(temp, "");
	}


	fclose(assem);
	return 1;


}
void print_asm(){
	LINE* tmp = asm_head->link;
	while(tmp!= NULL){
		if(strcmp(tmp->comment,"") != 0){
			printf("%d\t/%s\n",tmp->line,tmp->comment);
			
		}
		else if(tmp->loc != -1)
		printf("%d\t/%04X\t/%s\t/%s\t/%s\t/%s\n",tmp->line,tmp->loc,tmp->label,tmp->str,tmp->operand,tmp->operand2);
		else
			printf("%d\t/\t/%s\t/%s\t/%s\t/%s\t\n",tmp->line, tmp->label, tmp->str, tmp->operand,tmp->operand2);
		tmp = tmp->link;
	}

}

void make_lst(){




//	fclose(fp);


}
