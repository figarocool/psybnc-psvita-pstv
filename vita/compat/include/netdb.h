/*
 * netdb.h compatibility shim for PS Vita.
 *
 * Only implements what src/p_dns.c actually uses: getaddrinfo() (resolving
 * a hostname, socktype/family hints only, no service lookup) and
 * getnameinfo() (reverse lookup, NI_NAMEREQD only). Both are implemented
 * on top of sceNetResolver in vita_net_compat.c.
 */
#ifndef _VITA_COMPAT_NETDB_H_
#define _VITA_COMPAT_NETDB_H_

#include <netinet/in.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hostent {
    char  *h_name;
    char **h_aliases;
    int    h_addrtype;
    int    h_length;
    char **h_addr_list;
};
#define h_addr h_addr_list[0] /* historical BSD alias for the first address */

struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next;
};

#define NI_MAXHOST   256
#define NI_MAXSERV   32
#define NI_NUMERICHOST 0x0001
#define NI_NAMEREQD    0x0004
#define NI_NUMERICSERV 0x0008

#define EAI_FAMILY  -6
#define EAI_NONAME  -2
#define EAI_AGAIN   -3
#define EAI_FAIL    -4
#define EAI_MEMORY  -10
#define EAI_SYSTEM  -11

int   vita_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void  vita_freeaddrinfo(struct addrinfo *res);
const char *vita_gai_strerror(int errcode);
int   vita_getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags);

#define getaddrinfo(node, serv, hints, res)  vita_getaddrinfo(node, serv, hints, res)
#define freeaddrinfo(res)                    vita_freeaddrinfo(res)
#define gai_strerror(e)                      vita_gai_strerror(e)
#define getnameinfo(a, al, h, hl, s, sl, f)  vita_getnameinfo(a, al, h, hl, s, sl, f)

#ifdef __cplusplus
}
#endif

#endif
