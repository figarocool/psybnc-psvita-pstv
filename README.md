# psyBNC - IRC Bouncer per PS Vita / PSTV

*[English version below](#psybnc---irc-bouncer-for-ps-vita--pstv-english)*

**psyBNC** è lo storico e potentissimo IRC Bouncer (proxy IRC) originariamente scritto in C. Questo repository lo porta nativamente su **PS Vita e PSTV homebrew**, tramite [VitaSDK](https://vitasdk.org/).

Trasforma la tua PS Vita (o PSTV) in un bouncer IRC always-on sulla tua rete locale:

- All'avvio mostra a schermo l'IP della console e la porta del bouncer (`31337` di default).
- Nessuna password precompilata: **la prima persona che si connette e invia `/PASS unapassword` diventa amministratore**, esattamente come nel psyBNC originale.
- Sorgente C praticamente invariato: il porting aggiunge solo un layer di compatibilità socket (`vita/compat/`) che traduce le chiamate BSD standard verso l'API nativa `SceNet` della console.

📖 **Documentazione completa, build, installazione e limiti noti**: [`vita/README.md`](vita/README.md)
📦 **Download**: vedi la [ultima release](../../releases/latest) per il file `.vpk` pronto all'uso.

## Funzionalità storiche di psyBNC

- Gestione messaggi privati offline, gestione canali/topic
- Supporto DCC completo (SEND/GET e chat dirette)
- Multi-client e multi-server IRC contemporanei
- Gestione VHOST e PROXY
- Oltre 200 comandi amministrativi interni (`/BHELP` per la lista completa)

## Licenza

Il codice sorgente originale di psyBNC e le modifiche apportate in questo repository sono rilasciate sotto licenza **GNU General Public License v2 (GPLv2)**. Consulta il file `COPYING` per i dettagli completi.

---

# psyBNC - IRC Bouncer for PS Vita / PSTV (English)

*[Versione italiana sopra](#psybnc---irc-bouncer-per-ps-vita--pstv)*

**psyBNC** is the classic, powerful C-based IRC bouncer. This repository ports it natively to **PS Vita and PSTV homebrew**, built with [VitaSDK](https://vitasdk.org/).

Turns your PS Vita (or PSTV) into an always-on IRC bouncer on your local network:

- Prints the console's IP and the bouncer's port (`31337` by default) on screen at startup.
- No password baked in: **the first person to connect and send `/PASS somepassword` becomes the administrator**, exactly like in the original psyBNC.
- The C source is left almost untouched: the port only adds a socket compatibility layer (`vita/compat/`) that translates standard BSD calls into the console's native `SceNet` API.

📖 **Full documentation, build, install and known limitations**: [`vita/README.md`](vita/README.md)
📦 **Download**: see the [latest release](../../releases/latest) for a ready-to-install `.vpk`.

## psyBNC's classic features

- Offline private message logging, channel/topic tracking
- Full DCC support (SEND/GET and direct chat)
- Multiple simultaneous clients and IRC servers
- VHOST and PROXY support
- 200+ built-in admin commands (`/BHELP` for the full list)

## License

psyBNC's original source code and the changes made in this repository are released under the **GNU General Public License v2 (GPLv2)**. See `COPYING` for full details.
