#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
* Diego R Montoya B // 2213002014
* Sistemas Operativos 24P
*/

int main(int argc, char *argv[])
{
  if(argc != 2){
      printf("Debes pasar como parametro unicamente un valor Long\n");
      printf(" ->   ./ %s 10000000000\n", argv[0]);
      return 0;
  }

  long iterations = atol(argv[1]);

  printf("Started\n");

  register long m = 0;
  clock_t reg_time = clock();
  for(;m < iterations; m++);
  reg_time = clock() - reg_time;


  double reg_time_in_secs = (double) reg_time / CLOCKS_PER_SEC;

  printf("Finished\n");
  printf("Elapsed time -> %f\n", reg_time_in_secs);
  

  return 0;
}

