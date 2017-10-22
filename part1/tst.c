#include <stdlib.h>

int func();


int main(){  
  //value of system in a.out: 0x7ffff7a48070
  //char * val;
  //val = (char*)0x7ffff7b99673;
  //system(val);
  //int (*ptr)(char*) = 0x7ffff7a48070;
  //0x7ffff7a48070;
  //ptr(val);

  int (*func) (char *) = 0x7ffff7a48070;
  func(0x7ffff7b99673);

  /* address of exit()*/
  //int (*func2) (int) = 0x7FFFF7AD5FC0;
  //func2(0);

  
  //  char hello[] = "hello";
  //  system(hello);
  
  /*
    asm("nop\n\t"
	"movq $0x7ffff7b99673, %rax\n\t"
	"mov %rax,%rdi\n\t"
	"push %rax\n\t"
	"movabs $0x7ffff7a47ae0, %rax\n\t" // this is the address for do_system
	//"movabs $0x7ffff7a48070, %rax\n\t"  //this is the address for sysem	
	"call *%rax\n\t");
  */

}
