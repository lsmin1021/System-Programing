/*
 *  sp prog4
 *
 *  loader 및 linker
 *  20181666 이승민
 * 
 * 2020/05/24 v.01
 */


#include "20181666.h"

int PROGADDR = 0; //맨 처음 progaddr는 0x00 주소로 지정

int CSADDR1, CSADDR2, CSADDR3;

typedef struct ESTAB_{
	char csName[7]; //control section 이름
	char symName[7]; //symbol name 
	int addr;
	int length;
	struct ESTAB_* link;
}ESTAB;

ESTAB** csHead = NULL;
int estabCnt = 0;
int progLength = 0;


int usedBP = -1;
int *BP;
int bpCnt = 0;

int regA, regL, regX, regB, regT, regS, regPC;

/*
 * progaddr 명령어 입력 시 호출
 *
 * 범위에 맞는지 체크 후 맞으면 progaddr 값바꾸고 1 리턴
 * 범위 벗어나면 -1 리턴
 *
 */
int setProgaddr(char* value){
	if(is_hex(value) == 1){
		if(hex_to_dec(value) >= 0 && hex_to_dec(value) <= MAX_MEMORY){
			PROGADDR = hex_to_dec(value); //PROGADDR 설정
			return 1;
		}
		else{
			printf("error! address must be in memory domain\n");
			return -1;
		}
	}
	else { //value가 16진수가 아니면 에러처리
		printf("error! address must be hex\n");	
		return -1;
	}
}

/*
 * filename에 대해 ESTAB을 생성하는 함수
 * 에러 발생 시 -1 리턴
 * index = csHead table의 인덱스
 */
int make_ESTAB(char* filename, int* csAddr, int index){
	
	FILE *fp = fopen(filename, "r");
	if(fp == NULL){
		printf("error! there is no %s\n", filename);
		return -1;
	}

	char readLine[70]; //파일에서 읽어 올 한 줄

	int i;
	int startAddr = 0;
	csHead[index] = (ESTAB*) malloc(sizeof(ESTAB));
			
	fgets(readLine,70,fp);
	for(i=0; i<6; i++){
		if(readLine[i+1] == ' ') break;
		csHead[index]->csName[i] = readLine[i+1];
	}
	csHead[index]->csName[i] = '\0'; //control section 이름 저장
	
	csHead[index]->addr = *csAddr; //주소값 저장

	char strTmp[10];
	for(i = 7; i<13; i++){
		strTmp[i-7] = readLine[i];
	}
	strTmp[6] = '\0';
	startAddr = hex_to_dec(strTmp); //시작 주소

	for(i = 13; i<19; i++){
		strTmp[i-13] = readLine[i];
	}
	strTmp[6] = '\0';
	csHead[index]->length = hex_to_dec(strTmp); //section 길이

	progLength += csHead[index]->length; //프로그램 전체 길이에 더하기
	csHead[index]->link = NULL;

	while(1){
		fgets(readLine, 70, fp);
		if(feof(fp)) break; //파일 끝에 도달하면 종료 
		if(readLine[0] == 'E') break; //END에 도달하면 종료
			
		int lineIndex = 1;
		if(readLine[0] == 'D'){ //define 라인 처리
			char symTmp[7], addrTmp[7];
			int j;
			
			while(lineIndex+1 < (int)strlen(readLine)){
//				printf("lineIndex = %d  readLine = %d  ", lineIndex, strlen(readLine));
				for(j = lineIndex; j < lineIndex+6 ; j++){
					if(readLine[j] == ' ') break;
					symTmp[j-lineIndex] = readLine[j];
				}
				symTmp[j-lineIndex] = '\0';
				lineIndex+=6;
				if(is_duplicate_ESTAB(symTmp) == 1){
					printf("error! duplicate symbol!\n");
					return -1;
				}
				for(j = lineIndex; j <lineIndex+6; j++){
					addrTmp[j-lineIndex] = readLine[j];
				}
				lineIndex += 6;
				ESTAB *newNode = (ESTAB*) malloc(sizeof(ESTAB));
				strcpy(newNode->symName, symTmp); //symbol 이름 저장
				newNode->addr = hex_to_dec(addrTmp);
				newNode->addr -= startAddr; //이건 안해도 될 듯 
				newNode->addr += *csAddr; //주소 구하기
				newNode->link = NULL;
				//table에 추가
				if(csHead[index]->link == NULL){
					csHead[index]->link = newNode;
				}
				else{
					ESTAB* tmpLink = csHead[index];
					while(tmpLink ->link != NULL)
						tmpLink = tmpLink->link;
					tmpLink->link = newNode;

				}
			}
		}
	}

	fclose(fp);

	*csAddr += csHead[index]->length;

	return 1;
}

