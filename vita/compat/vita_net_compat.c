/*
 * BSD-socket / resolver / select() compatibility layer for PS Vita.
 *
 * Backs the macros declared in compat/include/{sys/socket.h, netinet/in.h,
 * arpa/inet.h, netdb.h} with the real SceNet API. See sys/socket.h for the
 * rationale (psyBNC is written against standard BSD sockets; the Vita has
 * neither those calls nor select()/fd_set).
 *
 * Include the Sony headers first so their own declarations are parsed
 * before our socket()/close()/read()/... macros become active.
 */
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "vita_net_compat.h"

static char s_net_memory[1 * 1024 * 1024];

int vita_net_init(void)
{
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

    SceNetInitParam param;
    param.memory = s_net_memory;
    param.size = sizeof(s_net_memory);
    param.flags = 0;
    int rc = sceNetInit(&param);
    if (rc < 0 && rc != SCE_NET_ERROR_ENOTINIT)
    {
        /* Continue anyway: SCE_NET_ERROR_EINTERNAL-class failures here are
         * rare on real hardware; if it truly failed every socket call
         * below will fail loudly too. */
    }
    sceNetCtlInit();
    return 0;
}

void vita_net_term(void)
{
    sceNetCtlTerm();
    sceNetTerm();
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

int vita_net_wait_connected(int timeout_seconds)
{
    for (int waited = 0; waited < timeout_seconds; waited++)
    {
        int state = SCE_NETCTL_STATE_DISCONNECTED;
        if (sceNetCtlInetGetState(&state) == 0 && state == SCE_NETCTL_STATE_CONNECTED)
            return 1;
        sceKernelDelayThread(1000 * 1000);
    }
    return 0;
}

int vita_net_get_ip(char *buf, size_t len)
{
    SceNetCtlInfo info;
    memset(&info, 0, sizeof(info));
    if (sceNetCtlInetGetInfo(SCE_NETCTL_INFO_GET_IP_ADDRESS, &info) < 0)
    {
        if (len > 0) buf[0] = 0;
        return -1;
    }
    strncpy(buf, info.ip_address, len - 1);
    buf[len - 1] = 0;
    return 0;
}

/* ---- errno mapping ---------------------------------------------------- */

static int sce_net_errno(int sce_ret)
{
    unsigned int code = (unsigned int)(-sce_ret);
    switch (code)
    {
        case SCE_NET_ERROR_EINTR:         return EINTR;
        case SCE_NET_ERROR_EBADF:         return EBADF;
        case SCE_NET_ERROR_EACCES:        return EACCES;
        case SCE_NET_ERROR_EFAULT:        return EFAULT;
        case SCE_NET_ERROR_EINVAL:        return EINVAL;
        case SCE_NET_ERROR_EMFILE:        return EMFILE;
        case SCE_NET_ERROR_EWOULDBLOCK:   return EWOULDBLOCK; /* == EAGAIN */
        case SCE_NET_ERROR_EINPROGRESS:   return EINPROGRESS;
        case SCE_NET_ERROR_EALREADY:      return EALREADY;
        case SCE_NET_ERROR_ENOTSOCK:      return ENOTSOCK;
        case SCE_NET_ERROR_EDESTADDRREQ:  return EDESTADDRREQ;
        case SCE_NET_ERROR_EMSGSIZE:      return EMSGSIZE;
        case SCE_NET_ERROR_EPROTOTYPE:    return EPROTOTYPE;
        case SCE_NET_ERROR_ENOPROTOOPT:   return ENOPROTOOPT;
        case SCE_NET_ERROR_EPROTONOSUPPORT: return EPROTONOSUPPORT;
        case SCE_NET_ERROR_EOPNOTSUPP:    return EOPNOTSUPP;
        case SCE_NET_ERROR_EAFNOSUPPORT:  return EAFNOSUPPORT;
        case SCE_NET_ERROR_EADDRINUSE:    return EADDRINUSE;
        case SCE_NET_ERROR_EADDRNOTAVAIL: return EADDRNOTAVAIL;
        case SCE_NET_ERROR_ENETDOWN:      return ENETDOWN;
        case SCE_NET_ERROR_ENETUNREACH:   return ENETUNREACH;
        case SCE_NET_ERROR_ECONNABORTED:  return ECONNABORTED;
        case SCE_NET_ERROR_ECONNRESET:    return ECONNRESET;
        case SCE_NET_ERROR_ENOBUFS:       return ENOBUFS;
        case SCE_NET_ERROR_EISCONN:       return EISCONN;
        case SCE_NET_ERROR_ENOTCONN:      return ENOTCONN;
        case SCE_NET_ERROR_ETIMEDOUT:     return ETIMEDOUT;
        case SCE_NET_ERROR_ECONNREFUSED:  return ECONNREFUSED;
        case SCE_NET_ERROR_EHOSTUNREACH:  return EHOSTUNREACH;
        default:                          return EIO;
    }
}

static int check(int ret)
{
    if (ret < 0)
    {
        errno = sce_net_errno(ret);
        return -1;
    }
    return ret;
}

/* ---- address conversion ------------------------------------------------
 * Only AF_INET is real; struct sockaddr_in6 exists solely so the (dead,
 * #ifdef IPV6-gated) IPv6 branches in the psyBNC sources still compile. */

static void to_sce_sin(const struct sockaddr *addr, SceNetSockaddrIn *out)
{
    const struct sockaddr_in *a = (const struct sockaddr_in *)addr;
    memset(out, 0, sizeof(*out));
    out->sin_len = sizeof(*out);
    out->sin_family = SCE_NET_AF_INET;
    out->sin_port = a->sin_port;
    out->sin_addr.s_addr = a->sin_addr.s_addr;
}

static void from_sce_sin(const SceNetSockaddrIn *in, struct sockaddr *addr, socklen_t *addrlen)
{
    struct sockaddr_in out;
    memset(&out, 0, sizeof(out));
    out.sin_len = sizeof(out);
    out.sin_family = AF_INET;
    out.sin_port = in->sin_port;
    out.sin_addr.s_addr = in->sin_addr.s_addr;
    socklen_t n = sizeof(out);
    if (addr)
    {
        if (addrlen && *addrlen < n) n = *addrlen;
        memcpy(addr, &out, n);
    }
    if (addrlen) *addrlen = sizeof(out);
}

/* ---- sockets ------------------------------------------------------------ */

int vita_socket(int domain, int type, int protocol)
{
    return check(sceNetSocket("psybnc", domain, type, protocol));
}

int vita_bind(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    (void)addrlen;
    SceNetSockaddrIn sin;
    to_sce_sin(addr, &sin);
    return check(sceNetBind(fd, (SceNetSockaddr *)&sin, sizeof(sin)));
}

int vita_listen(int fd, int backlog)
{
    return check(sceNetListen(fd, backlog));
}

int vita_accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    SceNetSockaddrIn sin;
    unsigned int len = sizeof(sin);
    memset(&sin, 0, sizeof(sin));
    int newfd = sceNetAccept(fd, (SceNetSockaddr *)&sin, &len);
    if (newfd < 0)
    {
        errno = sce_net_errno(newfd);
        return -1;
    }
    if (addr) from_sce_sin(&sin, addr, addrlen);
    return newfd;
}

