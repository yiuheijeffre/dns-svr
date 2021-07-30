// A simple server in the internet domain using TCP
// The port number is passed as an argument
// To compile: gcc server.c -o server
// Reference: Beej's networking guide, man pages
// Initial code framework is from week9-practical

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 500
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include "helper1.h"

int main(int argc, char** argv) {

	//Act as client to upstream Server
	int c_sockfd, c_n, c_s;
	struct addrinfo c_hints, *c_servinfo, *c_rp;
	memset(&c_hints, 0, sizeof c_hints);
	c_hints.ai_family = AF_INET;
	c_hints.ai_socktype = SOCK_STREAM;

	c_s = getaddrinfo(argv[1], argv[2], &c_hints, &c_servinfo);
	if(c_s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(c_s));
		exit(EXIT_FAILURE);
	}

	for(c_rp = c_servinfo; c_rp != NULL; c_rp = c_rp->ai_next) {
		c_sockfd = socket(c_rp->ai_family, c_rp->ai_socktype, c_rp->ai_protocol);
		if(c_sockfd == -1)
			continue;

		if(connect(c_sockfd, c_rp->ai_addr, c_rp->ai_addrlen) != -1)
			break;
		close(c_sockfd);
	}
	if(c_rp == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(c_servinfo);

	// define buffer
  unsigned char size[2];
	unsigned char rsize[2];
  ssize_t bytes_read;
	ssize_t r_bytes_read;
  uint16_t r;
  int reading = 1;
  int not_implement = 0;
	// open file for writing
	int filefd = open("dns_svr.log", O_WRONLY | O_CREAT, 0600);
	FILE* f = fdopen(filefd, "w");

	while (1) {
		int sockfd, newsockfd, n, re, s;
		struct addrinfo hints, *res;
		struct sockaddr_storage client_addr;
		socklen_t client_addr_size;

		// Create address we're going to listen on (with given port number)
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;       // IPv4
		hints.ai_socktype = SOCK_STREAM; // TCP
		hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
		// node (NULL means any interface), service (port), hints, res
		s = getaddrinfo(NULL, "8053", &hints, &res);
		if (s != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
			exit(EXIT_FAILURE);
		}

		// Create socket
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			perror("socket");
			exit(EXIT_FAILURE);
		}

		// Reuse port if possible
		re = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		// Bind address to the socket
		if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
			perror("bind");
			exit(EXIT_FAILURE);
		}

		// Listen on socket - means we're ready to accept connections,
		// incoming connection requests will be queued, man 3 listen
		if (listen(sockfd, 1) < 0) {
			perror("listen");
			exit(EXIT_FAILURE);
		}

		// Accept a connection - blocks until a connection is ready to be accepted
		// Get back a new file descriptor to communicate on
		client_addr_size = sizeof client_addr;
		newsockfd =
			accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
		if (newsockfd < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		freeaddrinfo(res);

		packet_t packet;
		packet_t rpacket;
		reading = 1;
		not_implement = 0;
    if(reading) {
      /* READ QUERY */
  		bytes_read = read(newsockfd, size, 2);
  		r = getuint16(size);
      packet.filesize = r;

      /*Reading packet content*/
      unsigned char data[packet.filesize];
      unsigned char temp2[2];
      bytes_read = read(newsockfd, data, packet.filesize); //Offset: 2 bytes
			int full_data_length = packet.filesize+2;
			unsigned char raw_data[full_data_length];
			memcpy(raw_data, size, 2);
			memcpy(&raw_data[2], data, packet.filesize);

      if (bytes_read < 0) {
  			perror("read");
  			exit(EXIT_FAILURE);
  		}
      /* ---------------- */

      /* HEADER */
      memcpy(temp2, &data[0], 2);
      packet.id = getuint16(temp2);

      /* QUERY PARAMETER */
      //Third byte: QR, Opcode, AA, TC, RD
      //Forth byte: RA/Z/AD/CD/RCODE
      memcpy(temp2, &data[2], 2);
      packet.query = getuint16(temp2);
      packet.qr = packet.query >> 15 & 1;
      if(packet.query >> 11 & 1 && packet.query >> 12 & 0) { //Ignore 3-15, 01 in decimal is 1
        packet.opcode = 1; //Inverse query
      } else if(packet.query >> 11 & 0 && packet.query >> 12 & 1) { //10 in decimal is 2
        packet.opcode = 2; //Server status request
      } else {
        packet.opcode = 0; //Standard query
      }
      packet.aa = packet.query >> 10 & 1;
      packet.tc = packet.query >> 9 & 1;
      packet.rd = packet.query >> 8 & 1;
      packet.ra = packet.query >> 7 & 1;
      packet.z = packet.query >> 6 & 1;
      packet.ad = packet.query >> 5 & 1;
      packet.cd = packet.query >> 4 & 1;
      //packet.rcode = packet.query >> 3, packet >> 2, packet >> 1, packet >> 0 (4bit)

      //Fifth and sixth byte: QDCOUNT
      //Seventh and eigth byte: ANCOUNT
      //Nineth and tenth byte: NSCOUNT
      //Eleventh and twelvth byte: ARCOUNT
      memcpy(temp2, &data[4], 2);
      packet.qdcount = getuint16(temp2);
      packet.num_questions = packet.qdcount;

      memcpy(temp2, &data[6], 2);
      packet.ancount = getuint16(temp2);
      packet.num_answers = packet.ancount;

      memcpy(temp2, &data[8], 2);
      packet.nscount = getuint16(temp2);
      packet.num_ns = packet.nscount;

      memcpy(temp2, &data[10], 2);
      packet.arcount = getuint16(temp2);
      packet.num_ar = packet.arcount;

      /* QUESTION */

      /* QNAME */
      uint8_t byte;
      int length;
      int curr = 0;
      packet.url_start = packet.url_end =  12; //QNAME start from 13th byte
      packet.url_length = 0;
      memcpy(&byte, &data[packet.url_start], 1);
      length = byte;
      while(length != 0) {
        packet.url_length += length;
        for(int i = 0; i < length; i++) {
          packet.url[curr] = (char) data[packet.url_end + 1 + i];
          curr += 1;
        }
        packet.url_end = packet.url_end + length + 1;
        memcpy(&byte, &data[packet.url_end], 1);
        length = byte;
        if(length != 0) {
          packet.url[curr] = '.';
          packet.url_length += 1;
          curr += 1;
        }
      }
      packet.url[curr] = '\0';

      /* QTYPE & QCLASS */
      int x;
      packet.url_end = packet.url_end + 1;
      x = packet.url_end; //Start from the end of qname section
      memcpy(temp2, &data[x], 2);
      packet.qtype = getuint16(temp2);
      memcpy(temp2, &data[x+2], 2);
      packet.qclass = getuint16(temp2);
      packet.qtype_value = packet.qtype;
      if(packet.qtype_value != 28) {
        //Not AAAA record
        not_implement = 1;
      }

      /* END OF QUERY */
      /* -------------------- */
			/* LOG FILE */

      size_t time_byte;
      char time_buf[30];
      time_t rawtime;
      struct tm *info;
      time(&rawtime);
      info = localtime(&rawtime);
      time_byte = strftime(time_buf, sizeof(time_buf), "%FT%T%z", info);

      char result[500];
      if(not_implement) {
        length = time_byte+22+1; //sizeof time_buf + strlen(unimplemented request) + null terminate
        strcpy(result, time_buf);
        strcat(result, " unimplemented request\n");
        result[length] = '\0';
      } else {
        length = time_byte+11+packet.url_length+1;
        strcpy(result, time_buf);
        strcat(result, " requested ");
        strcat(result, packet.url);
				strcat(result, "\n");
        result[length] = '\0';
      }

  		n = write(filefd, result, length);
			fflush(f);
  		if (n < 0) {
  			perror("write");
  			exit(EXIT_FAILURE);
  		}

			if(not_implement) {
				/* Response to client with query: qr = 1, rcode = 4 */
				raw_data[4] = raw_data[4] | (1 << 7); //Change from query to response (qr bit)
				raw_data[5] = raw_data[5] | (1 << 2); //Change rcode to 4 (Binary: 100)
				n = write(newsockfd, raw_data, full_data_length);
				if(n < 0) {
					perror("write");
					exit(EXIT_FAILURE);
				}

			} else{

				//Forward query to upstream server
				c_n = write(c_sockfd, raw_data, full_data_length);
				if(c_n < 0) {
					perror("socket");
					exit(EXIT_FAILURE);
				}

				r_bytes_read = read(c_sockfd, rsize, 2);
				if(r_bytes_read < 0) {
					perror("read");
					exit(EXIT_FAILURE);
				}

				/* READ RESPONSE */
				r = getuint16(rsize);
				rpacket.filesize = r;

				/* Reading response content */
				unsigned char response[rpacket.filesize];
				r_bytes_read = read(c_sockfd, response, rpacket.filesize);
				int full_rdata_length = rpacket.filesize + 2;
				unsigned char raw_response[full_rdata_length];
				memcpy(raw_response, rsize, 2);
				memcpy(&raw_response[2], response, rpacket.filesize);

				if(r_bytes_read < 0) {
					perror("read");
					exit(EXIT_FAILURE);
				}

				/* HEADER */
				memcpy(temp2, &response[0], 2);
	      rpacket.id = getuint16(temp2);

				/* QUERY PARAMETER */
	      //Third byte: QR, Opcode, AA, TC, RD
	      //Forth byte: RA/Z/AD/CD/RCODE
	      memcpy(temp2, &response[2], 2);
	      rpacket.query = getuint16(temp2);
	      rpacket.qr = rpacket.query >> 15 & 1;
	      if(rpacket.query >> 11 & 1 && rpacket.query >> 12 & 0) { //Ignore 3-15, 01 in decimal is 1
	        rpacket.opcode = 1; //Inverse query
	      } else if(rpacket.query >> 11 & 0 && rpacket.query >> 12 & 1) { //10 in decimal is 2
	        rpacket.opcode = 2; //Server status request
	      } else {
	        rpacket.opcode = 0; //Standard query
	      }
	      rpacket.aa = rpacket.query >> 10 & 1;
	      rpacket.tc = rpacket.query >> 9 & 1;
	      rpacket.rd = rpacket.query >> 8 & 1;
	      rpacket.ra = rpacket.query >> 7 & 1;
	      rpacket.z = rpacket.query >> 6 & 1;
	      rpacket.ad = rpacket.query >> 5 & 1;
	      rpacket.cd = rpacket.query >> 4 & 1;
	      //packet.rcode = packet.query >> 3, packet >> 2, packet >> 1, packet >> 0 (4bit)

	      //Fifth and sixth byte: QDCOUNT
	      //Seventh and eigth byte: ANCOUNT
	      //Nineth and tenth byte: NSCOUNT
	      //Eleventh and twelvth byte: ARCOUNT
	      memcpy(temp2, &response[4], 2);
	      rpacket.qdcount = getuint16(temp2);
	      rpacket.num_questions = rpacket.qdcount;

	      memcpy(temp2, &response[6], 2);
	      rpacket.ancount = getuint16(temp2);
	      rpacket.num_answers = rpacket.ancount;

	      memcpy(temp2, &response[8], 2);
	      rpacket.nscount = getuint16(temp2);
	      rpacket.num_ns = rpacket.nscount;

	      memcpy(temp2, &response[10], 2);
	      rpacket.arcount = getuint16(temp2);
	      rpacket.num_ar = rpacket.arcount;

	      /* QUESTION */

	      /* QNAME */
	      curr = 0;
	      rpacket.url_start = rpacket.url_end =  12; //QNAME start from 13th byte
	      rpacket.url_length = 0;
	      memcpy(&byte, &response[rpacket.url_start], 1);
	      length = byte;
	      while(length != 0) {
	        rpacket.url_length += length;
	        for(int i = 0; i < length; i++) {
	          rpacket.url[curr] = (char) response[rpacket.url_end + 1 + i];
	          curr += 1;
	        }
	        rpacket.url_end = rpacket.url_end + length + 1;
	        memcpy(&byte, &response[rpacket.url_end], 1);
	        length = byte;
	        if(length != 0) {
	          rpacket.url[curr] = '.';
	          rpacket.url_length += 1;
	          curr += 1;
	        }
	      }
	      rpacket.url[curr] = '\0';

				/* QTYPE & QCLASS */
	      rpacket.url_end = rpacket.url_end + 1;
	      x = rpacket.url_end; //Start from the end of qname section
	      memcpy(temp2, &response[x], 2);
	      rpacket.qtype = getuint16(temp2);
	      memcpy(temp2, &response[x+2], 2);
	      rpacket.qclass = getuint16(temp2);
	      rpacket.qtype_value = rpacket.qtype;

				/* ANSWER */
				//Only log the first answer if it is AAAA.
				// memcpy(temp2, &response[x+4], 2); //QNAME

				memcpy(temp2, &response[x+6], 2); //TYPE
				rpacket.type = getuint16(temp2);
				rpacket.type_value = rpacket.type;

				// memcpy(temp2, &response[x+8], 2); //CLASS
				// memcpy(temp2, &response[x+10], 4);//TTL
				uint16_t rlength;

				memcpy(temp2, &response[x+14], 2); //RDLENGTH
				rlength = getuint16(temp2);
				rpacket.rdlength = rlength;
				//RDATA
				inet_ntop(AF_INET6, &response[x+16], rpacket.rdata, INET6_ADDRSTRLEN);
				// printf("\nQUERY: \n");
				// for(int i = 0;i < full_data_length; i++) {
				// 	printf("%02x ", raw_data[i]);
				// }
				// printf("\nRESPONSE: \n");
	// 			printf("id: %x, qr:%d, opcode:%d, aa:%d, tc:%d, rd:%d, ra:%d, z:%d, ad:%d, cd:%d, num_question:%d, num_answer:%d, num_ns:%d, num_ar:%d, qtype:%d\n",
	// rpacket.id, rpacket.qr, rpacket.opcode, rpacket.aa, rpacket.tc, rpacket.rd, rpacket.ra, rpacket.z,
	// rpacket.ad, rpacket.cd, rpacket.num_questions, rpacket.num_answers, rpacket.num_ns, rpacket.num_ar, rpacket.type_value);
	//       printf("url: ");
	//       for(int i = 0; i < rpacket.url_length; i++) {
	//         printf("%c", rpacket.url[i]);
	//       }
	//       printf("\n");
	// 			printf("url length: %d\n", rpacket.url_length);
				// for(int i = 0; i < full_rdata_length; i++) {
				// 	printf("%02x ", raw_response[i]);
				// }

				time(&rawtime);
				info = localtime(&rawtime);
				time_byte = strftime(time_buf, sizeof(time_buf), "%FT%T%z", info);
				char rresult[500]; //response result
				if(rpacket.type_value == 28) {
					length = time_byte+8+rpacket.url_length+strlen(rpacket.rdata)+1;
					strcpy(rresult, time_buf);
					strcat(rresult, " ");
					strcat(rresult, rpacket.url);
					strcat(rresult, " is at ");
					strcat(rresult, rpacket.rdata);
					strcat(rresult, "\n");
					rresult[length+2] = '\0';
					n = write(filefd, rresult, length);
					if(n < 0) {
						perror("write");
						exit(EXIT_FAILURE);
					}
					fflush(f);
				}

				//Forward response to client
				n = write(newsockfd, raw_response, full_rdata_length);
				if(n < 0) {
					perror("write");
					exit(EXIT_FAILURE);
				}
			}
			close(sockfd);
			close(newsockfd);
    }
	}
	return 0;
}
