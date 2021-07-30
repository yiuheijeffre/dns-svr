#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "helper1.h"

int main(int argc, char* argv[]) {
    unsigned char size[2];
    uint16_t r;

    /* Reading packet size */
    read(STDIN_FILENO, size, 2);
    r = getuint16(size);
    int filesize = r;
    printf("%d\n", filesize);

    /* Reading packet content */
    unsigned char data[filesize];
    pread(STDIN_FILENO, data, sizeof(data), 2);
    // uint8_t test[2];
    // for(int i = 0; i < 1; i++) {
    //   test[0] = data[i];
    //   // printf("%02x\n", data[i]);
    // }

    /* HEADER */
    unsigned char temp2[2];
    // uint16_t id;
    /* QUERY PARAMETER*/
    uint16_t query;
    int qr;
    int opcode;
    int aa;
    int tc;
    int rd;
    int ra;
    /* QDCOUNT */
    uint16_t qdcount;
    uint16_t ancount;
    int num_questions;
    int num_answers;

    //First two bytes: ID
    memcpy(temp2, &data[0], 2);
    // id = getuint16(temp2);

    //Third byte: QR, Opcode, AA, TC, RD
    //Forth byte: RA/Z/AD/CD/RCODE
    memcpy(temp2, &data[2], 2);
    query = getuint16(temp2);
    qr = query >> 15 & 1;
    if(query >> 11 & 1 && query >> 12 & 0) { //Ignore 3-15, 01 in decimal is 1
      opcode = 1; //Inverse query
    } else if(query >> 11 & 0 && query >> 12 & 1) { //10 in decimal is 2
      opcode = 2; //Server status request
    } else {
      opcode = 0; //Standard query
    }
    aa = query >> 10 & 1;
    tc = query >> 9 & 1;
    rd = query >> 8 & 1;
    ra = query >> 7 & 1;

    //fifth and sixth bytes: QDCOUNT
    memcpy(temp2, &data[4], 2);
    qdcount = getuint16(temp2);
    num_questions = qdcount;

    memcpy(temp2, &data[6], 2);
    ancount = getuint16(temp2);
    num_answers = ancount;

    // data[6], data[7] = ANCOUNT
    // data[8], data[9] = NSCOUNT
    // data[10], data[11] = ARCOUNT

    /*QUESTION*/

    /*QNAME*/

    uint8_t byte;
    int length;
    int start = 12; //QNAME start from 13th byte
    char url[255];
    int curr = 0;
    int total_length = 0;
    //Assume every query have a QNAME section with url
    //First byte: label length
    memcpy(&byte, &data[start], 1); //data[12]
    length = byte;
    while(length != 0) { //not equal to NULL terminate byte
      total_length += length;
      for(int i = 0; i < length; i++) {
        url[curr] = (char) data[start + 1 + i];
        // printf("%c", data[start+1+i]);
        curr += 1;
      }
      start = start + length + 1;
      memcpy(&byte, &data[start], 1);
      length = byte;
      if(length != 0) {
        url[curr] = '.';
        total_length += 1;
        curr += 1;
      }
    }
    for(int i = 0; i < total_length; i++) {
      printf("%c", url[i]);
    }
    printf("\nqr: %d, Opcode: %d, AA: %d, TC: %d, RD: %d, RA: %d, num_q: %d, num_a: %d", qr, opcode, aa, tc, rd, ra, num_questions, num_answers);

    /* QTYPE & QCLASS */
    uint16_t qtype;
    uint16_t qclass;
    int qtype_value;

    start = start + 1;
    memcpy(temp2, &data[start], 2);
    qtype = getuint16(temp2);
    memcpy(temp2, &data[start+2], 2);
    qclass = getuint16(temp2);
    qtype_value = qtype;
    printf(" qtype: %d", qtype_value);
    printf(" qclass: %04x", qclass);

    /* ANSWER */
    uint16_t name;
    uint16_t type;
    uint16_t class;
    uint32_t ttl;
    uint16_t rdlength;
    uint16_t rdata;

    memcpy(temp2, &data[start+4], 2);
    name = getuint16(temp2);
    memcpy(temp2, &data[start+6], 2);
    type = getuint16(temp2);
    memcpy(temp2, &data[start+8], 2);
    class = getuint16(temp2);
    memcpy(temp2, &data[start+10], 4);
    ttl = getuint32(temp2);
    memcpy(temp2, &data[start+14], 2);
    rdlength = getuint16(temp2);
    memcpy(temp2, &data[start+16], 2);
    rdata = getuint16(temp2);

    printf("\n%x, %x, %x, %x, %x, %x", name, type, class, ttl, rdlength, rdata);

    // data[13+length+length] = QTYPE
    // data[13+length+length+1] = QCLASS
    //
    // /*response*/
    // data[0], data[1] = ID;
    // data[2] >> 7 & 1 = QR;
    // data[2] >> 3 = Opcode;
    // data[2] >> 2 = AA;
    // data[2] >> 1 & 1= TC;
    // data[2] & 1 = RD;
    //
    // data[3] >> 7 & 1 = RA;
    // //data[3] is RA/Z/RCODE (uuseless)
    // data[4], data[5] = QDCOUNT
    // data[6], data[7] = ANCOUNT
    // data[8], data[9] = NSCOUNT
    // data[10], data[11] = ARCOUNT
    //
    // /*QUESTION*/
    // //missing two bytes
    // // data[14] = length in raw
    // data[12] = length
    // data[13] = "x"
    // data[14] = "x"
    // ...
    // data[13 + length] = length
    // ...
    // data[13+length+length], data[13+length+length+1] = QTYPE
    // data[13+length+length+2], data[13+length+length+3] = QCLASS
    //
    // /*ANSWER*/
    // data[a], data[a+1] = name
    // data[a+2], data[a+3] = TYPE
    // data[a+4], data[a+5] = CLASS
    // data[a+6], data[a+7], data[a+8], data[a+9] = TTL
    // data[a+10], data[a+11] = length;
    // data[remaining] = IP;



    /*----------------------*/

    // printf("%x", test[0]);


    return 0;
}
