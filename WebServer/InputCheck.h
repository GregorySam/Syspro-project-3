#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <ctype.h>


bool IsNum(const char *s)
{
    while (*s) {
        if (isdigit(*s++) == 0) return 0;
    }

    return 1;
}



void Print_Usage()
{
  printf("Usage:./myhttpd -p serving_port -c command_port -t num_of_threads -d root_dir\n");
}

void ReadParameters(int argc,char* argv[],int& serving_port,int& command_port,int& number_of_threads)
{
  int option,errno;
  DIR* dir=NULL;



  while ((option = getopt(argc, argv,"p:c:t:d:")) != -1) {  //read parameters
    switch (option) {
      case 'p' :
      if(!IsNum(optarg)){
        Print_Usage();
        exit(EXIT_FAILURE);
      }
      else{
        serving_port=atoi(optarg);
      }
      break;

      case 'c' :
      if(!IsNum(optarg)){
        Print_Usage();
        exit(EXIT_FAILURE);
      }
      else{
        command_port = atoi(optarg);
      }
      break;

      case 't' :
      if(!IsNum(optarg)){
        Print_Usage();
        exit(EXIT_FAILURE);
      }
      else{
        number_of_threads=atoi(optarg);
      }
      break;

      case 'd': dir = opendir(optarg);
      break;
      default: Print_Usage();
      exit(EXIT_FAILURE);
    }
  }
  if(serving_port==-1 || command_port == -1 || number_of_threads==-1 || dir==NULL)
  {
    Print_Usage();
    exit(EXIT_FAILURE);
  }
  if(serving_port==command_port)
  {
    printf("Ports should be different unsigned integers\n");
    exit(EXIT_FAILURE);
  }
  if(number_of_threads<=0)
  {
    printf("Threads should be positive unsigned int\n");
    exit(EXIT_FAILURE);
  }
  if(errno == ENOENT){
    printf("Directory does not exist\n");
    exit(EXIT_FAILURE);
  }
  closedir(dir);

}
