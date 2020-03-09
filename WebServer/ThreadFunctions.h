typedef struct Args
{
  char** parent_dir;
  List* p;
  int fd;
  unsigned int pages_served;
  unsigned int bytes_sent;

}Args;

char* Read_File(char* filename)
{
  char * buffer = NULL;
  long length;
  FILE * f=NULL;

  f = fopen (filename, "rb");

  if (f!=NULL)
  {
    fseek (f, 0, SEEK_END);
    length = ftell (f);
    fseek (f, 0, SEEK_SET);
    buffer = (char*)malloc((length+1)*sizeof(char));
    if (buffer)
    {
      fread (buffer, 1, length, f);
    }
    fclose (f);
    buffer[length]='\0';

  }

  return buffer;
}


bool Packetize(char* message,unsigned int packet_size,int fd)
{
  unsigned int packets_number,packet_rem;
  char* p_mess;
  ssize_t message_size;


  message_size=strlen(message)+1;
  packets_number=message_size/packet_size;
  packet_rem=message_size%packet_size;
  unsigned int i=0,j=0;
  for(i=1;i<=packets_number;i++)
  {
      p_mess=&message[j];
      if(write(fd ,(char*) p_mess,packet_size)<0){return false;}
      j=j+packet_size;
  }
  if(packet_rem!=0)
  {
    p_mess=&message[j];
    if(write(fd ,(char*) p_mess,packet_rem)<0){return false;}
  }

  return true;


}

void Send_Message(char* file,int fd,unsigned int& sent_bytes)
{

  char* Contents;
  char* message=NULL;
  char* date=NULL;
  unsigned int packet_size=1024;


  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  asprintf(&date,"%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);fflush(stdout);

  Contents=Read_File(file);
  if(Contents==NULL && errno==EACCES)
  {

    printf("Permission DENIED");fflush(stdout);
    asprintf(&message,"HTTP/1.1 403 Forbidden\nDate: %s\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 40\nContent-Type: text/html\nConnection: Closed\n\n<html>Trying to access this file.</html>\f",date);
    write(fd ,message,packet_size);
  }
  else if(Contents==NULL)
  {

    printf("FILE NOT FOUND");fflush(stdout);
    asprintf(&message,"HTTP/1.1 404 Not Found\nDate: %s\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 49\nContent-Type: text/html\nConnection: Closed\n\n<html>Sorry dude, couldnt find this file.</html>\f",date);

    write(fd ,message,packet_size);
  }
  else
  {
    asprintf(&message,"HTTP/1.1 200 OK\nDate: %s\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length:%lu\nContent-Type: text/html\nConnection: Closed\n\n%s\f",date,(strlen(Contents)+2),Contents);
    if(!Packetize(message,packet_size,fd)){exit(1);}
    //
    free(Contents);
    Contents=NULL;
    //
  }




  sent_bytes+=strlen(message)+1;
  //
  free(date);
  date=NULL;
  free(message);
  message=NULL;
  //

}

bool CheckMessage(char* message)
{
  char* tmp_mess;
  char* get,*p;
  asprintf(&tmp_mess,"%s",message);
  get=strtok(tmp_mess," ");

  if(strcmp(get,"GET")!=0){free(tmp_mess);return false;}

  p=strtok(NULL,"\n");

  while((p=strtok(NULL," "))!=NULL)
  {
    if(strcmp(p,"Host:")==0){free(tmp_mess);return true;}
    strtok(NULL,"\n");
  }
  free(tmp_mess);
  return false;




}

char* Get_Request(int fd)
{
  char message[2000];
  char* file=NULL;
  char* s;

  if(recv(fd ,message , 2000, 0)==-1)
  {return NULL;}

  if(!CheckMessage(message))
  {

    return NULL;
  }


  strtok(message," ");
  file=strtok(NULL," ");



  asprintf(&s,"%s",file);
  return s;
}


void *Client_Communication(void* args)
{
    char *file,*filename,*parentdir;

    Args* pA;

    pA=(Args*)args;
    parentdir=*pA->parent_dir;

  for(;;)
  {
      file=obtain(pA->p);
      pthread_cond_signal(&cond_nonfull);

      if(strcmp(file,"(null)")==0){
        free(file);
        file=NULL;
        break;
      }
      filename=(char*)malloc(strlen(parentdir)+strlen(file)+1);
      strcpy(filename,parentdir);
      strcat(filename,file);

      Send_Message(filename,pA->fd,pA->bytes_sent);
      pA->pages_served++;

      free(filename);
      free(file);

  }

  pthread_exit(NULL);


}