/*
 * ESTAB에서 입력된 symbol에 대해
 * 중복된 symbol을 찾으면 1 리턴 아니면 0 리턴
 */
int is_duplicate_ESTAB(char *symbol){
	for(int i=0;i<estabCnt;i++){
		ESTAB* tmp = csHead[i];
		while(tmp != NULL){
			if(!strcmp(tmp->symName,symbol)||!strcmp(tmp->csName,symbol)){ //같은거찾으면
				return 1;
			}
			tmp = tmp->link;
		}
	}
	return 0;
}

/*
 * ESTAB에서 해당 symbol을 찾아 addr 값을 리턴해줌
 * 못찾으면 -1 리턴
 */
int find_symbol_ESTAB(char *symbol){
	for(int i=0;i<estabCnt;i++){
		ESTAB* tmp = csHead[i];
		while(tmp != NULL){
			if(!strcmp(tmp->symName,symbol)||!strcmp(tmp->csName,symbol)){ //같은거찾으면
				return tmp->addr;
			}
			tmp = tmp->link;
		}
	}
	return -1; //못찾으면 -1 리턴
}
/*
 * csHead free해주는 함수
 */
void free_ESTAB(){
	if(csHead == NULL)
		return ;
	ESTAB* rm;
	for(int i=0;i<estabCnt;i++){
		ESTAB* tmp = csHead[i];
		while(tmp != NULL){
			rm = tmp;
			tmp = tmp->link;
			free(rm);
		}
	}
	free(csHead);
	csHead = NULL;
}

/* 
 * filename에 대해 load 작업 진행
 * csAddr은 현재 파일 load될 위치, index는 csHead의 인덱스
 */
