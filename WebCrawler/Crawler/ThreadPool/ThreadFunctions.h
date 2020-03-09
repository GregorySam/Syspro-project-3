
#include "ThreadPool.h"
#include <sys/file.h>


typedef struct Args
{
  char** save_dir;
  List* p;
  int fd;
  int port;
  char** host;
  unsigned int pages_downloaded;
  unsigned int bytes_downloaded;
  bool* i_isworking;
  unsigned int threads_number;

}Args;



FILE* File_Existance(char* filepath,char* folder_file)
{
  FILE* fp;
  DIR* dir = opendir(folder_file);
  if (dir)
  {
    if(access( filepath, F_OK )!= -1) {
      closedir(dir);
      return NULL;
    }
    else
    {
      fp =fopen(filepath,"a+");
      closedir(dir);
      return fp;
    }
  }
  else if (ENOENT == errno)
  {
    mkdir(folder_file, 0755);
    fp =fopen(filepath,"a+");
    closedir(dir);
    return fp;
  }
  else
  {
    printf("Error opening folder");exit(1);
  }

}

bool CheckIfFileExists_Simple(char* parent,char* file)
{
  char* filepath;
  asprintf(&filepath,"%s%s",parent,file);

  if(access(filepath, F_OK )!= -1)
  {
    free(filepath);
    return true;
  }
  else
  {
    free(filepath);
    return false;
  }


}



char* ExtractFileFromURL(char* url)
{
  char *str,*file;

  str=strchr(url,'/');
  str=strchr(str,':');
  file=strchr(str,'/');


  return file;
}


FILE* CreateFile(char* savedir,char* file)
{
  char* filepath,*folder,*folder_file;

  filepath=(char*)malloc(strlen(savedir)+strlen(file)+1);
  strcpy(filepath,savedir);
  strcat(filepath,file);  //   root_dir/site_i/page_j

  folder=strtok(file,"/");  //   site_i


  folder_file=(char*)malloc(strlen(savedir)+strlen(folder)+2);
  strcpy(folder_file,savedir);
  strcat(folder_file,"/");
  strcat(folder_file,folder);    //  root_dir/site_i

  FILE *fp=File_Existance(filepath,folder_file);
  free(folder_file);
  free(filepath);

	return fp;
}

//
char* AppendString(char* str1,char* str2)
{

  str1=(char*)realloc(str1,strlen(str1)+strlen(str2)+1);
  str1=strcat(str1,str2);

  return str1;



}

char* ReadPackets(int fd,ssize_t packet_size,unsigned int& read_bytes)
{
  char str[1024],*final_mess=NULL;
  int size;

  if((size=read(fd,str,packet_size))<0){return NULL;}
  read_bytes+=size;
  str[size]='\0';
  asprintf(&final_mess,"%s",str);

  for(;;)
  {

    if(final_mess[strlen(final_mess)-1]=='\f'){return final_mess;}
    if((size=read(fd,str,packet_size))<0){return NULL;}
    read_bytes+=size;
    str[size]='\0';
    final_mess=AppendString(final_mess,str);
  //  asprintf(&final_mess,"%s%s",final_mess,str);

  }
  return NULL;

}


void FindLinks(char* html_file,int port,char* host,List* p)
{
    char* link,*p_link,*str,*url;

    while((str=strstr(html_file,"<a href=.."))!=NULL)
    {
      p_link=&str[strlen("<a href=..")];
      link=strtok(p_link,">");
      asprintf(&url,"http://%s:%d%s",host,port,link);
      place(p,url);
      free(url);
      pthread_cond_signal(&cond_nonempty);
      html_file=&link[strlen(link)+2];
    }

}


char* ReadResponse(char* rcv_message)
{
  char* first_line,*html_str;

  first_line=strtok(rcv_message,"\n");



  if(strcmp(first_line,"HTTP/1.1 200 OK")==0)
  {
    rcv_message=rcv_message+strlen(first_line)+2;
    html_str=strchr(rcv_message,'<');
    html_str[strlen(html_str)-1]='\0';
    return html_str;
  }
  else
  {
    return NULL;
  }




}

char* CreateGetMeassage(char* filename,char* host)
{
  char *mess;


  asprintf(&mess,"GET %s HTTP/1.1\nUser-Agent: Mozilla\nHost: %s\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nConnection: Keep-Alive\n",filename,host);
  return mess;
}


void *ThreadJob(void* args)
{
  char* url,*file,*mess,*html_str,*rcv_message;
  static int k=-1;
  int id;
  FILE* fp;

  Args* pA;

  pA=(Args*)args;

    k++;
    id=k;



  for(;;)
  {

    url=obtain(pA->p,pA->i_isworking,pA->threads_number,id);

    if(url==NULL){break;}


      pthread_mutex_lock(&mtx2);
    file=ExtractFileFromURL(url);


    if(CheckIfFileExists_Simple(*pA->save_dir,file))
    {
      free(url);
      pthread_mutex_unlock(&mtx2);
      continue;
    }

    mess=CreateGetMeassage(file,*pA->host);

    if( send(pA->fd, mess , strlen(mess), 0) < 0)
     {
         puts("Send failed");

     }
     free(mess);
     rcv_message=ReadPackets(pA->fd,1024,pA->bytes_downloaded);

     html_str=ReadResponse(rcv_message);

     if(html_str!=NULL)
     {
       fp=fopen("Filepaths","a+");
       fprintf(fp,"%s/%s\n",*pA->save_dir,file);
       fclose(fp);

       fp=CreateFile(*pA->save_dir,file);
       if(fp==NULL)
       {
         pthread_mutex_unlock(&mtx2);
         //dont analyze
       }
       else
       {
         fprintf(fp,"%s",html_str);
         pA->pages_downloaded++;
          fclose(fp);
          pthread_mutex_unlock(&mtx2);
          FindLinks(html_str,pA->port,*pA->host,pA->p);
        }
     }


     free(rcv_message);
     free(url);

   }
   pthread_exit(NULL);

}
