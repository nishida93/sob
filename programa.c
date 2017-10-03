#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv) {

  if(strcmp(argv[1], "c") == 0) {
    printf("\n\nString que sera encriptada: %s.\n\n", argv[2]);
  }

  if(strcmp(argv[1], "d") == 0) {
    printf("\n\nString que sera decriptada: %s.\n\n", argv[2]);
  }

  if(strcmp(argv[1], "h") == 0) {
    printf("\n\nHash gerado a partir da string: %s.\n\n", argv[2]);
  }

  return 0;
}