int file_loader(char *filename, int csAddr, int index){
	FILE *fp = fopen(filename, "r");
		
	char readLine[70];
	int curAddr = csAddr; //현재 dump에 쓸 주소
	int *referArr, referFlag = 0; //reference line 처리
	
	int prevAddr = -1, prevResult = 0; //이전과 같은 주소에 대한 계산이면 이전 값 이용
	
	while(1){
		fgets(readLine, 70, fp);
		if(feof(fp)) break;
		if(readLine[0] == '.') // . 이면 continue
			continue;
		int lineIndex = 1, j;
		char strTmp[7], numTmp[3], addrStr[3];
		if(readLine[0] == 'R'){ //reference number 처리
			referFlag = 1; //reference가 있는 경우 flag 설정
			int referCnt = (strlen(readLine)-1)/8 + 1; //reference 개수(+1은 자기 자신)
			referArr = (int*)malloc(sizeof(int)*referCnt);
			referArr[0] = find_symbol_ESTAB(csHead[index]->csName);
			int tmpCnt = 0;
			lineIndex = 3; //3열부터 시작
			while(tmpCnt < referCnt){
				int k=0;
				for(k=0;k<6;k++){
					if(readLine[lineIndex] == ' ' || readLine[lineIndex] == '\n')
						break;
					strTmp[k] = readLine[lineIndex++];
				}
				strTmp[k] = '\0';
				referArr[++tmpCnt] = find_symbol_ESTAB(strTmp);
				if(referArr[tmpCnt] == -1){ //symbol 못찾으면 에러처리 근데 이런경우 없음
					printf("error! there is no symbol!!\n");
					return -1;
				}
				lineIndex = tmpCnt * 8 + 1 + 2;// R : 1, 숫자 : 2
			}
		}

		int bitCnt = 0;
		if(readLine[0] == 'T'){ //Text record 처리
			strncpy(strTmp, readLine+1, 6);
			strTmp[6] = '\0';
			curAddr = csAddr + hex_to_dec(strTmp);
			strncpy(strTmp, readLine+7, 2);
			strTmp[2] = '\0';
			bitCnt = hex_to_dec(strTmp) * 2; //한 줄에 byte 수
			
			for(j = 9; j <= bitCnt+9; j+=2){
				strncpy(strTmp, readLine+j,2);
				strTmp[2] = '\0';
				sprintf(addrStr, "%X", curAddr);
				mem_edit(addrStr, strTmp);
				curAddr++;
			}
		}
		
		if(readLine[0] == 'M'){
			
			strncpy(strTmp, readLine+1, 6); //modify 주소
			strTmp[6] = '\0';
			curAddr = csAddr + hex_to_dec(strTmp); //수정할 주소
		
			strncpy(numTmp, readLine+7, 2); //바이트 수
			numTmp[2] = '\0';
			if(referFlag == 1){ //reference number 사용
				char value[7], valuetmp[3]; //주소에 해당하는 수정할 값
				char store;
				int result, flag = 0;
				int add; //value에 더해줄 값

				if(hex_to_dec(numTmp) == 5){ //5바이트 수정
					flag = 1; //5바이트인 경우 주소값 조정
				}
				if(prevAddr == curAddr){ //같은 주소에 대해 연속적인 계산인 경우
					result = prevResult; //이전 결과값 가져와서 계속 계산
				}
				else{ //메모리에서 해당 주소 값 가져오기
					strcpy(value,"");
					sprintf(valuetmp,"%02X",memory[curAddr%16][curAddr/16]);
					store = valuetmp[0]; //맨 앞  바이트 따로 저장
					strcat(value, valuetmp+flag);
					curAddr++;
					sprintf(valuetmp,"%02X",memory[curAddr%16][curAddr/16]);
					strcat(value, valuetmp);
					curAddr++;
					sprintf(valuetmp,"%02X",memory[curAddr%16][curAddr/16]);
					strcat(value, valuetmp);
					value[6-flag] = '\0';
					result = hex_to_dec(value);
				}
				strncpy(numTmp, readLine+10, 2); //referArr 인덱스 구하기
				numTmp[2] = '\0';
				add = referArr[hex_to_dec(numTmp)-1];
				if(readLine[9] == '-') //빼기 면 음수로 바꿔주기
					add *= -1;
				result += add; //값 수정 결과
				if(flag == 1){
					sprintf(value,"%c%05X",store,result); //결과 값 16진수로 바꿔서 저장
				}
				else{
					sprintf(value, "%06X",result);
				}
				curAddr = csAddr + hex_to_dec(strTmp);
				prevResult = result;
				prevAddr = curAddr;
				
				int minusFlag = 0;
				if(strlen(value) == 7){ //7자리 인 경우 앞에 1 제거해주기 위해 flag
					minusFlag = 1;
				}
				for(int k=0;k<3;k++){
					valuetmp[0] = value[k*2+minusFlag];
					valuetmp[1] = value[k*2+1+minusFlag];
					valuetmp[2] = '\0';
					sprintf(addrStr, "%02X", curAddr);
					mem_edit(addrStr, valuetmp);
					curAddr++;
				}
			}
		}
	}
	fclose(fp);	
	return 0;
}


/*
 * loader 함수
 *
 * loader [obj file1] [obj file2] [obj file3]입력 시 호출
 * 
 * obj 파일 에러는 고려 안함
 **/
