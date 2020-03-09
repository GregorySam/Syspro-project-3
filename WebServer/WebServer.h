#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <dirent.h>
#include "ThreadPool.h"


class WebServer
{
  fd_set read_fds;
  int client_socket;
  int command_socket;
  pthread_t* Threads;
  List Pool;
  time_t start_time;
  char* pDir;
  unsigned int number_of_threads;

  public:
    WebServer(unsigned int ,unsigned int,unsigned int,char*);
    char* GetRootDirectory();
    int GetClientSocket();
    void CreateThreads();
    void Communication();
    ~WebServer();
    void Exit();

};

#endif /*WEBSERVER_H_*/
