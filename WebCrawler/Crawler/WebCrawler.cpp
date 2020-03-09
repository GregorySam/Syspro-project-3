
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "WebCrawler.h"
#include "ThreadPool/ThreadFunctions.h"






void ClearString(char* &ret,size_t size)
{
  int j;

  for(j=size-1;j>=0;j--)
  {
    if(ret[j]<32 || ret[j]>127)
    {
      ret[j]='\0';
    }
  }



}

char* ReadFromTelnet(int fd)
{
  size_t size=0;
  char buff[512],*ret;
  int n;

    do {
      if((n = read(fd, buff+size, 512-size)) < 0) {
          printf("messages.socketreadfail");
          exit(0);
      }
      size += n;

    } while(strchr(buff, '\n') == NULL && size < 512);

    ret=(char*)malloc(size+1);

    memcpy(ret,buff,size);
    ClearString(ret,size);


    return ret;

}



WebCrawler::WebCrawler(char* hostname,unsigned int p,unsigned int command_port,unsigned int threads_num,char* save_dir)
{

    int sock,sock2,c;
    struct sockaddr_in server,commands;
    struct hostent *rem;

    asprintf(&host,"%s",hostname);
    free(hostname);
    //Create socket////////////////////////////////////////////
    sock = socket(AF_INET , SOCK_STREAM , 0);
    sock2 = socket(AF_INET , SOCK_STREAM , 0);

    if (sock == -1 || sock2==-1)
    {
        printf("Could not create socket");
        exit(EXIT_FAILURE);
    }
    //Connect to remote server///////////////////////////////////////
    if ((rem = gethostbyname(host)) == NULL) {
	   herror("gethostbyname");
     exit(EXIT_FAILURE);
    }
    port=p;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);


    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        exit(EXIT_FAILURE);
    }
    else
    {
          puts("Connected to server\n");
    }
    server_socket=sock;
    /////////////////////////////////////////////////////////


    ////Connect to Commands////////////////////////////////
    puts("Waiting for command connection...\n");
    commands.sin_family = AF_INET;
    commands.sin_addr.s_addr = INADDR_ANY;
    commands.sin_port = htons( command_port);

    c = sizeof(struct sockaddr_in);
    if(bind(sock2,(struct sockaddr *)&commands , sizeof(commands)))
    {
      perror("Bind Error");
      exit(EXIT_FAILURE);
    }


    listen(sock2 , 3);

    command_socket = accept(sock2, (struct sockaddr *)&commands, (socklen_t*)&c);
    if (command_socket < 0)
    {
        perror("Command Accept failed\n");
        exit(EXIT_FAILURE);

    }
    puts("Connection to Commands accepted\n");
    ///////////////////////////////////////////


    number_of_threads=threads_num;
    Threads=(pthread_t*)malloc(number_of_threads*sizeof(pthread_t));
    asprintf(&directory,"%s",save_dir);
  ////////////////////////////////////////////////////////////////////////

}

Args* A;

void WebCrawler::CreateThreads(char* start_url)
{
   pthread_t thr;
   int err;
   FILE* fp;

   start_time=time(NULL);
   A=(Args*)malloc(sizeof(Args));

   A->save_dir=&directory;
   A->p=&p;
   A->pages_downloaded=0;
   A->bytes_downloaded=0;
   A->fd=server_socket;
   A->port=port;
   A->host=&host;

   A->threads_number=number_of_threads;

  A->i_isworking=new bool[number_of_threads];
   for(unsigned int i=0;i<number_of_threads;i++)
   {

     A->i_isworking[i]=false;
   }
   fp=fopen("Filepaths","a+");
   fclose(fp);

   printf("Creating Threads\n");fflush(stdout);

   pthread_mutex_init(&mtx2, 0);
   pthread_mutex_init(&mtx, 0);
   pthread_cond_init(&cond_nonempty, 0);
   pthread_cond_init(&cond_nonfull, 0);

   place(&p,start_url);
   pthread_cond_signal(&cond_nonempty);
   for(unsigned int i=0;i<number_of_threads;i++)
   {
     if ((err = pthread_create(&thr, NULL, ThreadJob, A))){
       printf("pthread_create"); exit(1);
     }
      Threads[i]=thr;
   }

}

