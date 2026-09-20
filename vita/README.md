# psyBNC per PS Vita / PSTV

*[English version below](#psybnc-for-ps-vita--pstv-english)*

Porting di psyBNC 2.3.2 per PS Vita/PSTV homebrew, tramite [VitaSDK](https://vitasdk.org/).

Il sorgente C di psyBNC (`../src`) è quasi invariato: le poche differenze necessarie
sul codice originale sono racchiuse in blocchi `#ifdef VITA` (fork disabilitato,
niente esecuzione di programmi esterni, DNS via `sceNetResolver`, niente signal
handler hardware). Il grosso del lavoro è in questa cartella:

- `compat/` — layer che traduce le socket BSD standard usate da psyBNC verso
  l'API nativa `SceNet` della Vita (che non ha `select()`/`fd_set`, usa
  `sceNetEpoll*` al loro posto, e strutture indirizzo diverse), più
  `getaddrinfo()`/`getnameinfo()` implementati su `sceNetResolver`.
- `src/vita_platform.c` — bootstrap dell'app: inizializza rete/schermo, crea
  `ux0:data/psybnc/`, copia lingua/help/motd/config di default al primo avvio,
  mostra IP e porta a schermo.
- `data_template/psybnc.conf` — configurazione di default (porta 31337, in
  ascolto su tutte le interfacce).

## Build

Richiede [VitaSDK](https://vitasdk.org/) installato con `VITASDK` impostata:

```sh
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
cd vita
mkdir -p build && cd build
cmake ..
make
```

Il pacchetto installabile viene generato in `build/psybnc_vita.vpk`. Una copia
già compilata si trova in [`dist/psybnc_vita.vpk`](dist/psybnc_vita.vpk).

## Installazione e primo avvio

1. Installa `psybnc_vita.vpk` con VitaShell.
2. Avvia l'app: mostra a schermo l'IP della Vita e la porta (31337 di default),
   e crea `ux0:data/psybnc/psybnc.conf` al primo avvio se non esiste già.
3. **Collegati per primo** con un client IRC a `IP_DELLA_VITA:31337` e invia

   ```
   /PASS unapasswordatuascelta
   ```

   Questo è il comportamento nativo di psyBNC (`firstuser()` in
   `../src/p_client.c`): la prima persona che si connette e invia `/PASS`
   diventa amministratore del bouncer con quella password. Da lì puoi usare
   i comandi admin (`/BHELP`, `/ADDSERVER`, `/JUMP`, ...) per scegliere a
   quale rete/server IRC collegarti.

## Limiti noti

- Niente IPv6 (disabilitato già in `config.h`, la Vita non lo supporta bene).
- Niente esecuzione di script/programmi esterni (impossibile su Vita: nessun
  `fork()`/`exec()`/shell per le app homebrew).
- Chiudere l'app ferma il bouncer: nessun modo per farlo girare in background
  su Vita homebrew.
- Se la memory card/SD2Vita è formattata FAT32, i singoli file DCC (es. film
  ricevuti) sono limitati a 4GB.
- Non ancora validato a fondo su hardware reale in scenari di uso prolungato;
  segnalare eventuali problemi.

---

# psyBNC for PS Vita / PSTV (English)

*[Versione italiana sopra](#psybnc-per-ps-vita--pstv)*

A port of psyBNC 2.3.2 to PS Vita/PSTV homebrew, built with [VitaSDK](https://vitasdk.org/).

psyBNC's original C source (`../src`) is left almost untouched: the handful of
changes needed on top of it are wrapped in targeted `#ifdef VITA` blocks
(fork disabled, no external program execution, DNS via `sceNetResolver`, no
hardware signal handlers). Most of the actual work lives in this folder:

- `compat/` — a layer that translates the standard BSD sockets psyBNC uses
  into the Vita's native `SceNet` API (which has no `select()`/`fd_set` at
  all — it uses `sceNetEpoll*` instead — and different address structs),
  plus `getaddrinfo()`/`getnameinfo()` implemented on top of
  `sceNetResolver`.
- `src/vita_platform.c` — the app's bootstrap: brings up networking and the
  debug screen, creates `ux0:data/psybnc/`, seeds it with default
  language/help/motd/config files on first run, and prints the IP and port
  on screen.
- `data_template/psybnc.conf` — the default configuration (port 31337,
  listening on all interfaces).

## Building

Requires [VitaSDK](https://vitasdk.org/) installed, with `VITASDK` set:

```sh
export VITASDK=/usr/local/vitasdk
export PATH=$VITASDK/bin:$PATH
cd vita
mkdir -p build && cd build
cmake ..
make
```

The installable package is produced at `build/psybnc_vita.vpk`. A prebuilt
copy is checked in at [`dist/psybnc_vita.vpk`](dist/psybnc_vita.vpk).

## Installing and first run

1. Install `psybnc_vita.vpk` with VitaShell.
2. Launch the app: it prints the Vita's IP and port (31337 by default) on
   screen, and creates `ux0:data/psybnc/psybnc.conf` on first run if it
   doesn't already exist.
3. **Connect first, yourself**, with an IRC client to `VITA_IP:31337` and send

   ```
   /PASS somepasswordofyourchoice
   ```

   This is psyBNC's native behavior (`firstuser()` in `../src/p_client.c`):
   the first person to connect and send `/PASS` becomes the bouncer's
   administrator with that password. From there you can use the admin
   commands (`/BHELP`, `/ADDSERVER`, `/JUMP`, ...) to pick which IRC
   network/server to connect through.

## Known limitations

- No IPv6 (already disabled in `config.h` — the Vita doesn't support it well).
- No external script/program execution (impossible on the Vita: homebrew
  apps have no `fork()`/`exec()`/shell).
- Closing the app stops the bouncer: there is no way to run it in the
  background on Vita homebrew.
- If your memory card/SD2Vita is formatted FAT32, individual DCC files
  (e.g. received movies) are capped at 4GB.
- Not yet heavily validated on real hardware under long-running usage;
  please report any issues you hit.
