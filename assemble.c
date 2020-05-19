/*
 * assemble.c
 * assemble을 위한 코드
 *
 * 2020/05/19 v.02
 * 예외 처리
 */

#include "20181666.h"

char directives[6][6] =
	{"START", "END", "BYTE",
	"WORD", "RESB", "RESW"}; //assembler directives의 목록

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
//str이 적절한 instruction인지 판단하는 함수 아니면 -1 리턴
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
	return -1;
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
			printf("error! duplicate symbol \n");
			return 0;
		}
		while(temp->link != NULL){
			if(strcmp(temp->symbol, label) == 0){//중복
				printf("error! duplicate symbol ");
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
	strcpy(new_line->obj, "");
	new_line->line = lineCnt*5; //줄번호 저장
	new_line->loc = -1; //location 
	new_line->opCode = -1; //mnemonic인 경우 opCode값
	new_line->format = -1; //mnemonic인 경우 format
	new_line->indexed = 0; //mnemonic 중 indexed인 경우 1
	new_line->addressing = -1; //addressing 모드
	new_line->modified = 0; //modification 해야하는 것 저장
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
	//cnt에 따른 처리
	if(cnt == 1){
		valid_flag = is_valid_inst(Arr[0]);
		if(valid_flag == -1){
			printf("error! assembly error at %d line\n",new_line->line);
			
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
			new_line->addressing = 1;
			if(valid_flag == 4){ //format 4인 경우 + 제거
				char dest[10];
				for(int i=0;i<10;i++){
					dest[i] = 0;
				}
				new_line->opCode = is_mnemonic(dest);
			}
			else{ 
				new_line->opCode = is_mnemonic(Arr[0]);
			}
			strcpy(new_line->str, Arr[0]);
		}

	}
	else if(cnt == 2){ //label 없고 operand 하나인 경우
		valid_flag = is_valid_inst(Arr[0]); 
		if(valid_flag == -1){
			valid_flag = is_valid_inst(Arr[1]); //두번째 문자열 검사 (label있는 경우)
			//label 처리. symbol list에 label 저장
			
			if(valid_flag == -1){ //두 번째 문자열도 오류인 경우 error
				printf("error! assembly error at %d line\n",new_line->line); 
				free(new_line); //error난 경우 free
				return -1; //제대로 안됐으면 -1 return
			}
			if(strcmp(Arr[1],"START")==0 && asm_head->start == -1){ 
				asm_head->start = hex_to_dec(Arr[2]); //맨 처음 시작 주소 저장
				*loc = asm_head->start;//location 저장
				new_line->loc = *loc; //START의 location은 시작주소
				strcpy(asm_head->name, Arr[0]);
				strcpy(new_line->label, Arr[0]);
				strcpy(new_line->str, Arr[1]);
				strcpy(new_line->operand, "0"); //시작 주소 없는 경우 0?
				
				if(asm_head->link == NULL){
					asm_head->link = new_line;
				}
				return 1;
			}
			if(strcmp(Arr[1], "END") == 0){
				asm_head->end = *loc; //END의 경우 location 없음
				new_line->loc = *loc;
				strcpy(new_line->label, Arr[0]);
				strcpy(new_line->str, Arr[1]);
				strcpy(new_line->operand, Arr[2]);

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

				return 2; //END면 2 return	
			}

			if(put_symbol(Arr[0], *loc) == 1){ //symbol에러가 안나면 label 저장
				strcpy(new_line->label,Arr[0]);
			}
			else{
				printf("at %d line\n", new_line->line);
				free(new_line); //error난 경우 free
				return -1; //symbol이 error난 경우 -1 return
			}

			new_line->loc = *loc; //BASE가 아닌 경우 *loc
			
			if(valid_flag == 3){
				if(valid_flag == 3){
					if(find_format(Arr[0]) == 1||find_format(Arr[0]) == 2)
						valid_flag = find_format(Arr[0]); //format에 맞게 조정
				}
				*loc += valid_flag;
				new_line->format = valid_flag;
				new_line->addressing = 1;
				new_line->opCode = is_mnemonic(Arr[1]);
				strcpy(new_line->str, Arr[1]);
			}
			else{ //format4, directives가 들어울 수는 없을 듯 
				printf("assembly error at %d line\n", new_line->line);
				free(new_line);
				return -1;
			}
		}

		if(valid_flag != 2) //BASE인 경우 loc -1
			new_line->loc = *loc;
		else{
			strcpy(new_line->str, Arr[0]); //BASE인 경우 처리
			strcpy(new_line->operand, Arr[1]);
		}

		if(valid_flag == 1){ //directives인 경우 아마 END만
			strcpy(new_line->str, Arr[0]); //str에 저장
			strcpy(new_line->operand, Arr[1]); //operand에 값 저장
			if(strcmp(Arr[0],"START")==0 && asm_head->start == -1){
				asm_head->start = hex_to_dec(Arr[1]); //맨 처음 시작 주소 저장
				*loc = asm_head->start;//location 저장
				new_line->loc = *loc; //START의 location은 시작주소
				strcpy(asm_head->name,"");
				
				if(asm_head->link == NULL){
					asm_head->link = new_line;
				}
				return 1;

			}
			if(strcmp(Arr[0], "END") == 0){
				asm_head->end = *loc; //END의 경우 location 없음
				new_line->loc = *loc;

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

				return 2; //END면 2 return	

			}
		}
		else if(valid_flag == 3 || valid_flag == 4){ //format 3 mnemonic
			if(valid_flag == 3){
				if(find_format(Arr[0]) == 1 || find_format(Arr[0]) == 2)
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
			}
			else{ 
				new_line->opCode = is_mnemonic(Arr[0]);
			}
			strcpy(new_line->str, Arr[0]);

			if(Arr[1][0] == '#'){ //immediate addressing
				new_line->addressing = 2;
			}
			else if(Arr[1][0] == '@'){//indirect addressing
				new_line->addressing = 3;
			}
			else{
				new_line->addressing = 1; //simple addressing
			}
			strcpy(new_line->operand, Arr[1]); //operand 저장


		}
	}
	else if(cnt == 3){ //label이 있는 경우 혹은 operand가 2개인 경우
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

			if(Arr[1][0] == '#'){ //immediate addressing
				new_line->addressing = 2;
			}
			else if(Arr[1][0] == '@'){//indirect addressing
				new_line->addressing = 3;
			}
			else{
				new_line->addressing = 1; //simple addressing
			}
			strcpy(new_line->operand,Arr[1]);
			strcpy(new_line->operand2,Arr[2]);
			
			if(strcmp(Arr[2], "X") == 0)
				new_line->indexed = 1; //indexed인 경우
		}
		else{
			
			valid_flag = is_valid_inst(Arr[1]); //두번째 문자열 검사
			//label 처리. symbol list에 label 저장
			
			if(valid_flag == -1){
				printf("error! assembly error at %d line\n",new_line->line); 
				free(new_line); //error난 경우 free
				return -1; //제대로 안됐으면 -1 return
			}
			if(strcmp(Arr[1],"START")==0 && asm_head->start == -1){ 
				asm_head->start = hex_to_dec(Arr[2]); //맨 처음 시작 주소 저장
				*loc = asm_head->start;//location 저장
				new_line->loc = *loc; //START의 location은 시작주소
				strcpy(asm_head->name, Arr[0]);
				strcpy(new_line->label, Arr[0]);
				strcpy(new_line->str, Arr[1]);
				strcpy(new_line->operand, Arr[2]);
				
				if(asm_head->link == NULL){
					asm_head->link = new_line;
				}
				return 1;
			}
			if(strcmp(Arr[1], "END") == 0){
				asm_head->end = *loc; //END의 경우 location 없음
				new_line->loc = *loc;
				strcpy(new_line->label, Arr[0]);
				strcpy(new_line->str, Arr[1]);
				strcpy(new_line->operand, Arr[2]);

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

				return 2; //END면 2 return	
			}

			if(put_symbol(Arr[0], *loc) == 1){ //symbol에러가 안나면 label 저장
				strcpy(new_line->label,Arr[0]);
			}
			else{
				printf("at %d line\n", new_line->line);
				free(new_line); //error난 경우 free
				return -1; //symbol이 error난 경우 -1 return
			}

			if(valid_flag != 2) //BASE인 경우 loc -1
				new_line->loc = *loc; //BASE가 아닌 경우 *loc
			else{
				strcpy(new_line->str, Arr[1]); //BASE인 경우 처리
				strcpy(new_line->operand, Arr[2]);
			}
			//valid_flag 값에 따른 directives, mnemonic처리
			if(valid_flag == 1){ //directives
				if(strcmp(Arr[1], "WORD") == 0){ //WORD인 경우
					*loc += 3; //word 수 * 3	
					new_line->format = 0;
				}
				else if(strcmp(Arr[1], "BYTE") == 0){
					if(Arr[2][0] == 'C') //C인 경우
						*loc += (strlen(Arr[2])-3); 
					else{ //16진수인 경우
						*loc += (strlen(Arr[2])-3)/2;

						// 16진수 범위 내에 속하는지 검사
						char hex_tmp[10];
						strcpy(hex_tmp, "");
						strncpy(hex_tmp, Arr[2]+2, strlen(Arr[2])-3);
						hex_tmp[strlen(Arr[2])-3] = '\0';	
						if(is_hex(hex_tmp) == 0){
							printf("error! assembly error at %d line\n", new_line->line);
							free(new_line);
							return -1;
						}
					}

					new_line->format = 0;
				}
				else if(strcmp(Arr[1], "RESW") == 0){
					*loc += 3 * atoi(Arr[2]); 
				}
				else if(strcmp(Arr[1], "RESB") == 0){
					*loc += atoi(Arr[2]);
				}
				strcpy(new_line->str, Arr[1]); //str에 저장
				strcpy(new_line->operand, Arr[2]); //operand에 값 저장
			}
			else if(valid_flag == 3 || valid_flag == 4){ //format 3 mnemonic
				if(valid_flag == 3){
					if(find_format(Arr[1]) == 1||find_format(Arr[1]) == 2)
						valid_flag = find_format(Arr[1]); //format에 맞게 조정
				}

				*loc += valid_flag; //format 에 따라 location 증가
				new_line->format = valid_flag; //format 저장
				
				if(valid_flag == 4){ //format 4인 경우
					char dest[10];
					for(int i=0;i<10;i++){
						dest[i] = 0;
					}
					strncpy(dest, Arr[1]+1, strlen(Arr[1])-1);
					new_line->opCode = is_mnemonic(dest);
				}
				else{ 
					new_line->opCode = is_mnemonic(Arr[1]);
				}
				strcpy(new_line->str, Arr[1]);

				if(Arr[2][0] == '#'){ //immediate addressing
					new_line->addressing = 2;
				}
				else if(Arr[2][0] == '@'){//indirect addressing
					new_line->addressing = 3;
				}
				else{
					new_line->addressing = 1; //simple addressing
				}
				strcpy(new_line->operand, Arr[2]); //operand 저장
			}
		}
	}
	else if(cnt == 4){ //label있고 operand 2개인 경우
		valid_flag = is_valid_inst(Arr[1]); //두번째 문자열 검사
		//label 처리. symbol list에 label 저장
		if(put_symbol(Arr[0], *loc) == 1){ //symbol에러가 안나면 label 저장
			strcpy(new_line->label,Arr[0]);
		}
		else{
			free(new_line); //error난 경우 free
			return -1; //symbol이 error난 경우 -1 return
		}

		if(valid_flag == -1){
			printf("error! assembly error at %d line\n",new_line->line); 
			free(new_line); //error난 경우 free
			return -1; //제대로 안됐으면 -1 return
		}

		if(valid_flag != 2) //BASE인 경우 loc -1
			new_line->loc = *loc; //BASE가 아닌 경우 *loc
		else{
			strcpy(new_line->str, Arr[1]); //BASE인 경우 처리
			strcpy(new_line->operand, Arr[2]);
		}
		//valid_flag 값에 따른 directives, mnemonic처리
		if(valid_flag == 3 || valid_flag == 4){ //format 3 mnemonic
			if(valid_flag == 3){
				if(find_format(Arr[1]) == 1||find_format(Arr[1]) == 2)
					valid_flag = find_format(Arr[1]); //format에 맞게 조정
			}

			*loc += valid_flag; //format 에 따라 location 증가
			new_line->format = valid_flag; //format 저장

			if(valid_flag == 4){ //format 4인 경우
				char dest[10];
				for(int i=0;i<10;i++){
					dest[i] = 0;
				}
				strncpy(dest, Arr[1]+1, strlen(Arr[1])-1);
				new_line->opCode = is_mnemonic(dest);
			}
			else{ 
				new_line->opCode = is_mnemonic(Arr[1]);
			}
			strcpy(new_line->str, Arr[1]);
			

			if(Arr[2][strlen(Arr[2])-1] == ',') //쉼표 처리
				Arr[2][strlen(Arr[2])-1] = '\0';

			if(Arr[2][0] == '#'){ //immediate addressing
				new_line->addressing = 2;
			}
			else if(Arr[2][0] == '@'){//indirect addressing
				new_line->addressing = 3;
			}
			else{
				new_line->addressing = 1; //simple addressing
			}
			strcpy(new_line->operand, Arr[2]); //operand 저장

			strcpy(new_line->operand2,Arr[2]);

			if(strcmp(Arr[2], "X") == 0)
				new_line->indexed = 1; //indexed인 경우
		}
	}

	if(new_line->format == 4){ //modification 해야하는 개수 세기
		if(new_line->addressing == 2){ //immediate 상수 들어올 때 처리
			if(new_line->operand[1] < '0' || new_line->operand[1] > '9'){
				asm_head->modify_cnt++;
				new_line->modified = 1;
			}			
		}
		else{
			asm_head->modify_cnt++;
			new_line->modified = 1;
		}
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
/*
 * asm 파일을 읽어 링크드 리스트로 저장
 * assemble 에러 발생 시 -1 return
 * 파일이 없는 경우 -2 return
 * 제대로 실행 된 경우 1 return
 *
 */
int read_file(char* filename){

	FILE *assem = fopen(filename, "r");

	asm_head = NULL;
	if (assem == NULL){
		printf("error! There is no %s!\n", filename);
		return -2;
	}

	char valid_type[30];
	strncpy(valid_type, filename+(strlen(filename)-4), 4);
	if(strcmp(valid_type, ".asm") != 0){
		printf("error! %s is not asm file!\n",filename);
		fclose(assem);
		return -2;
	}
	//asm_head 초기화
	asm_head = (ASM*)malloc(sizeof(ASM));
	strcpy(asm_head->name,"");
	asm_head->start = -1;
	asm_head->end = -1; 
	asm_head->link = NULL;
	asm_head->modify_cnt = 0;

	//symbol_head 초기화
	symbol_head = NULL;
	
	char temp[51];
	int lineCnt = 1;
	int loc = -1;

	int flag;
	while(fgets(temp,51,assem) != NULL){ //파일의 끝까지 읽기
		flag = manage_line(temp,lineCnt, &loc);
		if(flag == -1){
			return -1;
		}
		else if(flag == 2){
			break;
		}
		lineCnt++;
		strcpy(temp, "");
		
	}
	fclose(assem);
	return 1;
}
//register에 해당하는 number 리턴
int reg_num(char* reg){
	if(strcmp(reg,"A")==0)
		return 0;
	if(strcmp(reg,"X")==0)
		return 1;
	if(strcmp(reg,"A")==0)
		return 2;
	if(strcmp(reg,"B")==0)
		return 3;
	if(strcmp(reg,"S")==0)
		return 4;
	if(strcmp(reg,"T")==0)
		return 5;
	if(strcmp(reg,"F")==0)
		return 6;
	if(strcmp(reg,"PC")==0)
		return 8;
	if(strcmp(reg,"SW")==0)
		return 9;
	else{
		if(is_hex(reg)){
			return -2; //16진수이면 return -2
		}
		printf("error!\n"); 
		return -1;
	}
}
//symbol에 해당하는 loc return
int find_sym_loc(char* symbol){
	
	SYMBOL* tmp = symbol_head;
	while(tmp!= NULL){
		if(strcmp(tmp->symbol,symbol)==0)
			return tmp->loc;
		tmp = tmp->link;
	}
	return -1;
}
//한 줄의 object code 생성해서 return
char* line_objectcode(LINE* node){
	char* obj; //object code 저장하는 포인터
	obj = (char*)malloc(sizeof(char)*9);
	strcpy(obj,"");
	int index = 0; //obj의 index값 저장
	int format = node->format; //format 저장
	if(format == 0){ //상수
		char operand[20];
		strcpy(operand, node->operand);
		if(operand[0] == 'X'){
			for(int i=1; i<(int)strlen(operand); i++){
				if(operand[i] != '\'') // ' 가 아니면 복사
					obj[index++] = operand[i];
			}
			obj[index] = '\0';
		}
		else if(operand[0] == 'C'){
			for(int i=1; i<(int)strlen(operand); i++){
				if(operand[i] != '\''){
					char tmpstr[3]; //16진수로 바꾸기 위해 임시로 사용하는 문자열
					sprintf(tmpstr,"%X",operand[i]);
					for(int j=0;j<2;j++)
						obj[index++] = tmpstr[j];
				}
			}
			obj[index] = '\0';
		}
	}
	else if(format == 1){
		sprintf(obj,"%X",node->opCode);
	}
	else if(format == 2){
		int reg1, reg2;
		reg1 = reg_num(node->operand);
		if(strcmp(node->operand2,"")==0)
			reg2 = 0;
		else
			reg2 = reg_num(node->operand2);
		if(reg2 == -1){ //없는 register인 경우 에러
			printf("assembly error at %d line\n", node->line);
			return obj;
		}
		else if(reg2 == -2){ //16진수인 경우
			if(hex_to_dec( node->operand2) <16){
				sprintf(obj,"%X%X%s",node->opCode, reg1, node->operand2);
			}
			else{ //숫자 범위 벗어난 경우
				printf("assembly error at %d line\n", node->line); 
				return obj;
			}
		}
		else sprintf(obj,"%X%X%X",node->opCode, reg1,reg2); 
	}
	else if(format == 3){
		int optmp = node->opCode; //opCode 저장 변수
		int midtmp=0;
		int n,i,x=0,b=0,p=0,e=0;
		int disp, ptr;
		//addressing mode에 따라 n, i설정
		switch(node->addressing){
			case -1: n=0;i=0; //operand가 없는 경우
					 break;
			case 0 : n=0;i=0;
					break;
			case 1 : n=1;i=1; //simple
					break;
			case 2 : n=0;i=1; //immediate
					break;
			case 3 : n=1;i=0; //indirect
					break;
			default:break;
		}
		if(n==1)
			optmp += 2; //n=1인 경우 이진수 처리
		if(i==1)
			optmp += 1; //i==1인 경우 이진수 처리

		if(node->addressing == -1){ //operand가 없는 경우
			sprintf(obj,"%02X%02X%02X",optmp,0,0);
			return obj;
		}
		if(strcmp(node->operand, "") == 0){ //operand는 없지만 addressing은 1
			sprintf(obj,"%02X%02X%02X",optmp,0,0);
			return obj;
		}
		char dest[10];
		if(node->addressing != 1){
			for(int i=0;i<10;i++)
				dest[i] = 0;
			strncpy(dest, node->operand+1, strlen(node->operand)-1);
		}
		else
			strcpy(dest, node->operand);
		if(dest[0]>='0' && dest[0] <= '9'){
			disp = atoi(dest);
		}
		else{
			ptr = find_sym_loc(dest);
			if(ptr == -1){ 
				printf("error!! assembly error %dline\n",node->line);
				return obj;
			}
			int flag = 0; //disp가 음수이면 보수 처리를 위해 flag 세움
			disp = ptr-pc;
			if(disp<0){
				disp += 2048;
				flag = 1;
			}
			if(disp<0 || disp > hex_to_dec("FFF")){ //base relative
				b = 1;
				disp = ptr-base;
			}
			else{ //pc relative
				p = 1;
				if(flag ==1)
					disp += 2048;
			}
		}
		
		//indexed이면 x가 1
		if(node->indexed == 1){
			x = 1;
		}
		midtmp += 8*x + 4*b + 2*p + 1*e;
	
		sprintf(obj,"%02X%X%03X", optmp, midtmp, disp);
		if(strcmp(node->str,"LDB")== 0){
			base = find_sym_loc(dest);
		}
	}
	else if(format == 4){
		int optmp = node->opCode; //opCode 저장 변수
		int midtmp=0;
		int n,i,x=0,b=0,p=0,e=1;
		int address, ptr;
		//addressing mode에 따라 n, i설정
		switch(node->addressing){
			case -1: n=0;i=0; //operand가 없는 경우
					 break;
			case 0 : n=0;i=0;
					break;
			case 1 : n=1;i=1; //simple
					break;
			case 2 : n=0;i=1; //immediate
					break;
			case 3 : n=1;i=0; //indirect
					break;
			default:break;
		}
		if(n==1)
			optmp += 2; //n=1인 경우 이진수 처리
		if(i==1)
			optmp += 1; //i==1인 경우 이진수 처리

		if(node->addressing == -1){ //operand가 없는 경우
			sprintf(obj,"%X%02X%02X%02X",node->opCode,0,0,0);
			return obj;
		}
		
		char dest[10];
		if(node->addressing != 1){
			for(int i=0;i<10;i++)
				dest[i] = 0;
			strncpy(dest, node->operand+1, strlen(node->operand)-1);
		}
		else
			strcpy(dest, node->operand);
		if(dest[0]>='0' && dest[0] <= '9'){
			address = atoi(dest);
		}
		else{
			ptr = find_sym_loc(dest);
			if(ptr == -1){ 
				printf("error!! assembly error %dline\n",node->loc);
				return obj;
			}
			address = ptr;
		}
		//indexed이면 x가 1
		if(node->indexed == 1){
			x = 1;

		}
		midtmp += 8*x + 4*b + 2*p + 1*e;
		sprintf(obj,"%02X%X%05X", optmp, midtmp, address);

		if(strcmp(node->str,"LDB")== 0){
			base = find_sym_loc(dest);
		}
	}
	
	
	return obj;
}
int make_objectcode(){
	LINE* tmp = asm_head->link;
	
	//modification해야하는 location 저장하는 배열 동적 할당
	modify = (int*)malloc(sizeof(int)*asm_head->modify_cnt);
	int modify_index = 0;
	char* obj;
	obj = (char*)malloc(sizeof(char)*9);
	pc = asm_head->start;
	base = -1;
	while(tmp != NULL){
		if(strcmp(tmp->str, "END") == 0){ //END이면 종료
			break;
		}
		obj = (char*)malloc(sizeof(char)*9);
		strcpy(obj,"");
		
		LINE *pctmp = tmp->link;
		pc = pctmp->loc; //pc는 다음 instruction의 location
		while(pc == -1){
			pctmp = pctmp->link;
			pc = pctmp->loc;
		}

		if(tmp->format != -1){//-1인 경우 object code 생성 안함
			obj = line_objectcode(tmp);
			if(strcmp(obj, "") == 0){ 
				return -1;
			}
			if(tmp->modified == 1)
				modify[modify_index++] = tmp->loc+1;		
		
			strcpy(tmp->obj, obj);
		}

		free(obj);
		tmp = tmp->link;
	}
	if(strcmp(tmp->str,"END")) //맨 끝에 END로 끝나지 않으면 error
		return -1;
	return 1;
}
//lst 파일을 만드는 함수
void makefile_lst(char* filename){
	char lst_filename[30];
	strcpy(lst_filename,"");
	strncpy(lst_filename, filename, (int)(strlen(filename)-4));
	lst_filename[strlen(filename)-4] = '\0';
	
	strcat(lst_filename, ".lst");
	FILE *fp = fopen(lst_filename,"w");

	LINE* tmp = asm_head->link;
	while(tmp!= NULL){
		if(strcmp(tmp->comment,"")){ //comment 출력
			fprintf(fp,"%-4d\t    \t%s\n",tmp->line,tmp->comment);
		}
		else{
			if(strcmp(tmp->label, "")== 0)
				strcpy(tmp->label, "    ");
			
			if(strcmp(tmp->str, "END") == 0){ //END는 loc 출력 안함
				fprintf(fp,"%-4d\t%-8s%-8s%s\t\n",tmp->line,tmp->label, tmp->str, tmp->operand);
			}
			else if(strcmp(tmp->operand2, "")){ //operand2가 있는 경우
				char tmp_operand[20];
				sprintf(tmp_operand, "%s, %s",tmp->operand, tmp->operand2);
				fprintf(fp,"%-4d\t%04X\t%-8s%-8s%-15s%s\n",tmp->line,tmp->loc,tmp->label,tmp->str,tmp_operand,tmp->obj);

			}
			else if(tmp->loc != -1){
				fprintf(fp,"%-4d\t%04X\t%-8s%-8s%-15s%s\n",tmp->line,tmp->loc,tmp->label,tmp->str,tmp->operand,tmp->obj);
			}
			else
				fprintf(fp,"%-4d\t     \t%-8s%-8s%-15s%s\n",tmp->line, tmp->label, tmp->str, tmp->operand,tmp->obj);
		}
		tmp = tmp->link;
	}
	fclose(fp);

}
//obj 파일을 만드는 함수
void makefile_obj(char *filename){
	char obj_filename[30];
	strcpy(obj_filename,"");
	strncpy(obj_filename, filename, strlen(filename)-4);	
	obj_filename[strlen(filename)-4] = '\0';
	strcat(obj_filename, ".obj");
	FILE *fp = fopen(obj_filename,"w");

	//첫줄 출력
	fprintf(fp,"H%-6s%06X%06X\n",asm_head->name,asm_head->start,asm_head->end-asm_head->start);

	LINE* tmp = asm_head->link;
	int obj_cnt = 0;
	int line_start=-1;
	char objStr[100];
	char printStr[100];

	strcpy(objStr,"");
	while(tmp!= NULL){
		
		//if(strcmp(tmp->obj, "") == 0){ //object code가 없는 경우
		if(strcmp(tmp->str,"RESW") * strcmp(tmp->str,"RESB") == 0){
			if(obj_cnt != 0){
				sprintf(printStr, "T%06X%02X%s",line_start, (int)(strlen(objStr)/2), objStr);
				fprintf(fp,"%s\n",printStr);
				strcpy(objStr, ""); //빈문자열로 초기화
				obj_cnt = 0;
				line_start = -1;
			}
		}
		else{
			if(line_start == -1)
				line_start = tmp->loc;
			if(strcmp(tmp->obj, "")){
				strcat(objStr, tmp->obj);
				obj_cnt++;
			}
			
			if(strlen(objStr) >= 56){
				sprintf(printStr, "T%06X%02X%s",line_start, (int)(strlen(objStr)/2), objStr);
				fprintf(fp,"%s\n",printStr);
				strcpy(objStr, ""); //빈문자열로 초기화
				obj_cnt = 0;
				line_start = -1;
			}
		}
		if( strcmp(tmp->str, "END") == 0)
			break;
		tmp = tmp->link;
	}
	if(strlen(objStr) > 0){
		sprintf(printStr, "T%06X%02X%s",line_start, (int)(strlen(objStr)/2), objStr);
		fprintf(fp,"%s\n",printStr);
	}
	for(int i = 0; i < asm_head->modify_cnt; i++){
		fprintf(fp,"M%06X%02X\n", modify[i], 5);
	}
	if(strcmp(tmp->str, "END") == 0){
		fprintf(fp,"E%06X\n",find_sym_loc(tmp->operand));
	}

	fclose(fp);	
}
// symbol table을 저장해주는 함수
void save_symbol(){
	SYMBOL *tmp, *savetmp;
	savetmp = save_symtab;
	tmp = symbol_head;
	while(tmp!= NULL){
		if(savetmp == NULL){
			SYMBOL* new_node;
			new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
			strcpy(new_node->symbol, tmp->symbol);
			new_node->loc = tmp->loc;
			new_node->link = NULL;
			save_symtab = new_node;
			savetmp = save_symtab;
			tmp = tmp->link;
		}
		else{
			SYMBOL* new_node;
			new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
			strcpy(new_node->symbol, tmp->symbol);
			new_node->loc = tmp->loc;
			new_node->link = NULL;
			savetmp->link = new_node;
			tmp = tmp->link;
			savetmp = savetmp->link;
		}
	}
}
/*
 * symbol 명령어 입력시 실행되는 함수로
 * symtab의 내용을 출력한다
*/
void print_symbol(){
	//정렬을 하기 위해 임시로 사용하는 linked list
 	SYMBOL* sort_sym=NULL;
	SYMBOL *tmp, *savetmp;
	savetmp = sort_sym;
	tmp = save_symtab;

	//sort_sym에 save_symtab의 내용 복사
	while(tmp!= NULL){
		if(savetmp == NULL){
			SYMBOL* new_node;
			new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
			strcpy(new_node->symbol, tmp->symbol);
			new_node->loc = tmp->loc;
			new_node->link = NULL;
			sort_sym = new_node;
			savetmp = sort_sym;
			tmp = tmp->link;
		}
		else{
			SYMBOL* new_node;
			new_node = (SYMBOL*)malloc(sizeof(SYMBOL));
			strcpy(new_node->symbol, tmp->symbol);
			new_node->loc = tmp->loc;
			new_node->link = NULL;
			savetmp->link = new_node;
			tmp = tmp->link;
			savetmp = savetmp->link;
		}
	}
	
	//sort_sym를 알파벳 순으로 정렬
	SYMBOL *sort;
	sort = sort_sym;
	SYMBOL *head = sort_sym;
	while(sort->link != NULL){
		if(strcmp(sort->symbol, sort->link->symbol) > 0){
			tmp = sort->link;
			sort->link = sort->link->link;
			tmp->link = head;
			head = sort = tmp;
			continue;
		}
		sort = sort->link;
	}

	tmp = head;
	while(tmp!= NULL){
		printf("\t%s\t%04X\n",tmp->symbol,tmp->loc);
		tmp = tmp->link;
	}

	//sort_sym를 free해준다
	free_symbol(sort_sym);	
}
//symbol table을 free 하는 함수
void free_symbol(SYMBOL* node){
	if(node == NULL)
		return;

	SYMBOL* temp = node;
	SYMBOL* del;
	while(temp != NULL){
		del = temp;
		temp = temp->link;
		free(del);
	}
}
// asm linked lst를 free 해주는 함수
void free_asm(){
	if(asm_head == NULL)
		return;

	LINE* temp = asm_head->link;
	LINE* del;
	while(temp != NULL){
		del = temp;
		temp = temp->link;
		free(del);
	}
	if(asm_head != NULL)
		free(asm_head);
}
