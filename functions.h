#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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

int packchecksum(packet pack);

int keypackchecksum(keyPacket pack);

int checkpackchecksum(packet pack);

int checkkeypackchecksum(keyPacket pack);


#endif
