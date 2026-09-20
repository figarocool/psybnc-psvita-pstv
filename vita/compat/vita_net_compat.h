#ifndef _VITA_NET_COMPAT_H_
#define _VITA_NET_COMPAT_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* App-level helpers used by vita/src/main.c (not part of the BSD shim). */
int  vita_net_init(void);
void vita_net_term(void);
int  vita_net_wait_connected(int timeout_seconds);
int  vita_net_get_ip(char *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif
