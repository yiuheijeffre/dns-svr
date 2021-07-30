uint16_t getuint16(unsigned char *bytes);
uint32_t getuint32(unsigned char *bytes);

typedef struct packet packet_t;
struct packet{
  /* HEADER */
  int filesize;
  uint16_t id;

  /* QUERY PARAMETER */
  uint16_t query;
  int qr;
  int opcode;
  int aa;
  int tc;
  int rd;
  int ra;
  int z;
  int ad;
  int cd;

  /* QDCOUNT */
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
  int num_questions;
  int num_answers;
  int num_ns;
  int num_ar;

  /* QUESTION */

  /* QNAME */
  char url[255];
  int url_length;
  int url_start;
  int url_end;

  /* QTYPE & QCLASS */
  uint16_t qtype;
  uint16_t qclass;
  int qtype_value;

  /* ANSWER */
  // uint16_t name;
  uint16_t type;
  int type_value;
  // uint16_t class;
  uint32_t ttl;
  int rdlength;
  char rdata[INET6_ADDRSTRLEN];
};