int loader(char* file1, char* file2, char* file3, int cnt){ 
	int csAddr = PROGADDR; //loading될 주소	
	progLength = 0;
	usedBP = -1;
	BP = NULL;

	resetREG(); //프로그램 load 시 레지스터 0으로 초기화

	//ESTAB 생성
	csHead = (ESTAB**)malloc(sizeof(ESTAB*) * cnt);
	estabCnt = cnt;
	for(int i=0;i<estabCnt;i++){
		csHead[i] = NULL;
	}
	//pass1 ESTAB 생성
	if(make_ESTAB(file1, &csAddr, 0) == -1) return -1;
	if(cnt >= 2)
		if(make_ESTAB(file2, &csAddr, 1) == -1) return -1;
	if(cnt >= 3)
		if(make_ESTAB(file3, &csAddr, 2) == -1) return -1;;

	csAddr = PROGADDR;
	if(file_loader(file1, csAddr, 0) == -1) return -1; 
	if(cnt >= 2){
		csAddr = csHead[1]->addr;
		if(file_loader(file2, csAddr, 1) == -1) return -1;
	}
	if(cnt >= 3){
		csAddr = csHead[2]->addr;
		if(file_loader(file3, csAddr, 2) == -1) return -1;
	}
	//load 시 regL과 PC는 각각 프로그램 길이, 시작주소로 초기화
	regL = progLength;
	regPC = csHead[0]->addr;
	
	printf("control\tsymbol\taddress\tlength\n");
	printf("section\tname\t\n");
	printf("------------------------------------\n");
	for(int i=0;i <estabCnt; i++){
		ESTAB* tmp = csHead[i];
		printf("%s\t\t%04X\t%04X\n",tmp->csName, tmp->addr, tmp->length);
		tmp = tmp->link;
		while(tmp != NULL){
			printf("\t%s\t%04X\n",tmp->symName, tmp->addr);
			tmp = tmp->link;
		}
	}	
	printf("------------------------------------\n");
	printf("\t   total length %04X\n",progLength);

	return 1;
}

//bp 관련 함수들

//addr에 breakpoint 설정
int breakPoint(char* addr){
	int value;
	value = hex_to_dec(addr); //break point 주소값

	if(BP == NULL){
		BP = (int*)malloc(sizeof(int));
		BP[bpCnt++] = value;
	}
	else{
		BP = realloc(BP,sizeof(int) * (bpCnt));
		BP[bpCnt++] = value;
	}

	printf("\t\t [ok] create breakpoint %s\n",addr);
	return 1;
}
// pc가 breakPoint에 해당하면 1 리턴. 아니면 0 리턴
int isBP(int pc){
	for(int i=0;i<bpCnt;i++){
		if(BP[i] == pc)
			return 1;
	}
	return 0;
}

// 설정 된 breakpoint를 초기화하는 함수
void clearBP(){
	if(BP != NULL){
		free(BP);
		BP = NULL;
	}
	bpCnt = 0;
}

//breakPoint 출력 
void printBP(){
	printf("\t\tbreakpoint\n\t\t----------\n");
	if(bpCnt == 0)
		printf("\t\tthere is no breakpoint\n");
	for(int i=0;i<bpCnt;i++)
		printf("\t\t%X\n", BP[i]);
	printf("\n");
}


//register 관련 함수들

/*
 * register를 0으로 초기화해주는 함수
 *
 */
void resetREG(){
	regA = 0;
	regL = 0;
	regX = 0;
	regB = 0;
	regT = 0;
	regS = 0;
	regPC = 0;
}
//regNum에 해당하는 register를 value로 설정
void setREG(int regNum, int value){
	switch(regNum){
		case 0: regA = value;
				break;
		case 1: regX = value;
				break;
		case 2: regL = value;
				break;
		case 3: regB = value;
				break;
		case 4: regS = value;
				break;
		case 5: regT = value;
				break;
		case 8: regPC = value;
				break;
		default: break;
	}
}
//regNum에 해당하는 register 값 return
int getREG(int regNum){
	switch(regNum){
		case 0: return(regA);
				break;
		case 1: return(regX);
				break;
		case 2: return(regL);
				break;
		case 3: return(regB);
				break;
		case 4: return(regS);
				break;
		case 5: return(regT);
				break;
		case 8: return(regPC);
				break;
		default: break;
	}
	return -1; //해당 register가 없으면 -1 리턴
}
// register 목록 출력
void printREG(){
	printf("A : %06X\t X : %06X\n", regA, regX);
	printf("L : %06X\tPC : %06X\n", regL, regPC);
	printf("B : %06X\t S : %06X\n", regB, regS);
	printf("T : %06X\n", regT);
}

