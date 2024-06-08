#include "ackermann.h"
#include "my_allocator.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char ** argv) {

  atexit(release_allocator);

  // input parameters (basic block size, memory length)
  int opt;
  // opt = getopt(int argc, char *const argv[], const char *optstring);
  int basic_block_size;
  int memory_length;
  int bflag = 0;
  int sflag = 0;

  while((opt = getopt(argc, argv, "b:s:")) != -1) 
  { 
    switch(opt) 
    {
      case 'b': 
        bflag++;
        basic_block_size = atoi(optarg);
        break; 
      case 's': 
        sflag++;
        memory_length =  atoi(optarg);
        break; 
    } 
  } 
  for(; optind < argc; optind++)
  { 
    printf("Given extra arguments: %s\n", argv[optind]);
  }

  if (bflag == 0 )
  {
    basic_block_size = 128;
  }
  if (sflag == 0 )
  {
    memory_length = 512*1024;
  }

  // printf("%d \n",basic_block_size);
  // printf("%d \n",memory_length);
  printf("Before initializing...\n");
  
  int x;
  x = init_allocator(basic_block_size, memory_length);
  printf("memory allocated = %d bytes\n",x);
  
  ackermann_main();

  return 0;

}
