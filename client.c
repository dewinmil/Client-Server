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

typedef struct{
  int i;
  int key;
  char c[1024];

} keyPacket;

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

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    int flag = 0;
   
   keyPacket keypack, requestpack;
    keypack.i = filesize;
    keypack.key = 1;
    strcpy(keypack.c, line);

    sendto(sockfd, &keypack, 1032, 0, (struct sockaddr*)&serveraddr, len);
    printf("Sent to server: %s\n", line);
    if(strcmp(line, "exit\n") == 0){
      return 1;
    }
    recvfrom(sockfd, &requestpack, 1032, 0, (struct sockaddr*)&serveraddr, &len);
    while(requestpack.key != 2){
      recvfrom(sockfd, &requestpack, 1032, 0, (struct sockaddr*)&serveraddr, &len);
      if(requestpack.key == 2){
        keypack.key = -2;
        sendto(sockfd, &keypack, 1032, 0, (struct sockaddr*)&serveraddr, len);
      }
      keypack.key = 1;
      sendto(sockfd, &keypack, 1032, 0, (struct sockaddr*)&serveraddr, len);
    }
    flag == 0;
    while(flag != -1){ 
      flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&serveraddr, &len);
      keypack.key = -2;
      sendto(sockfd, &keypack, 1032, 0, (struct sockaddr*)&serveraddr, len);
    }
    filesize = requestpack.i;
    fprintf(stderr, "filesize: %d\n", requestpack.i);


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
      int recvpack[1 + ((filesize -1) / 1024)]; // make that stupid loop
      for(index = 0; index < sizeof(recvpack); index++){
        recvpack[index] = 0;
      }
      index = 0;
      while(index <= filesize || flag != -1){
        flag = recvfrom(sockfd, &pack, 1028, 0, (struct sockaddr*)&serveraddr, &len);
        if(pack.i == filesize){
          continue;
        }
        fprintf(stderr, "pack.i: %d\n", pack.i);
        if(recvpack[pack.i] != 1){
          if(((pack.i+1) * 1024) >= filesize && filesize - index < 1024){
            fprintf(stderr, "bad memcpy: \n");
            memcpy(&buffer[pack.i * 1024], pack.c, filesize- index);
          }else{
            fprintf(stderr, "good memcpy: \n");
            memcpy(&buffer[pack.i * 1024], pack.c, 1024);
          }
        }
        fprintf(stderr, "packNum: %d\n", packNum);
        if(flag != -1){
          sendto(sockfd, &pack.i, sizeof(int), 0, (struct sockaddr*)&serveraddr, len);
        }
        packNum = pack.i; 
        fprintf(stderr, "packNum: %d\n", packNum);
        if(recvpack[pack.i] != 1){
          index += 1024;
        }
        recvpack[pack.i] = 1;
      }
     
      pack.i = packNum = 0; 
      pwrite(fd, buffer, sizeof(buffer), 0); 
      close(fd);
    }else{
      fprintf(stderr, "Error: Invalid File\n");
    }
  }
} 
