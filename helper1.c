#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

uint16_t getuint16(unsigned char *bytes) {
  uint16_t r;
  memcpy(&r, bytes, sizeof(r));
  return ntohs(r);
}

uint32_t getuint32(unsigned char *bytes) {
  uint32_t r;
  memcpy(&r, bytes, sizeof(r));
  return ntohs(r);
}