int vita_connect(int fd, const struct sockaddr *addr, socklen_t addrlen)
{
    (void)addrlen;
    SceNetSockaddrIn sin;
    to_sce_sin(addr, &sin);
    return check(sceNetConnect(fd, (SceNetSockaddr *)&sin, sizeof(sin)));
}

int vita_getsockname(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    SceNetSockaddrIn sin;
    unsigned int len = sizeof(sin);
    memset(&sin, 0, sizeof(sin));
    int rc = sceNetGetsockname(fd, (SceNetSockaddr *)&sin, &len);
    if (rc < 0) { errno = sce_net_errno(rc); return -1; }
    from_sce_sin(&sin, addr, addrlen);
    return 0;
}

int vita_getpeername(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    SceNetSockaddrIn sin;
    unsigned int len = sizeof(sin);
    memset(&sin, 0, sizeof(sin));
    int rc = sceNetGetpeername(fd, (SceNetSockaddr *)&sin, &len);
    if (rc < 0) { errno = sce_net_errno(rc); return -1; }
    from_sce_sin(&sin, addr, addrlen);
    return 0;
}

int vita_setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    return check(sceNetSetsockopt(fd, level, optname, optval, optlen));
}

int vita_getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    return check(sceNetGetsockopt(fd, level, optname, optval, optlen));
}

int vita_shutdown(int fd, int how)
{
    return check(sceNetShutdown(fd, how));
}

int vita_socket_close(int fd)
{
    return check(sceNetSocketClose(fd));
}

ssize_t vita_send(int fd, const void *buf, size_t len, int flags)
{
    return check(sceNetSend(fd, buf, len, flags));
}

ssize_t vita_recv(int fd, void *buf, size_t len, int flags)
{
    return check(sceNetRecv(fd, buf, len, flags));
}

ssize_t vita_sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
{
    (void)tolen;
    SceNetSockaddrIn sin;
    to_sce_sin(to, &sin);
    return check(sceNetSendto(fd, buf, len, flags, (SceNetSockaddr *)&sin, sizeof(sin)));
}

ssize_t vita_recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    SceNetSockaddrIn sin;
    unsigned int slen = sizeof(sin);
    memset(&sin, 0, sizeof(sin));
    int ret = sceNetRecvfrom(fd, buf, len, flags, (SceNetSockaddr *)&sin, &slen);
    if (ret < 0) { errno = sce_net_errno(ret); return -1; }
    if (from) from_sce_sin(&sin, from, fromlen);
    return ret;
}

