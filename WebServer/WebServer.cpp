#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "WebServer.h"
#include "ThreadFunctions.h"



WebServer::WebServer(unsigned int s_p,unsigned int c_p,unsigned int n_threads,char* dir_name):number_of_threads(n_threads)
{

    int socket_desc1,socket_desc2,c;
    struct sockaddr_in client,commands;



    start_time=time(NULL);

    asprintf(&pDir,"%s",dir_name);
    Threads=(pthread_t*)malloc(n_threads*sizeof(pthread_t));


    socket_desc1 = socket(AF_INET , SOCK_STREAM , 0);
    socket_desc2 = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc1 == -1 || socket_desc2 == -1)
    {
        printf("Could not create socket\n");
    }
    puts("Sockets created\n");

    //Client Socket///////////////////////////
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = INADDR_ANY;
    client.sin_port = htons( s_p );

    /////////Command Socket////////////////////////////////////////////////
    commands.sin_family = AF_INET;
    commands.sin_addr.s_addr = INADDR_ANY;
    commands.sin_port = htons( c_p );

    //Bind
    if(bind(socket_desc1,(struct sockaddr *)&client , sizeof(client))  || bind(socket_desc2,(struct sockaddr *)&commands , sizeof(commands)))
    {
        //print the error message
        perror("bind failed. Error\n");

    }
    puts("bind done\n");

    puts("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);


    //Listen
    listen(socket_desc1 , 3);
    //Listen
    listen(socket_desc2 , 3);

     //accept connection from an incoming client
      puts("Waiting for client connection...\n");
     client_socket = accept(socket_desc1, (struct sockaddr *)&client, (socklen_t*)&c);
     if (client_socket < 0)
     {
         perror("Client Accept failed\n");
     }
     puts("Connection to Client accepted\n");


     puts("Waiting for command connection...\n");
     command_socket = accept(socket_desc2, (struct sockaddr *)&commands, (socklen_t*)&c);
     if (command_socket < 0)
     {
         perror("Command Accept failed\n");
     }
     puts("Connection to Commands accepted\n");

}


WebServer::~WebServer()
{

  free(Threads);

  free(pDir);
}
int WebServer::GetClientSocket()
{
  return client_socket;
}


char* WebServer::GetRootDirectory()
{
  return pDir;

}


Args* A;

void WebServer::CreateThreads()
{
   pthread_t thr;
   int err;

   A=(Args*)malloc(sizeof(Args));

   A->parent_dir=&pDir;
   A->p=&Pool;
   A->pages_served=0;
   A->bytes_sent=0;
   A->fd=client_socket;

   printf("Creating Threads\n");fflush(stdout);

   pthread_mutex_init(&mtx, 0);
   pthread_cond_init(&cond_nonempty, 0);
   pthread_cond_init(&cond_nonfull, 0);
   for(unsigned int i=0;i<number_of_threads;i++)
   {
     if ((err = pthread_create(&thr, NULL, Client_Communication, A))){
       printf("pthread_create"); exit(1);
     }
      Threads[i]=thr;
   }

}


int Max(int a,int b)
{
  if(a>b){return a;}
  else{return b;}
}



void WebServer::Exit()
{

  for(unsigned int i=0;i<number_of_threads;i++)
  {

    place(&Pool,NULL);
    pthread_cond_signal(&cond_nonempty);
   }
   for(unsigned int i=0;i<number_of_threads;i++)
   {
     printf("Thread %ld is exiting\n",Threads[i]);fflush(stdout);
     pthread_join(Threads[i],NULL);
     printf("Thread %ld exited\n",Threads[i]);fflush(stdout);
   }
   free(A);
   close(client_socket);
   close(command_socket);
   pthread_cond_destroy(&cond_nonempty);
   pthread_cond_destroy(&cond_nonfull);
   pthread_mutex_destroy(&mtx);
}

void WebServer::Communication()
{
  int maxfd=Max(command_socket,client_socket),retval;
  char command[9],*file,*mess;



  for(;;)
  {
    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);
    FD_SET(command_socket, &read_fds);
    retval = select(maxfd+1, &read_fds, NULL, NULL, NULL);

    if(retval>0)
    {
      if (FD_ISSET(client_socket, &read_fds))
       {
          file=Get_Request(client_socket);
          if(file==NULL)
          {
            printf("Connetion closed\n");
            Exit();
            break;
          }

           place(&Pool,file);
           pthread_cond_signal(&cond_nonempty);
           free(file);
       }
       if (FD_ISSET(command_socket, &read_fds))
       {
         if(read(command_socket,command,9)>0)
         {
           command[strlen("SHUTDOWN")]='\0';
           if(strcmp(command,"SHUTDOWN")==0)
           {
             Exit();
             break;
           }
           else
           {
              command[strlen("STATS")]='\0';
            if(strcmp(command,"STATS")==0)
             {
               time_t end = time(NULL);
               asprintf(&mess,"Server up for %ld seconds,served %u pages, %u bytes\n",end-start_time,A->pages_served,A->bytes_sent);fflush(stdout);
               write(command_socket,mess,strlen(mess));
               free(mess);
             }
           }

         }
       }
    }
    else
    {
      printf("Error");
    }

  }

}
