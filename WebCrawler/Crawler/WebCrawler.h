


#ifndef WEBCRAWLER_H_
#define WEBCRAWLER_H_

#include "ThreadPool/ThreadPool.h"
class WebCrawler
{

  List p;
  pthread_t* Threads;
  time_t start_time;
  int server_socket;
  int command_socket;
  int port;
  char* directory;
  char* host;
  unsigned int number_of_threads;

public:
  WebCrawler(char*,unsigned int,unsigned int,unsigned int,char*);
  void CreateThreads(char*);
  void Communication();
  ~WebCrawler();

};




#endif
