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
	dump_print("0000","0100",3);
//	dump_print("4000","4133",3);
//
	//load 시 regL과 PC는 각각 프로그램 길이, 시작주소로 초기화
	regL = csHead[0]->length;
	regPC = csHead[0]->addr;
	
	return 0;
}


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

void printBP(){
	printf("\t\tbreakpoint\n\t\t----------\n");
	for(int i=0;i<bpCnt;i++)
		printf("\t\t%X\n", BP[i]);
	printf("\n");
}

int run(){







}
