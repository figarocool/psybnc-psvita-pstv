/*
 * A handful of libc calls that vitasdk's newlib *declares* (so the
 * original psyBNC sources compile untouched) but doesn't actually
 * implement, because they describe OS facilities the Vita doesn't have
 * (process signal masks, file permission bits). Everywhere psyBNC calls
 * these it's for best-effort hardening (ignore SIGPIPE, restrict a
 * created file's mode, ...), never for anything the bouncer actually
 * depends on, so a no-op/success stub is the correct behavior here.
 */
#include <sys/types.h>
#include <signal.h>

mode_t umask(mode_t mask)
{
    (void)mask;
    return 0;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    (void)signum;
    (void)act;
    (void)oldact;
    return 0;
}
