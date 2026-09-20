# psyBNC per PS Vita / PSTV

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
