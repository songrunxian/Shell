/*************************************************************************
	> File Name: main.c
	> Author: songrunxian 
	> Mail: 13359850710@163.com 
	> Created Time: Wed 24 Apr 2019 04:20:08 AM PDT
 ************************************************************************/

#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <signal.h>
#include <string.h>
#include <termios.h>

#define BUF_SIZE 256
#define PATH_SIZE 256
#define PRINT_FONT_RED  printf("\033[31m"); //红色
#define PRINT_FONT_YEL  printf("\033[33m"); //黄色
#define MAX_CMD 1000

typedef struct _input_type{
    char *str; //the value of input
    int size; //the length of size
}input_type;

input_type input[] = {
    {"exit" ,4},
    {"cd" , 2},
    {"ls" , 2}
};

int cds(const char *p){
    char path[PATH_SIZE];
    char *start;
    char *end;
    int res;
    int n = 0;

    memset(path,'\0',PATH_SIZE);

    start = strchr(p,' ');
    end = strchr(p,'\n');

    if(!start || !end){
        printf("can't support this format \n");
        return 1;
    }

    strncpy(path , p+3 ,end - start -1);

    res = chdir(path);

    if(res != 0){
        printf("%s is not a path,check please\n", path);
    }

    return res;
    
}

void show_attr(char *name)
{
  
  struct stat buf;
  struct passwd *pwd;
  struct group *grp;
  char type ;
  char permission[9];
  int i = 0 ;

  memset(permission,'-',9*sizeof(char));

    stat(name,&buf);
    if(S_ISLNK(buf.st_mode))
      type = 'l';
    else if(S_ISREG(buf.st_mode))
      type = '-';
    else if(S_ISDIR(buf.st_mode))
      type = 'd';
    else if(S_ISCHR(buf.st_mode))
      type = 'c';
    else if(S_ISBLK(buf.st_mode))
      type = 'b';
    else if(S_ISFIFO(buf.st_mode))
      type = 'p';
    else if(S_ISSOCK(buf.st_mode))
      type = 's';

    if(buf.st_mode & S_IRUSR)
      permission[0] = 'r';
    if(buf.st_mode & S_IWUSR)
      permission[1] = 'w';
    if(buf.st_mode & S_IXUSR)
      permission[2] = 'x';
    if(buf.st_mode & S_IRGRP)
      permission[3] = 'r';
    if(buf.st_mode & S_IWGRP)
      permission[4] = 'w';
    if(buf.st_mode & S_IXGRP)
      permission[5] = 'x';
    if(buf.st_mode & S_IROTH)
      permission[6] = 'r';
    if(buf.st_mode & S_IWOTH)
      permission[7] = 'w';
    if(buf.st_mode & S_IXOTH)
      permission[8] = 'x';

    pwd = getpwuid(buf.st_uid);
    grp = getgrgid(buf.st_gid);

    printf("%c",type);
    while(i<9)
    {
      printf("%c",permission[i]);
      i++;
    }
    printf("%2d ",buf.st_nlink);
    printf("%-4s",pwd->pw_name);
    printf("%-4s",grp->gr_name);
    printf( "%6ld ",buf.st_size);
    printf("%.16s",ctime(&buf.st_mtime)); 
    printf(" %s\n",name); 
}

void sort_name(char name[PATH_SIZE][PATH_SIZE],int len)
{
  char str[PATH_SIZE] = {'\0'};
  int i,j;
  int flag = 1;

  for(i=0; i<len && flag; ++i)
  {
    flag = 0;
    for(j=len-1; j> i; --j)
    {
      if(0 < strcmp(name[j],name[j-1]))
      {
        strcpy(str,name[j]);
        strcpy(name[j],name[j-1]);
        strcpy(name[j-1],str);
        flag = 1;
      }
    }
  }
}

int print(char *str)
{
  char path[PATH_SIZE] = {'\0'};
  char name[PATH_SIZE][PATH_SIZE] = {'\0'};
  DIR * dir;
  struct dirent *dir_info;
  int i =0;

  getcwd(path,PATH_SIZE);
  dir=opendir(path);
  while(NULL != (dir_info=readdir(dir)) )
    strcpy(name[i++],dir_info->d_name);

  closedir(dir);
  sort_name(name,i);
  
  if( !strcmp(str,".") )
  {
    int l = i;
    int length = 0;
    int parts_wid = 0;
    int parts_len = 0;
    while(--i>=0){
        if(strlen(name[i]) > length){
            length = strlen(name[i]);
        }
    }
    struct winsize size;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &size);
    parts_wid = size.ws_col / length;
    parts_len = size.ws_row;
    for(int k = 0; k < parts_len; k++){
       for(int j = 0; (j < parts_wid) && (l >= 0); j++){
              --l;
              printf("%-24s",name[l]);
              //printf("%d\t",0);
       }
       printf("\n");  
    }
  }

  if( !strcmp(str,"a") )
  {
    while(--i>=0)
      printf("%s \n",name[i]);
  }
  if( !strcmp(str,"l") )
  {
    while(--i>=0)
      show_attr(name[i]);
  }

  if( !strcmp(str,"al") )
  {
    while(--i>=0)
      show_attr(name[i]);
  }
  return 0;
}
int lss(const char *p)
{
  char *start;
  char *end;
  int res;
  int i= 0;
  char *cmd[]=
  {
    "ls",
    "ls -a",
    "ls -l",
    "ls -al",
  };

  start = strstr(p,"ls");
  if(!start )
  {
    printf("can't support this format \n");
    return 1;
  }
  end = strchr(p,'\n');
  *end = '\0'; 
  for(i=0; i<sizeof(cmd)/sizeof(char*); ++i)
  {
    if( !strcmp(p,cmd[i]) )
    {
      switch(i)
      {
      case 0: res = print(".");
        break;
      case 1: res = print("a");
        break;
      case 2: res = print("l");
        break;
      case 3: res = print("al");
        break;
      default:
        break;
      }
    }
  }

  if(res != 0)
    printf("can't support %s command ,please check again \n",p);

  return res;
}

void sigroutine(int dunno){
    switch(dunno){
        case 2:
        printf("Get a signal __ SIGHUP");
    }
}

int main(int argc , char *argv[]){
    char cmdstring[MAX_CMD];
    char buff[MAX_CMD];
    char hostname[MAX_CMD];
    char buf[BUF_SIZE];
    int res = 1;
    int flag = 1;
    int index = 0;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGTSTP);
    sigprocmask(SIG_SETMASK,&sigset,NULL);
   // pause();


    while(flag){
        getcwd(buf , sizeof(buff));
        gethostname(hostname,sizeof(hostname));
        struct passwd *pwd;
        pwd = getpwuid(getuid());
        PRINT_FONT_RED
        printf("%s@%s:" ,pwd->pw_name,hostname);
        PRINT_FONT_YEL
        printf("%s ",buf);
        
        if(NULL == fgets(buf,BUF_SIZE,stdin))
        return 0;

        index = sizeof(input)/sizeof(input_type);
        while(index-- > 0){
            res = strncmp(buf,input[index].str,input[index].size);
            if(res == 0){
                switch(index){
                    case 0:
                    printf("zoudaolezheli");
                    flag = 0;
                    break;
                    case 1:
                    cds(buf);
                    break;
                    case 2:
                    lss(buf);
                    break;
                    default:
                    printf("can not \n");
                    break;
                }
                index = -1;
            }
        }
        if(index == -1){
            printf("can not find commond: %s",buf);
        }

       // sigprocmask(SIG_SETMASK,&sigset,NULL);
    }
    pause(); 
    return 0;
}
