Reader :

#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#define FIFO_PIPE "xyz"
int main()
{
    int fd,i,c=0,w=0,l=0;
    char str[200];
    fd = open(FIFO_PIPE,O_RDONLY);
    read(fd,str,sizeof(str));
    for(i=0;str[i]!='\0';i++)
    {
        if(str[i] == ' '|| str[i] == '\n')
        w++;
        if(str[i] == '\n')
        l++;
    c++;
    }
printf("\n No of characters = %d",c);
printf("\n No of lines = %d", l+1);
printf("\n No of words = %d \n",w);
close(fd);
return 0;
}

Writer : 

#include<sys/types.h>
#include<sys/stat.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#define FIFO_PIPE "xyz"
int main()
{
    int fd,i=0;
    char str[200], ch;
    mkfifo(FIFO_PIPE,0644);//Pipe Created 
    printf("\n Pipe Successfully created \n");
    printf("\n Enter your message(press @ to terminate) \n");
    while((ch = fgetc(stdin)) != '@')
    {
        str[i++] = ch;
    }
    str[i] = '\0';//Null character
    fd = open(FIFO_PIPE,O_WRONLY);
    write(fd,str,strlen(str));
    close(fd);
    return 0;
}
