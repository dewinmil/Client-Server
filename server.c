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
  //char usrInput[20];
  fprintf(stderr, "Enter Port Number: ");
  //fgets(usrInput, 20, stdin);
  //sscanf(usrInput, "%d", &port);
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

    recvfrom(sockfd, &sendsize, sizeof(int), 0, (struct sockaddr*)&clientaddr, &len);
    if(sendsize == 0){
      recvfrom(sockfd, &sendsize, sizeof(int), 0, (struct sockaddr*)&clientaddr, &len);
    }
    char line[sendsize];
    recvfrom(sockfd, line, sendsize, 0, (struct sockaddr*)&clientaddr, &len);

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
      sendto(sockfd, &sendsize, sizeof(int), 0, (struct sockaddr*)&clientaddr, len);
      
      //char temp[1024];
      int index = 0;
      int packNum = 0;
      int ack = -1;
      int flag = 0;
      int notrecv = 0;
      int ackrecv[1 + ((sendsize -1) / 1024)];
      for(index = 0; index < sizeof(ackrecv); index++){
        ackrecv[index] = 0;
      }
      index = 0;
      packet pack;
      //+1 on ack because packNum increments 1 more than is sent
      while(index <= sendsize || ack+1 < packNum){
      //fprintf(stderr, "index: %d\n", index); 

        if(ack == -1){
          flag = 1;
          ack += flag;
        }
        if(packNum - ack < 6 && index <= sendsize){
          if(sendsize - index < 1024){
            memcpy(pack.c, &buffer[index], sendsize-index);
          }else{
            memcpy(pack.c, &buffer[index], 1024);
          }
          //memcpy(pack.c, buffer+index, 1024);
          //strcpy(pack.c, temp);
          pack.i = packNum;
          sendto(sockfd, &pack, sizeof(pack), 0, (struct sockaddr*)&clientaddr, len);
          index += 1024;
          packNum++;
          if(flag == 1){
            ack = -1;
            flag = 0;
          }
        }
        recvfrom(sockfd, &ack, sizeof(int), MSG_DONTWAIT, (struct sockaddr*)&clientaddr, &len);
        if(ack != -1){
          ackrecv[ack] = 1;
        }
        if(ackrecv[notrecv] == 1){
          notrecv++;
        }

        /*        
        if(packNum - notrecv >= 6){
	  fprintf(stderr,"REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEeeeeee %d\n", notrecv);
          if(sendsize - index < 1024){
            memcpy(pack.c, &buffer[notrecv * 1024], sendsize-index);
             }else{
               memcpy(pack.c, &buffer[notrecv * 1024], 1024);
             }
          pack.i = notrecv;
          sendto(sockfd, &pack, sizeof(pack), 0, (struct sockaddr*)&clientaddr, len);
          recvfrom(sockfd, &ack, sizeof(int), 0, (struct sockaddr*)&clientaddr, &len);
          while(ackrecv[ack] == 1){
            recvfrom(sockfd, &ack, sizeof(int), 0, (struct sockaddr*)&clientaddr, &len);
          }
          if(ack != -1){
            ackrecv[ack] = 1;
          }
          if(ackrecv[notrecv] == 1){
            notrecv++;
          }
        }
        */
        
	fprintf(stderr,"notrecv %d\n", notrecv);
	fprintf(stderr,"last ack received: %d\n", ack);
	fprintf(stderr,"last packNum sent: %d\n\n", packNum-1);
        
      }
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

