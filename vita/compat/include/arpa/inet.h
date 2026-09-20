/* arpa/inet.h compatibility shim for PS Vita */
#ifndef _VITA_COMPAT_ARPA_INET_H_
#define _VITA_COMPAT_ARPA_INET_H_

#include <netinet/in.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int in_addr_t;

unsigned int vita_inet_addr(const char *cp);
char        *vita_inet_ntoa(struct in_addr in);
const char  *vita_inet_ntop(int af, const void *src, char *dst, socklen_t size);
int          vita_inet_pton(int af, const char *src, void *dst);
int          vita_inet_aton(const char *cp, struct in_addr *inp);

#define inet_addr(cp)              vita_inet_addr(cp)
#define inet_ntoa(in)              vita_inet_ntoa(in)
#define inet_ntop(af, src, dst, n) vita_inet_ntop(af, src, dst, n)
#define inet_pton(af, src, dst)    vita_inet_pton(af, src, dst)
#define inet_aton(cp, inp)         vita_inet_aton(cp, inp)

#ifdef __cplusplus
}
#endif

#endif
