# COMP30023 - Miniature DNS server
 
- Accept a DNS “AAAA” query over TCP on port 8053
- Extract useful information from client's DNS query packet such as query type, class, Opcode etc. using bit operation
- Forward the query to upstream server given a valid AAAA query
- Log user's query and upstream server's response to txt file
- Return upstream server’s response to user
