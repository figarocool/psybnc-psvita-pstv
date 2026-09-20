/* netinet/in.h compatibility shim for PS Vita */
#ifndef _VITA_COMPAT_NETINET_IN_H_
#define _VITA_COMPAT_NETINET_IN_H_

#include <sys/socket.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct in_addr {
    unsigned int s_addr; /* network byte order */
};

/* Layout mirrors 4.4BSD (len/family bytes first) so a struct sockaddr_in *
 * can be reinterpreted through struct sockaddr / sockaddr_storage the way
 * the psyBNC code does. */
struct sockaddr_in {
    unsigned char  sin_len;
    unsigned char  sin_family;
    unsigned short sin_port;   /* network byte order */
    struct in_addr sin_addr;   /* network byte order */
    char           sin_zero[8];
};

struct in6_addr {
    unsigned char s6_addr[16];
};

struct sockaddr_in6 {
    unsigned char   sin6_len;
    unsigned char   sin6_family;
    unsigned short  sin6_port;
    unsigned int    sin6_flowinfo;
    struct in6_addr sin6_addr;
    unsigned int    sin6_scope_id;
};

#define INADDR_ANY       0x00000000U
#define INADDR_NONE      0xFFFFFFFFU
#define INADDR_LOOPBACK  0x7F000001U
#define INADDR_BROADCAST 0xFFFFFFFFU

#define INET_ADDRSTRLEN   16
#define INET6_ADDRSTRLEN  46

#define IN6ADDR_ANY_INIT { { {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} } }

unsigned short vita_htons(unsigned short v);
unsigned short vita_ntohs(unsigned short v);
unsigned int   vita_htonl(unsigned int v);
unsigned int   vita_ntohl(unsigned int v);

#define htons(v) vita_htons(v)
#define ntohs(v) vita_ntohs(v)
#define htonl(v) vita_htonl(v)
#define ntohl(v) vita_ntohl(v)

#ifdef __cplusplus
}
#endif

#endif