//16진수가 음수인지 판단하는 함수. 음수면 해당 int값 리턴
int negative_check(char* hex){
	int cnt = 1, minus = 1;
	while(cnt <= (int) strlen(hex)){
		minus *= 16;
		cnt++;
	}

	if(hex[0] >='8' && hex[0] <= '9')
		return hex_to_dec(hex)-minus;
	else if(hex[0] >= 'A' && hex[0] <= 'F')
		return hex_to_dec(hex)-minus;

	else return 0;
}
int run(){
	int format = 0, addressing = 0, opcode = 0;
	int pcRelative, baseRelative, indexed;
	char valueTmp[3], addrTmp[6], strTmp[6];
	
	int tmp; //메모리에서 한 바이트 값을 읽어와 저장하는 변수, 임시변수
	int m; //계산 결과
	int reg1; //format2에서 사용
	int xbpe; //xbpe 값
	int cursor = 0; //memory 접근 시 사용하는 인덱스값
	
	int CC; // >이면 1, = 이면 0, <이면 -1
	char disp[6]; //disp 5자리
	char address[4]; //address 3자리
	while(1){
		if(regPC >= progLength) //프로그램 끝에 도달하면 종료
			break;
		if(isBP(regPC) && regPC != usedBP){ //breakpoint이면 종료
			usedBP = regPC;
			break;
		}
		if(usedBP == regPC) //반복문에서 여러번 break 되는 경우 고려
			usedBP = -1;

		strcpy(disp,""); //초기화
		strcpy(address, "");

		cursor = regPC;
		tmp = memory[cursor%16][cursor/16];
		cursor++;

		opcode = tmp/4; //opcode 저장
		opcode *= 4;
		addressing = tmp % 4; //addressing 방식 저장
		// 3이면 simple, 2면 indirect, 1이면 immediate, 0이면 sic(아마 format2)
	
		tmp = memory[cursor%16][cursor/16];
		cursor++;

		if(addressing == 0){// format 2
			format = 2;
			reg1 = tmp/16; //reg1 번호
			//reg2 = tmp%16; //reg2 번호
		}
		else{
			xbpe = tmp/16; //xbpe 값
			indexed = xbpe/8; //8로 나눈 값이 1이면 indexed
			baseRelative = (xbpe%8)/4; //baseRelative인 경우
			pcRelative = (xbpe%4) / 2; //pcRelative인 경우
			format = xbpe%2; //format 4인 경우
		}
		if(format == 1){ //format 4
			//disp 구하기
			sprintf(disp,"%X",tmp%16);
			disp[1]='\0';
			tmp = memory[cursor%16][cursor/16];
			cursor++;
			sprintf(valueTmp, "%02X", tmp);
			valueTmp[2]='\0';
			strcat(disp, valueTmp);

			tmp = memory[cursor%16][cursor/16];
			cursor++;
			sprintf(valueTmp, "%02X", tmp);
			valueTmp[2]='\0';
			strcat(disp, valueTmp);
		}
		else if(format == 0){ //format 3
			//address 구하기
			sprintf(address,"%X",tmp%16);
			address[1] = '\0';
			tmp = memory[cursor%16][cursor/16];
			cursor++;
			sprintf(valueTmp, "%02X", tmp);
			valueTmp[2]='\0';
			strcat(address, valueTmp);
		}
		regPC = cursor; //jump가 아닌 경우 다음 instruction에 pc
		
		if(format == 0){ //format 3
			m = hex_to_dec(address); //address 값 m에 임시저장
			if(pcRelative == 1){ //pcrelative인 경우 
				if(negative_check(address) < 0){ //음수인 경우
					m = negative_check(address);
				
				}
				m += regPC;
			}
			else if(baseRelative == 1){ //base relative인 경우
				m+= regB;
			}
		}
		else if(format == 1){ //format 4
			m = hex_to_dec(disp);
		}
		
		if(addressing == 2){ //indirect addressing
			strTmp[0] = '\0';
			for(int i = 0; i<3;i++){
				tmp = memory[m%16][m/16];
				m++;
				sprintf(valueTmp, "%02X",tmp);
				valueTmp[2] = '\0';
				strcat(strTmp, valueTmp);
			}
			m = hex_to_dec(strTmp);
		}

		//instruction set 구현
		if( opcode == hex_to_dec("14")){ //STL
			sprintf(strTmp,"%06X",regL); //레지스터 L의 값
			for(int i=0;i<6;i+=2){
				sprintf(addrTmp, "%X",m++);
				strncpy(valueTmp, strTmp+i, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}
		}
		else if(opcode == hex_to_dec("68")){ //LDB
			if(addressing == 1) //immediate addressing
				regB = m;
		}
		else if(opcode == hex_to_dec("48")){ //JSUB
			regL = regPC;
			regPC = m;
		}
		else if(opcode == hex_to_dec("B4")){ //CLEAR
			setREG(reg1, 0);
		}
		else if(opcode == hex_to_dec("74")){ //LDT
			if(addressing == 1){ //immediate addressing
				regT = m;
			}
			else if(addressing == 3){ //simple addressing
				strTmp[0] = '\0';
				for(int i = 0; i<3;i++){
					tmp = memory[m%16][m/16];
					m++;
					sprintf(valueTmp, "%02X",tmp);
					valueTmp[2] = '\0';
					strcat(strTmp, valueTmp);
				}
				regT = hex_to_dec(strTmp);
			}
		}
		else if(opcode == hex_to_dec("E0")){ //TD
			CC = -1; //CC 는 <
		}
		else if(opcode == hex_to_dec("30")){ //JEQ
			if(CC == 0){ //CC가 =이면 jump
				regPC = m;
			}
		}
		else if(opcode == hex_to_dec("D8")){ //RD
			CC = 0; //CC가 =이라고 가정
		}
		else if(opcode == hex_to_dec("A0")){ //COMPR 그냥 넘어가기
			CC = CC;
		}
		else if(opcode == hex_to_dec("54")){ //STCH
			regA = 2143;
			if(indexed == 1){ //x = 1
				sprintf(strTmp,"%02X",regA%(16*16)); //레지스터 A의 값
				sprintf(addrTmp, "%X",(m++) + regX);
				strncpy(valueTmp, strTmp, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}
		}
		else if(opcode == hex_to_dec("10")){ //STX
			sprintf(strTmp,"%06X",regX); //레지스터 X의 값
			for(int i=0;i<6;i+=2){
				sprintf(addrTmp, "%X",m++);
				strncpy(valueTmp, strTmp+i, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}

		}
		else if(opcode == hex_to_dec("4C")){ //RSUB
			regPC = regL;
		}
		else if(opcode == hex_to_dec("00")){//LDA
			if(addressing == 1){ //immediate
				regA = m;
			}
			else{
				//m 주소에 접근하여 값 A에 넣기
				strTmp[0] = '\0';
				for(int i = 0; i<3;i++){
					tmp = memory[m%16][m/16];
					m++;
					sprintf(valueTmp, "%02X",tmp);
					valueTmp[2] = '\0';
					strcat(strTmp, valueTmp);
				}
				regA = hex_to_dec(strTmp);
			}
		}
		else if(opcode == hex_to_dec("28")){ //COMP
			if(addressing == 1) { //immediate addressing
				if(regA == m) CC = 0;
				else if(regA > m) CC = 1;
				else CC = -1;
			}
		}
		else if(opcode == hex_to_dec("0C")){ //STA
			sprintf(strTmp,"%06X",regA); //레지스터 A의 값
			for(int i=0;i<6;i+=2){
				sprintf(addrTmp, "%X",m++);
				strncpy(valueTmp, strTmp+i, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}
		}
		else if(opcode == hex_to_dec("50")){ //LDCH
			if(indexed == 1) //index 사용하는 경우
				m += regX;
			tmp = memory[m%16][m/16];
			regA -= regA%(16*16); //rightmost 바이트 지우기
			regA += tmp; //rightmost 바이트 쓰기
		}
		else if(opcode == hex_to_dec("DC")){//WD
			//그냥 넘어가기
		}
		else if(opcode == hex_to_dec("B8")){ //TIXR
			regX++;
			if(regX == getREG(reg1)) CC = 0;
			else if(regX > getREG(reg1)) CC = 1;
			else CC = -1;
		}
		else if(opcode == hex_to_dec("38")){ //JLT
			if(CC == -1){ //CC가 <이면 jump
				regPC = m;
			}
		}
		else if(opcode == hex_to_dec("3C")){ //J
			regPC = m;
		}
		else{
			printf("error!");
			return -1;
		}
	}
	printREG();
	if(regPC >= progLength)
		printf("\tEnd Program\n");
	else
		printf("\tStop at checkpoint[%X]\n",regPC);

	return 1;
}
