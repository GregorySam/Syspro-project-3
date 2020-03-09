#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
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
  printf("Usage: ./mycrawler -h host_or_IP -p port -c command_port -t num_of_threads -d save_dir starting_URL\n");
}

void ReadParameters(int argc,char* argv[],int& serving_port,int& command_port,int& number_of_threads,char*& hs_ip)
{
  int option,errno;
  struct stat st = {0};
  char* folder_name=NULL;



  while ((option = getopt(argc, argv,"h:p:c:t:d:")) != -1) {  //read parameters
    switch (option) {
      case 'h' :
      asprintf(&hs_ip,"%s",optarg);
      break;
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
      case 'd':
      asprintf(&folder_name,"%s",optarg);
      if (stat(folder_name, &st) == -1 && folder_name!=NULL) {
        mkdir(folder_name, 0755);
      }
      break;
      default: Print_Usage();
      exit(EXIT_FAILURE);
    }
  }
  if(serving_port==-1 || command_port == -1 || number_of_threads==-1 )
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
  if(folder_name==NULL)
  {
    Print_Usage();
    exit(EXIT_FAILURE);
  }
  free(folder_name);

}
