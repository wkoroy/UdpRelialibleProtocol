#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

class udp_client
{
  public:
  udp_client(const std::string &addr, int port)
  {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int  s, j;
    size_t len;
    size_t nread;
     
   

    /* Obtain address(es) matching host/port */

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    
    s = getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
    Try each address until we successfully connect(2).
    If socket(2) (or connect(2)) fails, we (close the socket
    and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */

        close(sfd);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           /* No longer needed */
  }


  ssize_t send(const char * data, size_t sz)
  {
    return write(sfd, data, sz);
  }

  ssize_t recv(char * data, size_t max_sz)
  {
    return read(sfd, data, max_sz);
  }
  ~udp_client()
  {
    close(sfd);
  }

  int sfd;
  char m_port[10];
};


class udp_server
{
  public:
  udp_server(int port,const char* addr = nullptr)
  {
     struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
     
     
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if(!addr)
    {
      s = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result);
    }
    else
    {
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_protocol = IPPROTO_UDP;
      int r(getaddrinfo(addr , std::to_string(port).c_str(), &hints, &f_addrinfo));
      if (r != 0 || result == NULL)
      {
         std::cout<<"invalid address or port: \""  <<addr << ":"<< port<< "\"";
         exit(EXIT_FAILURE);
      }
      sfd = socket(f_addrinfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
      if (sfd == -1)
      {
          freeaddrinfo(f_addrinfo);
          std::cout<<"could not create socket for: \"" <<addr << ":"<<port  << "\"";
          exit(EXIT_FAILURE);
      }
   }
    
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(sfd);
    }

  }

  ssize_t send_to_peer(const char * data, size_t sz)
  {
    return sendto(
                sfd,
                data,
                sz,
                0,
                (struct sockaddr *) &peer_addr,
                peer_addr_len
            );
  }
  ssize_t send(const char * data, size_t sz)
  {
    if(f_addrinfo)
    return sendto(
                sfd,
                data,
                sz,
                0,
               f_addrinfo->ai_addr,
               f_addrinfo->ai_addrlen
            );
    return sendto(
                sfd,
                data,
                sz,
                0,
                (struct sockaddr *) &peer_addr,
                peer_addr_len
            );
  }

  ssize_t recv(char * data, size_t max_sz)
  {

     peer_addr_len = sizeof(struct sockaddr_storage);
     return recvfrom(
            sfd, data, max_sz, 0, (struct sockaddr *) &peer_addr, &peer_addr_len
        );
  }
  ~udp_server()
  {
    close(sfd);
  }
  int sfd;
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len = 0;
  struct addrinfo *f_addrinfo = nullptr;

};