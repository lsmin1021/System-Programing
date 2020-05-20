/*
 * memory.c
 * 메모리 관련 함수들
 *
 * 2020/04/04 v.01
 *
 */


#include "20181666.h"


int mem_addr = 0; //dump 출력 시 사용되는 address


void mem_reset(){ //메모리를 리셋해주는 함수
	// memory 리셋
	for(int i=0;i<16;i++){
		for(int j=0;j<65536;j++){
			memory[i][j] = 0;
		}
	}
	//printf("메모리 리셋 완료!\n");
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
		if(i_end >= MAX_MEMORY)
			i_end = MAX_MEMORY-1;
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
	int flag = 0;
	int line_s, line_e; //해당 줄에 시작 인덱스와 끝 인덱스

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
		printf("hehe %d %s  ",val,value);
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