ssize_t vita_read(int fd, void *buf, size_t len)
{
    return vita_recv(fd, buf, len, 0);
}

ssize_t vita_write(int fd, const void *buf, size_t len)
{
    return vita_send(fd, buf, len, 0);
}

int vita_fcntl(int fd, int cmd, ...)
{
    va_list ap;
    va_start(ap, cmd);
    int result;
    if (cmd == F_SETFL)
    {
        int flags = va_arg(ap, int);
        int nb = (flags & O_NONBLOCK) ? 1 : 0;
        result = check(sceNetSetsockopt(fd, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nb, sizeof(nb)));
    }
    else if (cmd == F_GETFL)
    {
        int nb = 0;
        unsigned int len = sizeof(nb);
        if (sceNetGetsockopt(fd, SCE_NET_SOL_SOCKET, SCE_NET_SO_NBIO, &nb, &len) < 0)
            result = 0;
        else
            result = nb ? O_NONBLOCK : 0;
    }
    else if (cmd == F_GETFD)
    {
        int type = 0;
        unsigned int len = sizeof(type);
        if (sceNetGetsockopt(fd, SCE_NET_SOL_SOCKET, SCE_NET_SO_TYPE, &type, &len) < 0)
        {
            errno = EBADF;
            result = -1;
        }
        else
        {
            result = 0;
        }
    }
    else if (cmd == F_SETFD)
    {
        result = 0; /* close-on-exec has no meaning here */
    }
    else
    {
        result = 0;
    }
    va_end(ap);
    return result;
}

/* ---- select() via sceNetEpoll -------------------------------------------
 * The Vita has no select()/poll() at all; sceNetEpoll* is the only
 * multiplexing primitive SceNet exposes. We build a throwaway epoll set on
 * every call, which is wasteful but correct, and matches how psyBNC uses
 * select() (once per main-loop iteration). */

int vita_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    if (nfds <= 0)
    {
        if (readfds) FD_ZERO(readfds);
        if (writefds) FD_ZERO(writefds);
        if (exceptfds) FD_ZERO(exceptfds);
        return 0;
    }

    int eid = sceNetEpollCreate("psybnc_select", 0);
    if (eid < 0)
    {
        errno = EIO;
        return -1;
    }

    int watched = 0;
    for (int fd = 0; fd < nfds; fd++)
    {
        unsigned int ev = 0;
        if (readfds && FD_ISSET(fd, readfds)) ev |= SCE_NET_EPOLLIN;
        if (writefds && FD_ISSET(fd, writefds)) ev |= SCE_NET_EPOLLOUT;
        if (ev == 0) continue;
        SceNetEpollEvent event;
        memset(&event, 0, sizeof(event));
        event.events = ev;
        event.data.fd = fd;
        if (sceNetEpollControl(eid, SCE_NET_EPOLL_CTL_ADD, fd, &event) == 0)
            watched++;
    }

    fd_set out_r, out_w, out_e;
    FD_ZERO(&out_r);
    FD_ZERO(&out_w);
    FD_ZERO(&out_e);

    int ready = 0;

    if (watched > 0)
    {
        SceNetEpollEvent *events = (SceNetEpollEvent *)malloc(sizeof(SceNetEpollEvent) * watched);
        if (events != NULL)
        {
            int timeout_us = -1;
            if (timeout != NULL)
                timeout_us = (int)(timeout->tv_sec * 1000000L + timeout->tv_usec);
            int n = sceNetEpollWait(eid, events, watched, timeout_us);
            if (n > 0)
            {
                for (int i = 0; i < n; i++)
                {
                    int fd = events[i].data.fd;
                    unsigned int e = events[i].events;
                    if (e & (SCE_NET_EPOLLIN | SCE_NET_EPOLLHUP | SCE_NET_EPOLLERR))
                        FD_SET(fd, &out_r);
                    if (e & SCE_NET_EPOLLOUT)
                        FD_SET(fd, &out_w);
                }
                ready = n;
            }
            free(events);
        }
    }
    else if (timeout != NULL)
    {
        sceKernelDelayThread(timeout->tv_sec * 1000000L + timeout->tv_usec);
    }

    sceNetEpollDestroy(eid);

    if (readfds) *readfds = out_r;
    if (writefds) *writefds = out_w;
    if (exceptfds) *exceptfds = out_e;

    return ready;
}

/* ---- inet_* / htons family ------------------------------------------- */

unsigned short vita_htons(unsigned short v) { return sceNetHtons(v); }
unsigned short vita_ntohs(unsigned short v) { return sceNetNtohs(v); }
unsigned int   vita_htonl(unsigned int v)   { return sceNetHtonl(v); }
unsigned int   vita_ntohl(unsigned int v)   { return sceNetNtohl(v); }

