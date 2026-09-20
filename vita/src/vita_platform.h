#ifndef _VITA_PLATFORM_H_
#define _VITA_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Called once at the very top of psybnc's main(): brings up the debug
 * screen, the network, creates ux0:data/psybnc/... and seeds it with the
 * bundled lang/help/motd files on first run. */
void vita_platform_init(void);

/* Called right after psyBNC successfully opens its listening socket:
 * prints the console's IP address and bouncer port on screen. */
void vita_platform_show_status(int port);

#ifdef __cplusplus
}
#endif

#endif
