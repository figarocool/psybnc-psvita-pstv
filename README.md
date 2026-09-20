# psyBNC - IRC Bouncer per PS Vita / PSTV (e Android)

*[English version below](#psybnc---irc-bouncer-for-ps-vita--pstv-and-android-english)*

**psyBNC** è lo storico e potentissimo IRC Bouncer (proxy IRC) originariamente scritto in C. Questo repository lo porta nativamente su **PS Vita e PSTV homebrew**, oltre a contenere anche il porting su **Android** realizzato in precedenza.

## Porting PS Vita / PSTV

Il porting principale di questo repository. Trasforma la tua PS Vita (o PSTV) in un bouncer IRC always-on sulla tua rete locale, usando [VitaSDK](https://vitasdk.org/).

- All'avvio mostra a schermo l'IP della console e la porta del bouncer (`31337` di default).
- Nessuna password precompilata: **la prima persona che si connette e invia `/PASS unapassword` diventa amministratore**, esattamente come nel psyBNC originale.
- Sorgente C praticamente invariato: il porting aggiunge solo un layer di compatibilità socket (`vita/compat/`) che traduce le chiamate BSD standard verso l'API nativa `SceNet` della console.

📖 **Documentazione completa, build, installazione e limiti noti**: [`vita/README.md`](vita/README.md)
📦 **Download**: vedi la [ultima release](../../releases/latest) per il file `.vpk` pronto all'uso.

## Porting Android

Questo repository contiene anche il porting Android (cartella `android/`), realizzato tramite Android NDK: fa girare il core nativo di psyBNC su smartphone/tablet con un Foreground Service per restare online 24/7.

- Interfaccia grafica nativa, notifiche Android, Storage Access Framework per i download DCC.
- Compilazione: apri `android/` con Android Studio (richiede Android NDK + CMake dall'SDK Manager).
- Stesso flusso di primo utilizzo: la prima password inviata al bouncer diventa quella dell'amministratore.
- Se cerchi la vecchia versione sperimentale in Basic4Android (B4A), è nel branch `b4a-legacy`.

## Funzionalità storiche di psyBNC (comuni a entrambi i porting)

- Gestione messaggi privati offline, gestione canali/topic
- Supporto DCC completo (SEND/GET e chat dirette)
- Multi-client e multi-server IRC contemporanei
- Gestione VHOST e PROXY
- Oltre 200 comandi amministrativi interni (`/BHELP` per la lista completa)

## Crediti e licenza

**Porting Android**: Stefano Basile (contatti tramite GitHub)
**Porting PS Vita/PSTV**: vedi [`vita/README.md`](vita/README.md)

Il codice sorgente originale di psyBNC e le modifiche apportate in questo repository sono rilasciate sotto licenza **GNU General Public License v2 (GPLv2)**. Consulta il file `COPYING` per i dettagli completi.

---

# psyBNC - IRC Bouncer for PS Vita / PSTV (and Android) (English)

*[Versione italiana sopra](#psybnc---irc-bouncer-per-ps-vita--pstv-e-android)*

**psyBNC** is the classic, powerful C-based IRC bouncer. This repository ports it natively to **PS Vita and PSTV homebrew**, and also contains an earlier **Android** port.

## PS Vita / PSTV port

The main port in this repository. Turns your PS Vita (or PSTV) into an always-on IRC bouncer on your local network, built with [VitaSDK](https://vitasdk.org/).

- Prints the console's IP and the bouncer's port (`31337` by default) on screen at startup.
- No password baked in: **the first person to connect and send `/PASS somepassword` becomes the administrator**, exactly like in the original psyBNC.
- The C source is left almost untouched: the port only adds a socket compatibility layer (`vita/compat/`) that translates standard BSD calls into the console's native `SceNet` API.

📖 **Full documentation, build, install and known limitations**: [`vita/README.md`](vita/README.md)
📦 **Download**: see the [latest release](../../releases/latest) for a ready-to-install `.vpk`.

## Android port

This repository also contains the Android port (`android/` folder), built with the Android NDK: it runs psyBNC's native core on phones/tablets with a Foreground Service to stay online 24/7.

- Native UI, Android notifications, Storage Access Framework for DCC downloads.
- Build: open `android/` with Android Studio (requires the Android NDK + CMake from the SDK Manager).
- Same first-run flow: the first password sent to the bouncer becomes the administrator's.
- Looking for the old experimental Basic4Android (B4A) version? It's on the `b4a-legacy` branch.

## psyBNC's classic features (shared by both ports)

- Offline private message logging, channel/topic tracking
- Full DCC support (SEND/GET and direct chat)
- Multiple simultaneous clients and IRC servers
- VHOST and PROXY support
- 200+ built-in admin commands (`/BHELP` for the full list)

## Credits and license

**Android port**: Stefano Basile (contact via GitHub)
**PS Vita/PSTV port**: see [`vita/README.md`](vita/README.md)

psyBNC's original source code and the changes made in this repository are released under the **GNU General Public License v2 (GPLv2)**. See `COPYING` for full details.