unsigned int vita_inet_addr(const char *cp)
{
    SceNetInAddr addr;
    if (sceNetInetPton(SCE_NET_AF_INET, cp, &addr) == 1)
        return addr.s_addr;
    return INADDR_NONE;
}

char *vita_inet_ntoa(struct in_addr in)
{
    static char buf[INET_ADDRSTRLEN];
    SceNetInAddr a;
    a.s_addr = in.s_addr;
    if (sceNetInetNtop(SCE_NET_AF_INET, &a, buf, sizeof(buf)) == NULL)
        strncpy(buf, "0.0.0.0", sizeof(buf));
    return buf;
}

const char *vita_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    if (af == AF_INET)
        return sceNetInetNtop(SCE_NET_AF_INET, src, dst, size);
    if (size > 0) dst[0] = 0;
    return dst;
}

int vita_inet_pton(int af, const char *src, void *dst)
{
    if (af == AF_INET)
        return sceNetInetPton(SCE_NET_AF_INET, src, dst);
    return 0;
}

int vita_inet_aton(const char *cp, struct in_addr *inp)
{
    SceNetInAddr addr;
    if (sceNetInetPton(SCE_NET_AF_INET, cp, &addr) != 1)
        return 0;
    inp->s_addr = addr.s_addr;
    return 1;
}

/* ---- resolver (getaddrinfo / getnameinfo) ------------------------------
 * psyBNC's own DNS layer (src/p_dns.c) always resolves through the system
 * getaddrinfo()/getnameinfo() - c-ares is only used there as fd-plumbing
 * for an async engine that never actually issues a query. We replace both
 * with a small blocking sceNetResolver wrapper. */

int vita_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    (void)service;
    if (node == NULL || res == NULL)
        return EAI_NONAME;
    if (hints != NULL && hints->ai_family == AF_INET6)
        return EAI_FAMILY;

    SceNetInAddr addr;
    memset(&addr, 0, sizeof(addr));

    if (sceNetInetPton(SCE_NET_AF_INET, node, &addr) != 1)
    {
        int rid = sceNetResolverCreate("psybnc_res", NULL, 0);
        if (rid < 0)
            return EAI_FAIL;
        int rc = sceNetResolverStartNtoa(rid, node, &addr, 2000000, 3, 0);
        sceNetResolverDestroy(rid);
        if (rc < 0)
            return EAI_NONAME;
    }

    struct addrinfo *ai = (struct addrinfo *)calloc(1, sizeof(struct addrinfo));
    struct sockaddr_in *sin = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in));
    if (ai == NULL || sin == NULL)
    {
        free(ai);
        free(sin);
        return EAI_MEMORY;
    }
    sin->sin_len = sizeof(*sin);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = addr.s_addr;
    sin->sin_port = 0;

    ai->ai_family = AF_INET;
    ai->ai_socktype = hints ? hints->ai_socktype : SOCK_STREAM;
    ai->ai_protocol = IPPROTO_TCP;
    ai->ai_addrlen = sizeof(*sin);
    ai->ai_addr = (struct sockaddr *)sin;
    ai->ai_canonname = NULL;
    ai->ai_next = NULL;

    *res = ai;
    return 0;
}

void vita_freeaddrinfo(struct addrinfo *res)
{
    while (res != NULL)
    {
        struct addrinfo *next = res->ai_next;
        free(res->ai_addr);
        free(res);
        res = next;
    }
}

const char *vita_gai_strerror(int errcode)
{
    switch (errcode)
    {
        case 0:           return "Success";
        case EAI_NONAME:  return "Name or service not known";
        case EAI_FAMILY:  return "Address family not supported";
        case EAI_AGAIN:   return "Temporary resolution failure";
        case EAI_FAIL:    return "Non-recoverable resolution failure";
        case EAI_MEMORY:  return "Out of memory";
        default:          return "Unknown resolver error";
    }
}

int vita_getnameinfo(const struct sockaddr *addr, socklen_t addrlen, char *host, socklen_t hostlen,
                      char *serv, socklen_t servlen, int flags)
{
    (void)addrlen;
    (void)serv;
    (void)servlen;
    (void)flags;
    if (addr == NULL || host == NULL)
        return EAI_FAIL;

    const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
    SceNetInAddr a;
    a.s_addr = sin->sin_addr.s_addr;

    int rid = sceNetResolverCreate("psybnc_rres", NULL, 0);
    if (rid < 0)
        return EAI_FAIL;
    int rc = sceNetResolverStartAton(rid, &a, host, hostlen, 2000000, 3, 0);
    sceNetResolverDestroy(rid);
    if (rc < 0)
        return EAI_NONAME;
    return 0;
}
