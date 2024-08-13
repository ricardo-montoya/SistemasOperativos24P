#include <stdio.h>
#include "imprime_lib.h"

#define EXIT_SUCCESS 0

int main(int argc, char *argv[])
{
  puts("Imprimendo texto desde Shared Object.\n");
  imprime("Hola Mundooooo.\n");

  return EXIT_SUCCESS;
}
