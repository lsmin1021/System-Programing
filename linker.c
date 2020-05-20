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

ESTAB** csHead;
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
int setProgaddr(int value){
	PROGADDR = value;
	return 1;
}


/*
 * ESTAB에서 str이 중복되는지 확인하는 함수
 *
 * type에 따라 다르게 판단.
 * type 0인 경우 control section 이름
 * type 1인 경우 symbol name
 *
 * 중복인 경우 0 리턴, 중복 아닌 경우 1 리턴
 */
int check_ESTAB(char* str, int type){


}
/*
 *
 * index = csHead table의 인덱스
 */
int make_ESTAB(char* filename, int* csAddr, int index){
	
	FILE *fp = fopen(filename, "r");
	char readLine[70]; //파일에서 읽어 올 한 줄
//	char csName[7];

	int i;
	int startAddr = 0;
	csHead[index] = (ESTAB*) malloc(sizeof(ESTAB));
			
	//fscanf(fp, "%s", readLine);
	fgets(readLine,70,fp);
	for(i=0; i<6; i++){
		if(readLine[i+1] == ' ') break;
		csHead[index]->csName[i] = readLine[i+1];
	}
	csHead[index]->csName[i] = '\0'; //control section 이름 저장
	
	csHead[index]->addr = *csAddr;

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
	progLength += csHead[index]->length;
	csHead[index]->link = NULL;
	while(1){
		fgets(readLine, 70, fp);
		if(feof(fp)) break; //파일 끝에 도달하면 종료 
	//	printf("%s",readLine);
		int lineIndex = 1;
		if(readLine[0] == 'E') //END에 도달하면 종료
			break;

		if(readLine[0] == 'D'){ //define 라인 처리
			char symTmp[7], addrTmp[7];
			int j;
			
			while(lineIndex < strlen(readLine)){
				for(j = lineIndex; j < lineIndex+6 ; j++){
					if(readLine[j] == ' ') break;
					symTmp[j-lineIndex] = readLine[j];
				}
				symTmp[j-lineIndex] = '\0';
				lineIndex+=6;
				if(check_ESTAB(symTmp, 1) == 0){//symbol 이름이 중복이 아닌 경우
					printf("error! duplicate symbol name!\n");
					return -1; //TODO 에러 처리 할 필요 없을듯????
				}

				for(j = lineIndex; j <lineIndex+6; j++){
					addrTmp[j-lineIndex] = readLine[j];
				}
				lineIndex += 6;
				ESTAB *newNode = (ESTAB*) malloc(sizeof(ESTAB));
				strcpy(newNode->symName, symTmp);
				newNode->addr = hex_to_dec(addrTmp);
				newNode->addr -= startAddr; //TODO 이거 해야하나..?
				newNode->addr += *csAddr;
				newNode->link = NULL;

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

	ESTAB* tmpLink = csHead[index];
	while(tmpLink != NULL){
		printf("%s %s  %X\n",tmpLink->csName, tmpLink->symName, tmpLink->addr);
		tmpLink = tmpLink->link;
	}
	fclose(fp);

	*csAddr += csHead[index]->length;

	return 1;


}


/*
 * ESTAB에서 해당 symbol을 찾아 addr 값을 리턴해줌
 *
 */
int find_symbol_ESTAB(char *symbol){
	for(int i=0;i<estabCnt;i++){
		ESTAB* tmp = csHead[i];
		while(tmp != NULL){
			if(!strcmp(tmp->symName, symbol) || !strcmp(tmp->csName, symbol)){ //같은거 찾으면
				return tmp->addr;
			}
			tmp = tmp->link;
		}
	}
	return -1; //못찾으면 -1 리턴
}


int file_loader(char *filename, int* csAddr, int index){
	FILE *fp = fopen(filename, "r");

	char readLine[70];
	int curAddr = *csAddr; //현재 dump에 쓸 주소
	int *referArr, referFlag = 0;
	
	int prevAddr = -1, prevResult = 0;
	

	while(1){
		fgets(readLine, 70, fp);
		if(feof(fp)) break;
		if(readLine[0] == '.') // . 이면 continue
			continue;
		int lineIndex = 1,j;
		char strTmp[7],numTmp[3], addrStr[3];
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
				if(referArr[tmpCnt] == -1){ //TODO 지우기
					printf("erororororoorr\n",tmpCnt);
				}
				lineIndex = tmpCnt * 8 + 1 +2;
			}
			
			/*for(int p = 0; p<referCnt; p++)
				printf("%04X ",referArr[p]);
			printf("\n");*/
		
		}

		int bitCnt = 0;
		if(readLine[0] == 'T'){ //Text record 처리
			strncpy(strTmp, readLine+1, 6);
			strTmp[6] = '\0';
			curAddr = *csAddr + hex_to_dec(strTmp);
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
			curAddr = *csAddr + hex_to_dec(strTmp); //수정할 주소

		
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
					store = valuetmp[0];
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
				curAddr = *csAddr + hex_to_dec(strTmp);
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
 * register를 0으로 초기화해주는 함수
 *
 */
void resetReg(){
	regA = 0;
	regL = 0;
	regX = 0;
	regB = 0;
	regT = 0;
	regS = 0;
	regPC = 0;
}

/*
 * loader 함수
 *
 * loader [obj file1] [obj file2] [obj file3]입력 시 호출
 * 
 * obj 파일 에러는 고려 안함
 **/
int loader(char* file1, char* file2, char* file3, int cnt){ 
	PROGADDR = hex_to_dec("000");
	int csAddr = PROGADDR;	
	progLength = 0;
	usedBP = -1;
	BP = NULL;
	
	resetReg(); //프로그램 load 시 레지스터 0으로 초기화

	//ESTAB 생성
	csHead = (ESTAB**)malloc(sizeof(ESTAB*) * cnt);
	estabCnt = cnt;
//	if(strcmp(file1, "")){ //같으면?
//	}

	//pass1 ESTAB 생성
	make_ESTAB(file1, &csAddr, 0);
	if(cnt >= 2)
		make_ESTAB(file2, &csAddr, 1);
	if(cnt >= 3)
		make_ESTAB(file3, &csAddr, 2);

	printf("ll = %04X\n",progLength);
	csAddr = PROGADDR;
	file_loader(file1, &csAddr, 0); 
/*	csAddr = csHead[1]->addr;
	file_loader(file2, &csAddr, 1);
	csAddr = csHead[2]->addr;
	file_loader(file3, &csAddr, 2);*/
	dump_print("0000","1080",3);
//	dump_print("4000","4133",3);
//
	//load 시 regL과 PC는 각각 프로그램 길이, 시작주소로 초기화
	regL = csHead[0]->length;
	regPC = csHead[0]->addr;
	
	return 0;
}

//addr에 breakpoint 설정
void breakPoint(char* addr){
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
}
// pc가 breakPoint에 해당하면 1 리턴. 아니면 0 리턴
int isBP(int pc){
	for(int i=0;i<bpCnt;i++){
		if(BP[i] == pc)
			return 1;
	}
	return 0;
}

//breakPoint 출력 
void printBP(){
	printf("\t\tbreakpoint\n\t\t----------\n");
	for(int i=0;i<bpCnt;i++)
		printf("\t\t%X\n", BP[i]);
	printf("\n");
}
// register 목록 출력
void printREG(){
	printf("A : %06X\t X : %06X\n", regA, regX);
	printf("L : %06X\tPC : %06X\n", regL, regPC);
	printf("B : %06X\t S : %06X\n", regB, regX);
	printf("T : %06X\n", regT);
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
//16진수가 음수인지 판단하는 함수. 음수면 해당 int값 리턴
int negative_check(char* hex){
	int cnt = 1, minus = 1;
	while(cnt <= strlen(hex)){
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
	
	char strTmp[6];
	int format = 0, addressing = 0, opcode = 0;
	int pcRelative, baseRelative, indexed;
	char valueTmp[3], addrTmp[6];
	int tmp; //메모리에서 한 바이트 값을 읽어와 저장하는 변수, 임시변수
	int m; //계산 결과
	int reg1, reg2; //format2에서 사용
	int option;
	int cursor = 0;
	
	int CC; // >이면 1, = 이면 0, <이면 -1
	char disp[6]; //disp 5자리
	char address[4]; //address 3자리
//	printREG();
	while(1){
		if(regPC >= progLength) //프로그램 끝에 도달하면 종료
			break;
		if(isBP(regPC) && regPC != usedBP){ //breakpoint이면 종료
			usedBP = regPC;
			break;
		}
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
			reg1 = tmp/16;
			reg2 = tmp%16;
		}
		else{
			option = tmp/16; //xbpe 값
			indexed = option/8; //8로 나눈 값이 1이면 indexed
			baseRelative = (option%8)/4; //baseRelative인 경우
			pcRelative = (option%4) / 2; //pcRelative인 경우
			format = option%2; //format 4인 경우
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
			sprintf(address,"%X",tmp%16);
			address[1] = '\0';
			tmp = memory[cursor%16][cursor/16];
			cursor++;
			sprintf(valueTmp, "%02X", tmp);
			valueTmp[2]='\0';
			strcat(address, valueTmp);
		}
		regPC = cursor; //jump가 아닌 경우 다음 instruction에 pc
	//	printf("opcode = %02X",opcode);
		
		if(format == 0){ //format 3
			m = hex_to_dec(address); //address 값 m에 임시저장
			if(pcRelative == 1){ //pcrelative인 경우  //TODO 음수인경우 고려??
				if(negative_check(address) < 0){
					m = negative_check(address);
				
				}
				m += regPC;
			}
			else if(baseRelative == 1){ //base relative인 경우
				m+= regB;
			}
	//		printf("m = %04X",m);
		}
		else if(format == 1){ //format 4
			m = hex_to_dec(disp);
	//		printf("m - %04X",m);
		}
	//	printf("\n");
		
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
//			printf("%s  %d  %04X<<\n",address, tmp, regPC);
			for(int i=0;i<6;i+=2){
				sprintf(addrTmp, "%X",m++);
				strncpy(valueTmp, strTmp+i, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}
			
			dump_print("0","0040",3);
			dump_print("1030","1080",3);
//			break;	
		}
		else if(opcode == hex_to_dec("68")){ //LDB
			regB = m;
//			break;
		}
		else if(opcode == hex_to_dec("48")){ //JSUB
			regL = regPC;
			regPC = m;
		}
		else if(opcode == hex_to_dec("B4")){ //CLEAR
			setREG(reg1, 0);
		}
		else if(opcode == hex_to_dec("74")){ //LDT
			if(addressing == 1){ //immediate
				regT = m;
			}
		}
		else if(opcode == hex_to_dec("E0")){ //TD
			CC = -1; //CC 는 <
		}
		else if(opcode == hex_to_dec("30")){ //JEQ
			if(CC == 0){ //CC가 =이면 jump
				regPC = m;
			}
		//	break;
		}
		else if(opcode == hex_to_dec("D8")){ //RD
			CC = 0; //CC가 =이라고 가정
//			break;
		}
		else if(opcode == hex_to_dec("A0")){ //COMPR 그냥 넘어가기
			CC = CC;
//			break;
		}
		else if(opcode == hex_to_dec("54")){ //STCH
		//	printf("hhe\n");
			regA = 2143;
			if(indexed == 1){ //x = 1
				sprintf(strTmp,"%02X",regA%(16*16)); //레지스터 A의 값
				sprintf(addrTmp, "%X",(m++) + regX);
				strncpy(valueTmp, strTmp, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}
			break;
		}
		else if(opcode == hex_to_dec("10")){ //STX
			sprintf(strTmp,"%06X",regX); //레지스터 X의 값
//			printf("%s  %d  %04X<<\n",address, tmp, regPC);
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
		//		printf("m = %d  %04X  ",m, m);
				strTmp[0] = '\0';
				for(int i = 0; i<3;i++){
					tmp = memory[m%16][m/16];
					m++;
					sprintf(valueTmp, "%02X",tmp);
					valueTmp[2] = '\0';
					strcat(strTmp, valueTmp);
				}
			//	printf(">>%s  %d<<\n",strTmp, hex_to_dec(strTmp));
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
//			printf("%s  %d  %04X<<\n",address, tmp, regPC);
			for(int i=0;i<6;i+=2){
				sprintf(addrTmp, "%X",m++);
				strncpy(valueTmp, strTmp+i, 2);
				valueTmp[2]='\0';
				mem_edit(addrTmp, valueTmp);
			}

//		break;

		}
		else if(opcode == hex_to_dec("50")){ //LDCH
			if(indexed == 1) //index 사용하는 경우
				m += regX;
			tmp = memory[m%16][m/16];
			regA -= regA%(16*16); //rightmost 바이트 지우기
			regA += tmp; //rightmost 바이트 쓰기
//			break;
		}
		else if(opcode == hex_to_dec("DC")){//WD
			//그냥 넘어가기
		}
		else if(opcode == hex_to_dec("B8")){ //TIXR
			regX += 1;
			if(regX == reg1) CC = 0;
			else if(regX > reg1) CC = 1;
			else CC = -1;
		}
		else if(opcode == hex_to_dec("3C")){
			regPC = m;
		//	printf("dddd 요기%04X\n",regPC);
		}
//		printREG();


	
	}
	printREG();
	printf("\tStop at checkpoint[%X]\n",regPC);
//	dump_print("0","0040",3);
//	dump_print("1030","1080",3);





}
