// FILE CONTENETE TUTTA LA PATRE DELLA GESTIONE WEB, seutp, loop e funzioni associate

  // FUNZIONE CHE AVVIA E SETTA LA PARTE WEB DEL SISTEMA: abilita la scheda, verifica la sua presenza e 
  // funzionalita'. In caso tutto ok, prima cerca di connettersi al router e se non lo trova (fuori
  // campo, spento, non funzionante) attiva AccessPoint per potersi collegare direttamente.
  // Idea per usarlo anche da telefonino come telecomando e visualizzatore generale del sistema.
  // Il setup finisce con l'avvio del server web " Server_Orto "
  // Collegandosi con gli IP fissi (messi sul file SECRET) si visualizza la pagina principale
void SETUP_WEB() { 
  // CONFIGURAZIONE CON IP STATICI 
  lcd.clear();
  strcpy(W_BUFF, " - AVVIO AREA WEB - "); SCRIVI_LCD(0, 0, W_BUFF);

  // se non c'e' la scheda o non funziona...
  if (WiFi.status() == WL_NO_MODULE) { 
    WifiOK = false;
    strcpy(W_BUFF, "ER-CHIP WIFI KO!    "); SCRIVI_LCD(0, 1, W_BUFF);
    strcpy(W_BUFF, "AVVIO SENZA RETE... "); SCRIVI_LCD(0, 2, W_BUFF);
    delay(1500); 
  } else { 
    WifiOK = true; 
  }

  if (WifiOK) {
    // Tentativo connessione al router
    strcpy(W_BUFF, "CONNES. ROUTER");  SCRIVI_LCD(0, 1, W_BUFF);
    
    // APPLICO L'IP FISSO Sintassi: WiFi.config(IP, DNS, GATEWAY, SUBNET)
    WiFi.config(sta_IP, sta_dns, sta_gateway, sta_subnet);

    WiFi.begin(ssid_router, pass_router);  // Tenta il router di casa
    uint8_t tentativi = 0;
    
    while (WiFi.status() != WL_CONNECTED && tentativi < 10) {
      snprintf(W_BUFF, sizeof(W_BUFF), "CONNES. ROUTER %d", tentativi);
      SCRIVI_LCD(0, 1, W_BUFF);
      delay(500);
      tentativi++;
    }

    // se connessione avvenuta con successo ...
    if (WiFi.status() == WL_CONNECTED) {
      strcpy(W_BUFF, "ROUTER CONNESSO");  SCRIVI_LCD(0, 1, W_BUFF);
      snprintf(W_BUFF, sizeof(W_BUFF), "IP %d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      SCRIVI_LCD(0, 2, W_BUFF);
      WIFINET_AP = false;  // bool per sapere il tipo di connessione
      delay(1500);
    } else {
      // Fallito il router -> Attiva l'Access Point locale
      strcpy(W_BUFF, "AVVIO A.P. LOCALE"); 
      SCRIVI_LCD(0, 1, W_BUFF);

       WiFi.beginAP(ssid_AP, pass_AP);
      //WiFi.beginAP("Orto_Smart_AP", "1234567890");

      IPAddress ipApCorrente = WiFi.softAPIP(); 
      snprintf(W_BUFF, sizeof(W_BUFF), "IP %d.%d.%d.%d", ipApCorrente[0], ipApCorrente[1], ipApCorrente[2], ipApCorrente[3]);
      SCRIVI_LCD(0, 2, W_BUFF);

      WIFINET_AP = true;  // bool per sapere il tipo di connessione

      strcpy(W_BUFF, "FINE WEB SETUP"); 
      SCRIVI_LCD(0, 3, W_BUFF);

      delay(1500);
    }
    
    // Avvio server web (va avviato in entrambi i casi)
    Server_Orto.begin();
  }
}




// FUNZIONE CHE GESTISCE IL LOOP DELLA PARTE WEB, funzionando con le variabili globali impostate
// nella parte alta del main.ino... La funzione controlla se c'e' un client collegato (tel, tablet, pc)
// e se risulta richiedere una connessione intercetta la richiesta  GET ed in base a quella carica la pagina
// corretta trasformando i tag tra gli intercettiri %  (es: %TAG%) nel relativo valore o dicitura da visualizzare
// tramite la funzione Converti_TAG(). Basta inserire la funzione dove si vuole mettere la parte web nel main.ino

// da sistemare i messaggi a lcd inserendo qualcosa del tipo "connessione web in atto" con scrivi_lcd(qualcosa da scrivere)

void LOOP_WEB() {
  if (WifiOK) {
    WiFiClient client = Server_Orto.available();
    if (client) {
      CLIENT_PRESENTE = true;             // modifica il bool, se c'e' una richiesta c'e' un client !
      ULTIMO_CONTATTO_CLIENT = millis();   // quando si è connesso 
      Serial.println("Telefono connesso! Elaborazione pagina... ");    
      
      // messaggio a LCD : avvertimento della connessione web e presenza client
      strcpy(W_BUFF, "CONNESSIONE WEB     "); SCRIVI_LCD(0, 0, W_BUFF);
      strcpy(W_BUFF, "VERIFICA CLIENT = OK"); SCRIVI_LCD(0, 1, W_BUFF);
      strcpy(W_BUFF, "--SOLO COMADI WEB-- "); SCRIVI_LCD(0, 2, W_BUFF);
      strcpy(W_BUFF, " ELABORAZIONE...    "); SCRIVI_LCD(0, 3, W_BUFF);
      
      String richiestaAttuale = ""; // Salverà solo la prima riga del comando
      String rigaCorrente = "";     // Per intercettare il doppio a capo \r\n\r\n

      // ALLOCAZIONE PREVENTIVA RAM per evitare la frammentazione...
      richiestaAttuale.reserve(100); // Sappiamo già che non supererà i 100 byte
      rigaCorrente.reserve(80);     // Una riga di header HTTP è raramente più lunga
    
      // --- TIMEOUT DI SICUREZZA: 2 secondi ---
      unsigned long tempoInizio = millis();
      const unsigned long timeout = 2000; 

      // Il ciclo gira finché il client è connesso E non è scaduto il timeout
      while (client.connected() && ((millis() - tempoInizio) < timeout)) {
        if (client.available()) {
          char c = client.read();

          // 1. Salviamo i primi 100 caratteri della richiesta (bastevoli per leggere l'URL)
          if (richiestaAttuale.length() < 100) { 
            richiestaAttuale += c; 
          }

          // 2. Intercettiamo la fine delle intestazioni HTTP (riga vuota \r\n)
          if (c == '\n') {
            // Se rigaCorrente è vuota, significa che abbiamo trovato due \n di seguito (\r\n\r\n)
            if (rigaCorrente.length() == 0) {
              
              // --- Caso 1. Intercettato il comando "bagna" => bagna tutto ---
              if (richiestaAttuale.indexOf("GET /bagna") != -1) {
                ATTIVA_IRRIGAZIONE_FORZATA = true; 
                Serial.println("Attivata Irrigazione Forzata...");
                client.println("HTTP/1.1 303 See Other");
                client.println("Location: /"); // Rimanda dritto alla pagina principale
                client.println("Connection: close");
                client.println(); // Riga vuota obbligatoria per chiudere gli header HTTP
                break; // Esce dal ciclo while, la pagina è stata inviata
              }

              // --- Caso 2. Richiesta della Home Page ---
              else if (richiestaAttuale.indexOf("GET / ") != -1) { 
                Serial.println("Richiesta Home Page...");
                GESTISCI_PAGINA_WEB(client, HOME_PAGE);
                break; // Esce dal ciclo while
              }

              // --- Caso 3. Richiesta di pagine inesistenti o link errati (404) ---
              else {
                Serial.println("Richiesta sconosciuta! Errore 404");
                client.println("HTTP/1.1 404 Not Found"); 
                client.println("Content-Type: text/html; charset=utf-8"); 
                client.println("Connection: close");
                client.println(); 
                client.println("<html><body><div style='text-align:center;'><br><br><h2>ER 404 - Pagina non trovata nel sistema ORTO!</h2></div></body></html>");
                break; // Esce dal ciclo while
              }
            } else {
              // Nuova riga trovata, svuotiamo il buffer della riga per la successiva
              rigaCorrente = "";
            }
          } else if (c != '\r') {
            // Accumuliamo i caratteri della riga corrente (scartando il carattere \r)
            rigaCorrente += c;
          }
        } // Fine client.available()
      } // Fine ciclo while 

      // --- CHIUSURA CONNESSIONE ---
      client.flush(); 
      delay(1); 
      client.stop();
      Serial.println("Connessione chiusa.");
      lcd.clear();
    } // Fine controllo if(client)

    // Se non c'e' stata una richiesta del client da almeno 45 sec si puo considerare il client sconnesso
    if (CLIENT_PRESENTE && ((millis() - ULTIMO_CONTATTO_CLIENT) > 45000UL)) { 
      CLIENT_PRESENTE = false; 
    }
  } // Fine del if (WifiOK)
} // Fine della funzione LOOP_WEB
// -------------------------------------------------------------------------------------------------------------------------------------------------------
// GESTIONE PAGINE WEB - questa funzione gestisce l'invio delle pagine web in base alla pressione dei
// link intercettando  le pagine come comandi. inoltre trasforma i tag contenuti tra i "%" nei numeri o strighe 
// specifici in base al contesto tramite la funzione "processor". 
void GESTISCI_PAGINA_WEB(WiFiClient& client, const char* pagina) {
  // Invio intestazione "pagina esiste"
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println(); // Riga vuota obbligatoria

  // 1. Dichiariamo i due buffer vuoti e definiamo i "recinti" entro i quali devono stare i dati
  String bufferTag = "";
  String bufferInvio = ""; 
  // ricordarsi che nell'allocare la ram devi tener conto di letture al limite: sono arrivato a 199 caratteri (<200 per scrivere) e inizia il
  // tag che ne occupoa 10, finirei a 209 quindi con 9 in overflow !!! stai largo !!!! non scenere sotto il 230 meglio 250 !!!
  bufferInvio.reserve(250); // Alloca subito 250 byte in RAM per la dimensione del buffer dell'HTML
  bufferTag.reserve(30);    // Alloca subito 30 byte per contenere i nomi dei tag (es. %VERSIONE%)
  bool dentroIlTag = false;
  int i = 0;

  // --- 2. IL SETACCIO GENERICO IN TEMPO REALE ---
  // Leggiamo la pagina carattere per carattere finché non finisce ('\0')
  while (pagina[i] != '\0') {
    char carattereAttuale = pagina[i];
    if (carattereAttuale == '%') {          // se c'e' il carattere che => inizio tag
      if (!dentroIlTag) {                   // e non sei gia dentro il tag => % all'inizio
        // Inizio del tag: svuotiamo il buffer di testo normale inviando quello che c'è finora
        if (bufferInvio.length() > 0) {
          client.print(bufferInvio);   // invia quello che hai letto fin'ora..
          bufferInvio = "";            // poi svuotalo
        }
        dentroIlTag = true;
        bufferTag = "";
      }
      else {  // e come capisce che è alla fine del tag?????
        // Fine del tag: interpelliamo il doganiere (processor) e incolliamo il valore vero
        dentroIlTag = false;
        bufferInvio += Converti_TAG(bufferTag);
      }
    } else {
      if (dentroIlTag) {
        bufferTag += carattereAttuale; // Stiamo accumulando il nome del tag (es. VERSIONE)
      }
      else {
        bufferInvio += carattereAttuale; // Testo HTML normale
        // Per non finire la RAM dell'Arduino, ogni 200 caratteri svuotiamo il magazzino sul Wi-Fi
        if (bufferInvio.length() >= 200) {
          client.print(bufferInvio);
          bufferInvio = "";
        }
      }
    }
    i++;
  }

  // Svuotiamo l'ultimo carico rimasto nel buffer prima di chiudere la funzione
  if (bufferInvio.length() > 0) {
    client.print(bufferInvio);  // invia il resto se avanza qualosa...
  }
}
// -------------------------------------------------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------------------------------------------------------------
// --- FUNZIUONE PROCESSOR => CONVERTE I %TAG% IN VALORI O STRINGHE ---
/*    
      REFRESH_TIME
      VERSIONE
      STATO_CISTERNA 
      UMIDITA_Z1
      STATO_Z1
      UMIDITA_Z2
      STATO_Z2
      UPTIME
      MISURAZIONI
      IRRIGAZIONI
      da VAL14_Z1 a VAL0_Z1 (valori array storico Z1)
      da VAL14_Z2 a VAL0_Z2 (valori array storico Z2)
*/


String Converti_TAG(const String& var) {
    // TRADUTTORE DA % TAG % A VALORI DA VISUALIZZARE
  // GRAFICO SVG UNICO CON TUTTE LE ZONE IN VARI COLORI
  if (var == "GRAFICO_SVG_WW") {
    String svg = "";
    svg.reserve(2400); // Buffer leggermente aumentato per contenere entrambe le linee, sperando bastino !!!
    // Contenitore SVG
    // copiare questa parte per le 3 zone modificando i colori per ogni zona (prendili dall'html)
    // limato gli spazi per rendere la String piu piccola possibile!
    svg += "<svg viewBox=\"0 0 540 100\" width=\"100%\" height=\"100\" style=\"background: #fafafa; border-radius: 10px; border: 1.5px dotted #888;\">";
    // Griglia di sfondo (50%)
    svg += "<line x1=\"0\" y1=\"50\" x2=\"540\" y2=\"50\" stroke=\"#ccc\" stroke-dasharray=\"23\" stroke-width=\"1.25\"/>";
    svg += "<text x=\"5\" y=\"47\" fill=\"#888\" font-size=\"9\" font-family=\"sans-serif\">Limite Secco</text>";
    // invece di fare 3 volte la stessa cosa cambiando solo il colore, creo un  Array dei colori abbinato alle 3 zone (indice 1, 2, 3)
    // CONTROLLARE INDICE COLORE CON QUELLI IN HTML PER OGNI RELATIVA ZONA
    const char* colori[] = {"", "#2196f3", "#ff5722", "#ffc107"}; // non riempito indice [0] !
    // Unico ciclo che fa tutte e 3 le zone zenza farlo a mano !
    for (int z = 1; z <= NUMERO_ZONE; z++) {
      svg += "<polyline fill=\"none\" stroke=\"";
      svg += colori[z];
      svg += "\" stroke-width=\"1.25\" points=\"";
      // genera il grafico prendendo tutti i punti nell'arrai dello storico
      for (int i = 59; i >= 0; i--) {
        int x = (59-i)*9;
        int y = 100-ZONA[z].STORICO_UMIDITA[i]; // Diretto e pulito!
        svg += String(x) + "," + String(y) + " ";
      }
      svg += "\" />";
    }
    // terminatori tag html
    svg += "</svg>";
    return svg;
  }
  
  if (var == "VERSIONE_WW")       { return String(VERSIONE); }         // definita
  if (var == "STATO_CISTERNA_WW") { 
    if (ACQUA.STATO_CISTERNA) { return String ("LIVELLO OK"); } 
    else { return String ("RISERVA"); }
  }
  if (var == "LITRI_CISTERNA_WW") { return String (ACQUA.LITRI_CISTERNA); }

  if (var == "UMIDITA_Z1_WW")     { return String(ZONA[1].STORICO_UMIDITA[0]); }  // definita 
  if (var == "STATO_Z1_WW") { 
    if (ZONA[1].STORICO_UMIDITA[0] >= ZONA[1].SOGLIA + 5) { return String ("OK");     }
    if (ZONA[1].STORICO_UMIDITA[0] <  ZONA[1].SOGLIA + 5) { return String ("LIMITE"); }
    if (ZONA[1].STORICO_UMIDITA[0] <  ZONA[1].SOGLIA)     { return String ("SECCO");  }
  }
  if (var == "UMIDITA_Z2_WW")     { return String(ZONA[2].STORICO_UMIDITA[0]); }  // definita 
  if (var == "STATO_Z2_WW") { 
    if (ZONA[2].STORICO_UMIDITA[0] >= ZONA[2].SOGLIA + 5) { return String ("OK");     }
    if (ZONA[2].STORICO_UMIDITA[0] <  ZONA[2].SOGLIA + 5) { return String ("LIMITE"); }
    if (ZONA[2].STORICO_UMIDITA[0] <  ZONA[2].SOGLIA)     { return String ("SECCO");  }
  }
  if (var == "UMIDITA_Z3_WW")     { return String(ZONA[3].STORICO_UMIDITA[0]); }  // definita 
  if (var == "STATO_Z3_WW") { 
    if (ZONA[3].STORICO_UMIDITA[0] >= ZONA[3].SOGLIA + 5) { return String ("OK");     }
    if (ZONA[3].STORICO_UMIDITA[0] <  ZONA[3].SOGLIA + 5) { return String ("LIMITE"); }
    if (ZONA[3].STORICO_UMIDITA[0] <  ZONA[3].SOGLIA)     { return String ("SECCO");  }
  }

if (var == "UPTIME_WW") { 
    // inserisco valori usando meno variabili possibile, conti al volo e via....
    unsigned long t = millis() / 1000UL; // t in secondi
    snprintf_P(W_BUFF, sizeof(W_BUFF), PSTR("%02luGG %02lu:%02lu"), t/86400UL, (t%86400UL)/3600UL, (t%3600UL)/60UL);
    return String(W_BUFF);
  }
  // area misurazioni
  if (var == "N_MIS_Z1_WW")      { return String (ZONA[1].MISURAZIONI); } 
  if (var == "N_MIS_Z2_WW")      { return String (ZONA[2].MISURAZIONI); } 
  if (var == "N_MIS_Z3_WW")      { return String (ZONA[3].MISURAZIONI); }
  // area acqua
  if (var == "LT_ULT_IRR_Z1_WW") { return String (ZONA[1].LITRI_ULTIMA_IRRIGAZIONE); }
  if (var == "LT_ULT_IRR_Z2_WW") { return String (ZONA[2].LITRI_ULTIMA_IRRIGAZIONE); }
  if (var == "LT_ULT_IRR_Z3_WW") { return String (ZONA[3].LITRI_ULTIMA_IRRIGAZIONE); }
  // area irrigazioni
  if (var == "N_IRR_Z1_WW")      { return String (ZONA[1].NUMERO_IRRIGAZIONI); }
  if (var == "N_IRR_Z2_WW")      { return String (ZONA[2].NUMERO_IRRIGAZIONI); }
  if (var == "N_IRR_Z3_WW")      { return String (ZONA[3].NUMERO_IRRIGAZIONI); }
  // area meteo
  if (var == "TEMP_WW")          { return String (AMBIENTE.STORICO_TEMPERATURA[0]); }
  if (var == "UM_WW")            { return String (AMBIENTE.STORICO_UMIDITA[0]); }
  // AL MOMENTO NON USATE
  if (var == "PRESS_WW")         { return String ("Non Impostato"); }
  if (var == "LUX_WW")           { return String ("Non Impostato"); }
  
  // stato modale per la visualizzazione della finestra tipo popup nel caso di irrigazione
  // facendo cosi la pagina web è bloccata in caso di irrigazione!
  if (var == "STATO_MODALE_WW") {
    if (ATTIVA_IRRIGAZIONE_FORZATA || ATTIVA_IRRIGAZIONE) { return String("flex"); } // flex attiva la "popup" e blocca la pagina
    else { return String("none"); } // none non attiva la "popup" e la pagina funziona
  }

  // Se non trova corrispondenze, cancella il tag tornando nulla ("")
  return "";
}
// -------------------------------------------------------------------------------------------------------------------------------------------------------
