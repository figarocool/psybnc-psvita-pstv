/*
 * BSD-socket compatibility shim for PS Vita (VitaSDK / SceNet).
 *
 * psyBNC is written against standard BSD sockets. The Vita's SceNet API
 * uses different names/structs and has no select()/fd_set at all, so this
 * header (and its companions in netinet/, arpa/, netdb.h) stand in for the
 * missing system headers and route every call to vita_net_compat.c.
 *
 * This header intentionally also declares read()/write()/close() as
 * macros: every .c file that includes <sys/socket.h> in this codebase only
 * ever uses those three on network sockets (never on real files or
 * pipes), so it's safe to route them all to the socket-only backend here.
 */
#ifndef _VITA_COMPAT_SYS_SOCKET_H_
#define _VITA_COMPAT_SYS_SOCKET_H_

#include <stddef.h>
/* sys/types.h unconditionally pulls in sys/select.h itself (before this
 * header gets a chance to run), which locks in fd_set/FD_SETSIZE at
 * newlib's default of 256 there and then. Redefining FD_SETSIZE here
 * would NOT enlarge the already-fixed fd_set layout - it would just make
 * FD_ZERO/FD_SET (which re-expand FD_SETSIZE at every call site) write
 * past the end of the smaller struct. So: leave FD_SETSIZE alone and
 * live with a 256-socket cap (comfortably above config.h's MAXCONN). */
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int socklen_t;

#define AF_INET     2
#define AF_INET6    10
#define AF_UNSPEC   0
/* AF_UNIX is referenced only inside the disabled fork/exec script path
 * (startpipe() in p_script.c); it just needs to exist to compile. */
#define AF_UNIX     1

#define PF_INET     AF_INET

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define IPPROTO_IP  0
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

#define SOL_SOCKET      0xFFFF
#define SO_REUSEADDR    0x00000004
#define SO_KEEPALIVE    0x00000008
#define SO_BROADCAST    0x00000020
#define SO_LINGER       0x00000080
#define SO_SNDBUF       0x1001
#define SO_RCVBUF       0x1002
#define SO_SNDTIMEO     0x1005
#define SO_RCVTIMEO     0x1006
#define SO_ERROR        0x1007
#define SO_TYPE         0x1008

#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2

/* vitasdk's fcntl.h only knows sceIo file flags; these fcntl() commands
 * are only ever used on sockets in this codebase (F_GETFL/F_SETFL with
 * O_NONBLOCK, F_GETFD as a "is this fd still valid" probe). */
#define F_GETFL     3
#define F_SETFL     4
#define F_GETFD     1
#define F_SETFD     2
#define O_NONBLOCK  0x4000

#define MSG_PEEK        0x00000002
#define MSG_DONTWAIT    0x00000080
#define MSG_NOSIGNAL    0 /* no-op on Vita, kept so call sites still compile */
#define MSG_WAITALL     0x00000040

struct sockaddr {
    unsigned char sa_len;
    unsigned char sa_family;
    char          sa_data[14];
};

/* Generic storage large enough for sockaddr_in / sockaddr_in6 */
struct sockaddr_storage {
    unsigned char ss_len;
    unsigned char ss_family;
    char          ss_padding[126];
};

struct linger {
    int l_onoff;
    int l_linger;
};

/* Real implementations live in vita_net_compat.c */
int vita_socket(int domain, int type, int protocol);
int vita_bind(int fd, const struct sockaddr *addr, socklen_t addrlen);
int vita_listen(int fd, int backlog);
int vita_accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
int vita_connect(int fd, const struct sockaddr *addr, socklen_t addrlen);
int vita_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen);
int vita_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen);
int vita_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
int vita_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen);
int vita_shutdown(int fd, int how);
int vita_socket_close(int fd);
ssize_t vita_send(int fd, const void *buf, size_t len, int flags);
ssize_t vita_recv(int fd, void *buf, size_t len, int flags);
ssize_t vita_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
ssize_t vita_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);
ssize_t vita_read(int fd, void *buf, size_t len);
ssize_t vita_write(int fd, const void *buf, size_t len);
int vita_fcntl(int fd, int cmd, ...);
int vita_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#define socket(domain, type, protocol)         vita_socket(domain, type, protocol)
#define bind(fd, addr, len)                    vita_bind(fd, addr, len)
#define listen(fd, backlog)                    vita_listen(fd, backlog)
#define accept(fd, addr, len)                  vita_accept(fd, addr, len)
#define connect(fd, addr, len)                 vita_connect(fd, addr, len)
#define getsockname(fd, addr, len)             vita_getsockname(fd, addr, len)
#define getpeername(fd, addr, len)             vita_getpeername(fd, addr, len)
#define setsockopt(fd, lvl, opt, val, len)     vita_setsockopt(fd, lvl, opt, val, len)
#define getsockopt(fd, lvl, opt, val, len)     vita_getsockopt(fd, lvl, opt, val, len)
#define shutdown(fd, how)                      vita_shutdown(fd, how)
#define send(fd, buf, len, flags)              vita_send(fd, buf, len, flags)
#define recv(fd, buf, len, flags)              vita_recv(fd, buf, len, flags)
#define sendto(fd, buf, len, flags, to, tl)    vita_sendto(fd, buf, len, flags, to, tl)
#define recvfrom(fd, buf, len, flags, fr, fl)  vita_recvfrom(fd, buf, len, flags, fr, fl)

#define close(fd)                              vita_socket_close(fd)
#define read(fd, buf, len)                     vita_read(fd, buf, len)
#define write(fd, buf, len)                    vita_write(fd, buf, len)
#define fcntl(...)                             vita_fcntl(__VA_ARGS__)
#define select(n, r, w, e, t)                  vita_select(n, r, w, e, t)

/* socketpair() is only reachable from the disabled fork/exec script path. */
static inline int socketpair(int domain, int type, int protocol, int sv[2])
{
    (void)domain; (void)type; (void)protocol; (void)sv;
    return -1;
}

#ifdef __cplusplus
}
#endif

#endif
