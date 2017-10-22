#include <stdlib.h>
#include <string.h>

typedef struct{
  int i;
  char c[1024];
  int checksum;
} packet;

typedef struct{
  int i;
  int key;
  char c[1024];
  int checksum;
} keyPacket;


int packchecksum(packet pack){
  int count = 0;
  int i;
  for(i = 0; i < strlen(pack.c); i++){
    count += (int)pack.c[i];
  }
  count += pack.i;
  return count;
}


int keypackchecksum(keyPacket pack){
  int count = 0;
  int i;
  for(i = 0; i < strlen(pack.c); i++){
    count += (int)pack.c[i];
  }
  count += pack.i;
  count += pack.key;
  return count;
}

int checkpackchecksum(packet pack){
  int count = packchecksum(pack);
  if(count != pack.checksum){
    return -1;
  }
  return 1;
}


int checkkeypackchecksum(keyPacket pack){
  int count = keypackchecksum(pack);
  if(count != pack.checksum){
    return -1;
  }
  return 1;
}
