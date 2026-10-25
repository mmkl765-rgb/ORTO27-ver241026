/*

FILE PURAMENTE TESTUALE che tengo come storico delle versioni precedenti
il software di partenza è quello della versione 2025 con tutti i suoi pro e contro.
ho tentato di inserire nel sistema versione 2026 l'interazione con le radio l'ora ma può dei problemi di comunicazione fra have software serial
interazione sicuramente da rivedere perché non voglio buttare il progetto in quanto ho già scritto tutto il protocollo di comunicazione


060526 => sistema funzionante completo, rimodulazione tempi di irrigazione (z2 = 2min // z1 = 1,5 min)
          Lo sketch usa 17740 byte (42%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 657 byte (33%) di memoria dinamica, lasciando altri 1537 byte liberi per le variabili locali. Il massimo è 2048 byte.

070526 => migliorato gestione avvio con   ottimizzazione buffer scrittura
          Lo sketch usa 13548 byte (42%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 522 byte (27%) di memoria dinamica, lasciando altri 1537 byte liberi per le variabili locali. Il massimo è 2048 byte.

080526 => migliorata la gestione memoria con l'uso completo di messaggi in flash
          Lo sketch usa 12754 byte (39%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 511 byte (24%) di memoria dinamica, lasciando altri 1537 byte liberi per le variabili locali. Il massimo è 2048 byte.

090526 => riconfigurata la parte dello show dati, da migliorare ancora se mi viene qualche idea tipo testo scorrevole..
          effettuate sistemazioni codice in loco (livello acqua che non andava e tempi irrigazione)  --  VERSIONE STABILE 1  --
          Lo sketch usa 13168 byte (40%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.
          La stessa versione è stata rinominata "ORTO_16_2026_v1-backup" in modo da non toccarla

130526 => risistemazione codice vedendo di rosicchiare ancora qualcosana ma dalla compilazione... rosicchiato nulla !!!
          Lo sketch usa 13010 byte (40%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.
          INSTALLATA IL 13 / 05 / 26

220526 => modificato i dati di settaggio umidita come richiesto da papa' in quanto sembra bagni poco.... impostato un fattore di correzione a -10 punti
          sul valore medio di umidita
          modificati i tempi di irrigazione, aumentati di 30 secondi entrambi
          Lo sketch usa 13010 byte (40%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.\

250526 => modifica scritta display nella modalita ordinaria aggiunto il numero delle irrigazioni. comodo perche l'acqua va giu un po velocemente in cisterna !!!
          modifica della visualizzazione dello storico delle letture con tabella (sperando sia leggibile e comprensibile)
          modifica di parecchi messaggi lcd con "parole" piu appropriate
          modifica grafica della pagina di visualizzazione del sistema in automatico (pagina piu leggibile)
          Lo sketch usa 12712 byte (39%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.

260526 => migliorato il warning visivo nel caso la cisterna sia vuota con introduzione del lampeggio dei led verde - blu intermittente - salta maggiormente
          all'occhio la lampadina che lampeggia rispetto ad una accesa fissa !!!!!
          Lo sketch usa 12726 byte (39%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.

280526 => modificato la tempistica irrigazione (aumentato il tempo) e messo valori dsoglia differenti per zona e fatta inclusione per variabile "versione"
          Lo sketch usa 12718 byte (39%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.

080626 => aggiunto l'irrigazione forzata nelle zone: in ordine avremo irrigazione forzata tutto, irrigazione forzata zolo zona 1, irrigazione forzata zolo zona 2
          Lo sketch usa 12980 byte (40%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 528 byte (25%) di memoria dinamica, lasciando altri 1520 byte liberi per le variabili locali. Il massimo è 2048 byte.
    
200726 => rifatto interamente il sistema con tutte le funzioni non bloccanti, un lavoraccio !!!!! spero funzioni. al momento mancano le scritte sul lcd
          che devo ancora vedere dove e come inserirle usando sempre il wbuff delle versioni precedenti....

040826 => quasi finito il sistema non bloccante, manca il controllo e le ultime scritte sul display. creato l'aggiornamento display temporizzato, gestito tutto
          tramite funzioni. ancora da rifinire, spero funzioni come si deve !!! Inoltre tolta definitivamente la modalita operativa TEST - bella l'idea e la
          realizzazione nelle versioni precedenti ma ho visto che non la uso mai, leggendo i valori del terreno capisco immediatamente se ci sono o no problemi..

050826 => prima compilazione del nuovo fw ed a parte degli errori stupidi (nomi variabili scritti male, qualche ; dimenticato) è andato tutto a buon fine con questo
          output . Lo sketch usa 11146 byte (34%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 563 byte (27%) di memoria dinamica, lasciando altri 1485 byte liberi per le variabili locali. Il massimo è 2048 byte.

080826 => versione ALPHA1 del sistema non bloccante, controllato tutte le funzioni, i tempi, le scritte lcd e la loro tempistica di 
          apparizione e di vita nel display. in teoria va tutto. i dati di tempo, soglie ecc sono gli stessi della versione 080626 che ha
          funzionato alla perfezione nell'orto per 2 mesi (usarla come bakup sw di sicurezza !!!!)
          Lo sketch usa 12598 byte (39%) dello spazio disponibile per i programmi. Il massimo è 32256 byte.
          Le variabili globali usano 712 byte (34%) di memoria dinamica, lasciando altri 1336 byte liberi per le variabili locali. Il massimo è 2048 byte.

180826 => versione ALPHA1 contenente solo il setup per tutto il sistema setup implementato con il server web le avvio connessioni e la modalità Access point e collegamento 
          a router con addio di salvare web chiamato server orto al momento solo la parte del setup ah questi valori di consumo RAM e ROM.
          Lo sketch usa 67100 byte (25%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
          Le variabili globali usano 10144 byte (30%) di memoria dinamica, lasciando altri 22624 byte liberi per le variabili locali. Il massimo è 32768 byte.
        
220826 => versione Alfa due teoricamente completa di tutta la parte web sperando funzionante nella compilazione non mi da errori ma vanno ancora rifinite le pagine 
          web nella loro totalità la parte grafica è accettabile ma vorrei vedere cosa salta fuori sul telefono al momento della compilazione lo utilizzo dei RAM e rom è
          accettabile Lo sketch usa 70924 byte (27%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
          Le variabili globali usano 10144 byte (30%) di memoria dinamica, lasciando altri 22624 byte liberi per le variabili locali. Il massimo è 32768 byte.

250826 => versione DEFINITIVA con porting da r3 a r4 completato, ci sara qualcosa da sistemare sul campo ma in linea di massima è tutto ok, no nci credo nemmeno io !!!
          Lo sketch usa 89932 byte (34%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
          Le variabili globali usano 10204 byte (31%) di memoria dinamica, lasciando altri 22564 byte liberi per le variabili locali. Il massimo è 32768 byte.

******************************************************************************************+
101026 => CAMBIO DI PROGRAMMA: questa versione è la base di partenza del nuov osistema non bloccante con web, shift register e scheda ad 8 rele. manca ncora da 
          fare la lettura dei sensori zona in quanto non so ancora che sensori mettere quindi.... da valutare. struttura portante fatta, manca da riempire l'array !
          da controllare se ci sono delle variabili che non uso, in caso cancellarle. ad oggi non ho fatto ancora una compilazione. controllato logica con gemini.
        
211026 => inserita la finestra irrigazione in atto che viene visualizzata tramite modale e lettura flags irrigazione. cambiato grafica e dati su controllo da tel/pc
          tolto definitivamente la pagina irrigazione. inserito e controllato tutte le variabili web per il controllo (i %TAG%). prva di compilaizone ma da errore
          su funzioni vecchie che probabilmente non ho tolto ovunque...da ricontrollare PULISCI_LCD ! 
          Lo sketch usa 93920 byte (35%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
          Le variabili globali usano 10628 byte (32%) di memoria dinamica, lasciando altri 22140 byte liberi per le variabili locali. Il massimo è 32768 byte.
        
241026 => Versione precedente compilata sembra funzioni tutto, tolto il lcd nella funzione di scrittura che dava errore non avendo collegato lo schermo su i2c.
          Testato con pc e telefono. Possibile mettere un mac protection alla connessione ma è un upgrade !
          Dati finali di compilazione sistema completo : 
          Lo sketch usa 93584 byte (35%) dello spazio disponibile per i programmi. Il massimo è 262144 byte.
          Le variabili globali usano 10628 byte (32%) di memoria dinamica, lasciando altri 22140 byte liberi per le variabili locali. Il massimo è 32768 byte.
        
*/

// VERSIONE OPERATIVA SOFTWARE : per maggiori info leggere le informazioni riportate sopra 
char VERSIONE[7] = "241026"; // FORMATTAZIONE : ggmmaa 
