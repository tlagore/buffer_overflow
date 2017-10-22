#include <stdlib.h>
#include <unistd.h>

int func();

int main(int argc, char *argv[]){  
  //find 0x7ffff7a01000, 0x7ffff7dd2000, '/', 'b', 'i', 'n', '/', 's', 'h'
  // execve = 0x7ffff7ad6020
  // 0x7ffff7b99673 = /bin/sh

  //char *c1[] = {"sh", NULL};
  //char *c1[] = { (char*)0x7ffff7b99678, NULL };

  //int (*func) (char *, char**, char**) = 0x7ffff7ad6020;

  //execl("/bin/sh", "sh", NULL);//(char*)0x7ffff7b99673, NULL, NULL);//(char**)0x7fffffffdca0, NULL);
  //func((char*)0x7ffff7b99673, (char**)0x7fffffffdca0, NULL);
  //execve((char*)0x7ffff7b99673, c1, c2);
	 //c1, c2);
  
  asm("movq $59, %rax\n\t"
      "movabs $0x7ffff7b99673, %rdi\n\t"
      "movabs $0x7fffffffdca0, %rsi\n\t"
      "movq $0x0, %rdx\n\t"
      "syscall\n\t");
      

      //rdi, rsi, rdx
      //file_name '/bin/sh' , argv ('sh',null), envp (null)

}
