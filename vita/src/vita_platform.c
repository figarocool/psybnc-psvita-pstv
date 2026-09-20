/*
 * PS Vita bootstrap for psyBNC: brings up the network, exposes
 * ux0:data/psybnc/ as the bouncer's home directory, mirrors stdout/stderr
 * onto the debug screen (so the countless existing printf()/p_log() calls
 * in the original codebase become visible without touching every call
 * site), and shows the console's IP:port once the bouncer is listening.
 *
 * Deliberately does NOT include the sys/socket.h compat shim: nothing
 * here needs it, and it keeps the read()/write()/close() macros defined
 * there from ever seeing this file's real sceIo-based file access.
 */
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <psp2/ctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debugScreen.h"
#include "vita_net_compat.h"
#include "vita_platform.h"

#define PSYBNC_BASE "ux0:data/psybnc"

static int s_net_ready = 0;

/* ---- stdout/stderr -> debug screen -------------------------------------
 * newlib's default _write() just forwards to sceIoWrite(); psyBNC prints
 * its startup banner, config errors and log lines with plain printf(),
 * which would otherwise go nowhere visible. Overriding this one libc hook
 * makes all of that show up on screen for free. */
int _write(int fd, const char *buf, int size)
{
    if (fd == 1 || fd == 2)
    {
        psvDebugScreenPrintf("%.*s", size, buf);
        return size;
    }
    return sceIoWrite(fd, buf, size);
}

/* Every exit() in the original code (missing config, language file, port
 * already in use, ...) would otherwise close the app the instant it
 * prints its error, before anyone can read the screen. */
static void vita_pause_before_exit(void)
{
    psvDebugScreenPrintf("\n\nPremi START per uscire.\n");
    for (int i = 0; i < 60 * 10; i++) /* ~60s safety timeout */
    {
        SceCtrlData pad;
        memset(&pad, 0, sizeof(pad));
        sceCtrlPeekBufferPositive(0, &pad, 1);
        if (pad.buttons & SCE_CTRL_START)
            break;
        sceKernelDelayThread(100 * 1000);
    }
}

static void ensure_dir(const char *path)
{
    SceUID d = sceIoDopen(path);
    if (d >= 0)
    {
        sceIoDclose(d);
        return;
    }
    sceIoMkdir(path, 0777);
}

static void copy_file(const char *src, const char *dst)
{
    SceUID in = sceIoOpen(src, SCE_O_RDONLY, 0);
    if (in < 0) return;
    SceUID out = sceIoOpen(dst, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0666);
    if (out < 0) { sceIoClose(in); return; }
    char buf[4096];
    int n;
    while ((n = sceIoRead(in, buf, sizeof(buf))) > 0)
        sceIoWrite(out, buf, n);
    sceIoClose(out);
    sceIoClose(in);
}

/* Copies every file from a bundled read-only VPK folder (app0:assets/...)
 * into its writable ux0:data/psybnc/... counterpart, but only the first
 * time: if the destination directory already exists we leave it alone so
 * a user's edited language file or motd survives an app update. */
static void seed_from_bundle(const char *bundle_dir, const char *dest_dir)
{
    SceUID probe = sceIoDopen(dest_dir);
    if (probe >= 0)
    {
        sceIoDclose(probe);
        return; /* already provisioned */
    }
    sceIoMkdir(dest_dir, 0777);

    SceUID d = sceIoDopen(bundle_dir);
    if (d < 0) return;
    SceIoDirent entry;
    memset(&entry, 0, sizeof(entry));
    while (sceIoDread(d, &entry) > 0)
    {
        if (SCE_S_ISDIR(entry.d_stat.st_mode)) continue;
        char src[512], dst[512];
        snprintf(src, sizeof(src), "%s/%s", bundle_dir, entry.d_name);
        snprintf(dst, sizeof(dst), "%s/%s", dest_dir, entry.d_name);
        copy_file(src, dst);
        memset(&entry, 0, sizeof(entry));
    }
    sceIoDclose(d);
}

void vita_platform_init(void)
{
    psvDebugScreenInit();
    psvDebugScreenPrintf("psyBNC per PS Vita - avvio in corso...\n");

    vita_net_init();
    psvDebugScreenPrintf("In attesa della connessione di rete");
    for (int i = 0; i < 15 && !s_net_ready; i++)
    {
        if (vita_net_wait_connected(1)) { s_net_ready = 1; break; }
        psvDebugScreenPrintf(".");
    }
    psvDebugScreenPrintf("\n");
    if (!s_net_ready)
        psvDebugScreenPrintf("ATTENZIONE: nessuna connessione di rete rilevata (Wi-Fi spento?)\n");

    ensure_dir("ux0:data");
    ensure_dir(PSYBNC_BASE);
    ensure_dir(PSYBNC_BASE "/log");
    ensure_dir(PSYBNC_BASE "/key");
    ensure_dir(PSYBNC_BASE "/dccfiles");

    seed_from_bundle("app0:assets/lang", PSYBNC_BASE "/lang");
    seed_from_bundle("app0:assets/help", PSYBNC_BASE "/help");
    seed_from_bundle("app0:assets/motd", PSYBNC_BASE "/motd");

    setenv("PSYBNC_BASE_DIR", PSYBNC_BASE, 1);
    setenv("PSYBNC_DOWNLOAD_DIR", PSYBNC_BASE "/dccfiles", 1);
    setenv("PSYBNC_NOFORK", "1", 1);

    atexit(vita_pause_before_exit);

    SceIoStat st;
    memset(&st, 0, sizeof(st));
    if (sceIoGetstat(PSYBNC_BASE "/psybnc.conf", &st) < 0)
    {
        /* First run: seed the default conf (port 31337, listen on all
         * interfaces). No user/password is baked in here - psyBNC's own
         * bootstrap flow (firstuser() in p_client.c) makes the first
         * person to connect and send /PASS the admin, using whatever
         * password they send. */
        copy_file("app0:assets/conf/psybnc.conf", PSYBNC_BASE "/psybnc.conf");
        psvDebugScreenPrintf("\nConfigurazione di default creata in %s/psybnc.conf\n", PSYBNC_BASE);
        psvDebugScreenPrintf("Collegati per PRIMO con un client IRC e invia /PASS <password>\n");
        psvDebugScreenPrintf("per diventare admin del bouncer.\n");
    }
}

void vita_platform_show_status(int port)
{
    char ip[32];
    vita_net_get_ip(ip, sizeof(ip));
    psvDebugScreenPrintf("\n===============================================\n");
    psvDebugScreenPrintf(" psyBNC in ascolto\n");
    psvDebugScreenPrintf(" IP:   %s\n", s_net_ready ? ip : "(rete non connessa)");
    psvDebugScreenPrintf(" Porta: %d\n", port);
    psvDebugScreenPrintf("===============================================\n\n");
}
