#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>

typedef struct{
  int i;
  char c[1024];

} packet;


int main(int argc, char** argv){
  
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  

  int num;
  char usrInput[20];
  fprintf(stderr, "Enter Port Number: ");
  fgets(usrInput, 20, stdin);
  sscanf(usrInput, "%d", &num);
  if(num > 1024 && num < 65535){
    fprintf(stderr, "\nThe Port Is Now: %d\n", num);
  }else{
    fprintf(stderr, "\nInvalid Port: Please enter a port between 1025 and 65535\n");
    return 0;
  }

  char ip[20];
  fprintf(stderr, "Enter IP Address: ");
  fgets(usrInput, 20, stdin);
  sscanf(usrInput, "%s", ip);
  fprintf(stderr, "\nThe Ip Address Is Now: %s\n", ip);
  
  
  struct sockaddr_in serveraddr;
  serveraddr.sin_family=AF_INET;
  serveraddr.sin_port=htons(num);
  //serveraddr.sin_addr.s_addr=inet_addr(ip);
  if(inet_pton(AF_INET, ip, &serveraddr.sin_addr) <= 0){
    fprintf(stderr, "Error: Invalid Ip Address\n");
    return 0;
  }

  while(1){
    int len=sizeof(serveraddr); 
    char line[5000];
    fprintf(stderr, "Enter a line: ");
    fgets(line, 5000, stdin);
    int filesize = strlen(line)+1;
    sendto(sockfd, &filesize, sizeof(int), 0, (struct sockaddr*)&serveraddr, len);
    sendto(sockfd, line, filesize, 0, (struct sockaddr*)&serveraddr, len);
    printf("Sent to server: %s\n", line);
    if(strcmp(line, "exit\n") == 0){
      return 1;
    }
    recvfrom(sockfd, &filesize, sizeof(int), 0, (struct sockaddr*)&serveraddr, &len);
    fprintf(stderr, "filesize: %d\n", filesize);

    int packNum;
    if(filesize > -1){
      char* str1 = "clientFiles/";
      char* str2 = (char *) malloc(1 + strlen(str1)+strlen(line));
      strcpy(str2, str1);
      strcat(str2, line);
      sscanf(str2, "%s", str2);//get rid of newline char
      int fd;
      remove(str2); 
      fd = open(str2, O_CREAT | O_RDWR, S_IRWXU);

      int index = 0;
      packet pack;
      char buffer[filesize];
      while(index <= filesize){
        recvfrom(sockfd, &pack, 1028, 0, (struct sockaddr*)&serveraddr, &len);
        //fprintf(stderr, "pack.c: %s\n", pack.c);
        /*
        if(index == 0){ 
          strcpy(buffer, pack.c);
        }else{
          strcat(buffer, pack.c);
        }*/
        if(filesize - index < 1024){
          memcpy(&buffer[index], pack.c, filesize-index);
        }else{
          memcpy(&buffer[index], pack.c, 1024);
        }
        sendto(sockfd, &pack.i, sizeof(int), 0, (struct sockaddr*)&serveraddr, len);
        packNum = pack.i; 
        fprintf(stderr, "packNum: %d\n", packNum);
        index += 1024;
      }
     
 
      pwrite(fd, buffer, sizeof(buffer), 0); 
      close(fd);
    }else{
      fprintf(stderr, "Error: Invalid File\n");
    }
  }
} 