WebCrawler::~WebCrawler()
{
  free(directory);
  free(host);
  free(Threads);
}


fd_set read_fds;






void Search(bool first_time,unsigned int pages_number,char* words,int& fd)
{
  char* argv[6],*comm;
  pid_t child;

  static int in_fds[2];




  if(first_time)
  {
    asprintf(&argv[0],"./JE");
    asprintf(&argv[1],"-d");
    asprintf(&argv[2],"Filepaths");
    asprintf(&argv[3],"-w");

    if(pages_number==1){pages_number++;}
    if(pages_number==0){return;}

    asprintf(&argv[4],"%u",pages_number/2);
    argv[5]=(char*)0;

    pipe(in_fds);


    child=fork();


    if(child==0)
    {

      dup2(in_fds[0], STDIN_FILENO);

      execvp("./JE",argv);

    }
    else if(child==-1)
    {
      printf("Error Creatig SEARCH process\n");
      exit(EXIT_FAILURE);
    }
    free(argv[0]);
    free(argv[1]);
    free(argv[2]);
    free(argv[3]);
    free(argv[4]);
    free(argv[5]);
    asprintf(&comm,"/search %s\n",words);
    write(in_fds[1], comm,strlen(comm));
    fd=in_fds[1];

    free(comm);

  }
  else
  {
    asprintf(&comm,"/search %s\n",words);
    write(in_fds[1], comm,strlen(comm));

    free(comm);
  }


}







void WebCrawler::Communication()
{
  int retval,proc_stdin;
  char* command,*mess;
  bool first_time=true;
  pid_t child;

  dup2(command_socket,STDOUT_FILENO);

  for(;;)
  {
    FD_ZERO(&read_fds);
    FD_SET(command_socket, &read_fds);
    retval = select(command_socket+1, &read_fds, NULL, NULL, NULL);
    if(retval<0){exit(EXIT_FAILURE);}

     if (FD_ISSET(command_socket, &read_fds))
     {

       mess=ReadFromTelnet(command_socket);
       command=strtok(mess," ");

       if(strcmp(command,"STATS")==0)
       {
         time_t end = time(NULL);
         printf("Crawler up for %ld seconds,downloaded %u pages, %u bytes\n",end-start_time,A->pages_downloaded,A->bytes_downloaded);
       }
       else if(strcmp(command,"SEARCH")==0)
       {
         if(AllNotWorking(A->i_isworking,number_of_threads)  && first_time)
         {
           for(unsigned int i=0;i<number_of_threads;i++)
           {
             place(&p,NULL);
             pthread_cond_signal(&cond_nonempty);
           }
           for(unsigned int i=0;i<number_of_threads;i++)
           {
             printf("Thread %ld is exiting\n",Threads[i]);fflush(stdout);
             pthread_join(Threads[i],NULL);
             printf("Thread %ld exited\n",Threads[i]);fflush(stdout);
           }
           Search(first_time,A->pages_downloaded,strtok(NULL,""),proc_stdin);
           first_time=false;
         }
         else if(!first_time)
         {
           Search(first_time,A->pages_downloaded,strtok(NULL,""),proc_stdin);
         }
         else
         {
           printf("Crawling in progress..\n");fflush(stdout);
         }

       }
       else if(strcmp(command,"SHUTDOWN")==0)
       {
         if(first_time)
         {
            free(mess);
           break;
         }
         else
         {
           write(proc_stdin,"/exit\n",6);
            free(mess);
            break;
         }

       }
       free(mess);
     }
  }
  remove("Filepaths");
  close(command_socket);
  close(server_socket);

 }
