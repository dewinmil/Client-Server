#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h> 
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>

typedef struct{
  int i;
  char c[1024];

} packet;

typedef struct{
  int i;
  int key;
  char c[1024];
} keyPacket;

int findFileSize(char* string){
  int filesize;
  FILE *fp;
  fp = fopen(string, "r");

  if(fp == NULL){
    return -1;
  }
  fseek(fp, 0, SEEK_END);

  filesize = ftell(fp);

  fclose(fp);

  return filesize;

}


int main(int argc, char** argv){
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
 
 
  int port;
  fprintf(stderr, "Enter Port Number: ");
  fscanf(stdin, "%d", &port);

 
  struct sockaddr_in serveraddr, clientaddr;   
  serveraddr.sin_family=AF_INET;
  
  if(port >1024 && port <= 65535){
    serveraddr.sin_port=htons(port);
    fprintf(stderr, "\nThe Port Is Now: %d\n", port);
  }else{
    fprintf(stderr, "Invalid Port: Please select a port between 1025 and 65535\n\n");
    return 1;
  }
  serveraddr.sin_addr.s_addr=INADDR_ANY;

  bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
  
  while(1){ 
    int len=sizeof(clientaddr);
    int sendsize;
    fprintf(stderr, "new iteration\n");   
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    int flag = -1;

    keyPacket keypack, requestpack;
    keypack.i = 0;
    keypack.key = 0;
    requestpack.i = 0;
    requestpack.key = 0; 
    
    while(keypack.key != 1 || keypack.i == 0){
      recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
      //if(keypack.i == -1){
      //  requestpack.key = -1;
      //  sendto(sockfd, &requestpack, 1032, 0, (struct sockaddr*)&clientaddr, len);  
      //}
    }
    char line[keypack.i];
    strcpy(line, keypack.c);
 
    printf("Got from client: %s\n", line);
    sscanf(line, "%s", line); 
     
    char* str1;
    str1 = "serverFiles/";
    char* str2 = (char *) malloc(1 + strlen(str1)+strlen(line));
    strcpy(str2, str1);
    strcat(str2, line);
    sendsize = findFileSize(str2);
    if(sendsize > -1){
      
       
      int fd;
      fd = open(str2, O_RDONLY | S_IRUSR);
      char buffer[sendsize];
      pread(fd, buffer, sizeof(buffer), 0);
      fprintf(stderr, "sendsize: %d\n", sendsize);  
      
      keypack.i = sendsize;
      fprintf(stderr, "keypack.i: %d\n", keypack.i);  
      keypack.key = 2;
      while(requestpack.key != -2){
        sendto(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, len);
        recvfrom(sockfd, &requestpack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
      }
      while(flag != -1){
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
      }
      //char temp[1024];
      int index = 0;
      int packNum = 0;
      int temp = -1;
      int ack = -1;
      int checkpack;
      packet pack;
      int notrecv = 0;
      int arrsize = 1 + ((sendsize - 1) / 1024);
      int ackrecv[arrsize];
      for(checkpack = 0; checkpack < arrsize; checkpack++){
        ackrecv[checkpack] = 0;
      }
      //+1 on ack because packNum increments 1 more than is sent
      while(index <= sendsize || notrecv+1 < arrsize){
        if(ack < 0){
          ack = 0;
        }


        if(packNum - ack < 6 && index <= sendsize){
          if(temp == -1){
            ack = -1;
          }
          if(sendsize - index < 1024){
            memcpy(pack.c, &buffer[packNum * 1024], sendsize-index);
          }else{
            memcpy(pack.c, &buffer[packNum * 1024], 1024);
          }
          pack.i = packNum;
          sendto(sockfd, &pack, sizeof(pack), 0, (struct sockaddr*)&clientaddr, len);
	  fprintf(stderr,"last packNum sent: %d\n\n", packNum);
          index += 1024;
          packNum++;
        }


        recvfrom(sockfd, &ack, sizeof(int), MSG_DONTWAIT, (struct sockaddr*)&clientaddr, &len);
        if(ack != -1){
 	  fprintf(stderr,"last ack received: %d\n", ack);
 	  fprintf(stderr,"notrecv: %d\n", notrecv);
          temp = 0;
          ackrecv[ack] = 1;
	}


        for(checkpack = 0; checkpack < arrsize; checkpack++){
          if(ackrecv[checkpack] == 0){
            notrecv = checkpack;
            break;
          }
        } 
        if(packNum - ack >=6){
          packNum = notrecv;
          index = 1024 * notrecv;
        } 
        
      }
      flag = 0;
      while(flag != -1){
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
        flag = recvfrom(sockfd, &keypack, 1032, 0, (struct sockaddr*)&clientaddr, &len);
      }
      index = ack = notrecv = pack.i = packNum = 0;
      close(fd);
    }else{
      if(strcmp(line, "exit") == 0){
        //do nothing the clinet will have closed
      }else{
        sendsize = -1;
        fprintf(stderr, "Error: invalid file name\n");
        sendto(sockfd, &sendsize, sizeof(int), 0, (struct sockaddr*)&clientaddr, len);
      }
    }
  }

}
 
