// AREA-FILE CONTENENTE LE FUNZIONI RELATIVE ALLA IRRIGAZIONE VERA E PROPRIA

// GESTISCI_SENSORI_TERRENO: funzione che controlla e legge i sensori del terreno facendo una media tra 100 letture
// ed incapsulando tutto dentro un constrain e map per trasformare il dato raw in dato "%"
// prima di effettuare la misurazione il sistema fa una pausa di 100 millis per stabilizzare la linea sensori
// ipotizzata in 50 metri dicavo e qualche condensatore a fine linea, sorse anchein mezzo. le letture vengono fatte in
// rapida sucessione perche il rumore elettrico casuale ("white noise" captato dai 50 metri di cavo come se fossero un'antenna)
// è oscillante. Prelevare campioni vicinissimi nel tempo e sommarli fa sì che i picchi casuali verso l'alto e verso il basso
// si bilancino a vicenda, lasciando emergere il valore medio il più reale e pulito possibile., 
void GESTISCI_SENSORI_TERRENO() {
  // imposta le variabili temporanee interne per le letture
  static uint8_t stato_lettura_HR = 1;
  static unsigned long timer_stabilizzazione_HR = 0;
  static uint8_t letture_ciclo = 100;
  switch (stato_lettura_HR) {
      case 1:
        // attacca la spina e inserisco effetto visivo = luce blu accesa
        bitWrite(ComandoShiftReg,LedBlu,HIGH);
        bitWrite(ComandoShiftReg,AlimentazioneTerreno,HIGH);
        AggiornaShiftRegister (ComandoShiftReg);
        // resetta il valore grezzo per ogni zona e trasla lo storico per "liberare"la posizione 0
        for (int i=1; i<(NUMERO_ZONE+1); i++) {
          ZONA[i].RAW = 0;
          for (uint8_t iz = 59; iz > 0; iz--) { ZONA[i].STORICO_UMIDITA[iz] = ZONA[i].STORICO_UMIDITA[iz-1]; }
        }
        timer_stabilizzazione_HR = millis();  // setta a questo punto lo zero del timer
        stato_lettura_HR = 2;  // vai allo stato numero 2...
        break;
      case 2:
        // dopo aver atteso il tempo di stabilizzazione sensori (alzato da 100 a 200 millis)...
        if (millis() - timer_stabilizzazione_HR >= 200) {      // dopo 200 millis...
          for (uint8_t i=1; i<(NUMERO_ZONE+1); i++) {          // per tutte le zone presenti impostate
            analogRead (ZONA[i].PIN_SENSORE);                  // 1) fai a vuoto per commutare l'ADC e caricare il condensatore della linea
            for (uint8_t ic = 0; ic < letture_ciclo; ic++) {   // 2) per 100 volte (in modo da ridurre il white noise)...
              ZONA[i].RAW += analogRead(ZONA[i].PIN_SENSORE);  // ...faccio la lettura reale e stabile del sensore e lo salvo nel "grezzo"
            }
          }
        stato_lettura_HR = 3;     // vai allo stato numero 3...
        }
        break;
      case 3:
        for (uint8_t i=1; i<(NUMERO_ZONE+1); i++) {   // per tutte le zone impostate...
          // ... trasforma il valore raw letto nel "case2" in un numero percentuale arrotondato ...
          ZONA[i].STORICO_UMIDITA[0] = constrain(map(((ZONA[i].RAW+letture_ciclo/2)/letture_ciclo),SECCO_RAW_VALUE,BAGNATO_RAW_VALUE,0,99),0,99);
          ZONA[i].MISURAZIONI++;  // ...ed aumento contatore letture in struct
        }
        stato_lettura_HR = 4;     // vai allo stato 4 ...
        break;
      case 4:
        // spengo tutto...
        bitWrite(ComandoShiftReg,LedBlu,LOW);
        bitWrite(ComandoShiftReg,AlimentazioneTerreno,LOW);
        AggiornaShiftRegister (ComandoShiftReg);
        LEGGI_SENSORI = false;     // reset flag attivazione lettura sensori in modo che non rientri nella funzione
        stato_lettura_HR = 1;      // reset a 1 per il controllo successivo altrimenti parte dal 4 (static !!!!!)
        break;
        // fine switch
    }
    // fine lettura sensori
  }

// INTERRUPT: funzione veloce per il conteggio degli impulsi del sensore di consumo acqua
// l'interrupt viene messo solo dentro la funzione "GESTISCI IRRIGAZIONE" (quella sotto)
// nel punto in cui avvio la pompa. Qui blocco un istante l'interrupt per esportare il valore
// letto su una variabile esterna per il controllo/elaborazione finale
void contaImpulsi() {
  impulsi_conta_acqua++;
}


// FUNZIONE "GESTISCI IRRIGAZIONE": la parte operativa vera e propria. 
// Logica operativa elencata sotto caso per caso, partendo con la zona di partenza =1
// caso 0: attiva i flag solo se c'e' acqua ed è stata richiesta irrigazione normale (verifica temporale) o forzata
// caso 1: verifica se la zona deve essere bagnata (normale o forzata). se si inizia le attivazioni altrimenti passa alla prossima
// caso 2: attendo apertura valvola zona,poi accendo pompa e spengo rele apertura valvola zona
// caso 3: attendo il tempo di irrigazione, finito il quale spengo la pompa ed inizio la chiusura delle valvole
// caso 4: attendo chiusura valvola zona poi imposto zona sucessiva, se la zona è l'ultima imposto per chiudere tutto
// caso 5: spegnimento totale e ripristino flag per la prossima volta
void GESTISCI_IRRIGAZIONE() {
  // Variabili di stato che vivono dentro la funzione
  static uint8_t stato_generale = 0; // 0 = Fermo / In attesa di comando
  static uint8_t zona_corrente = 1;  // Tiene traccia di quale zona stiamo bagnando
  static unsigned long timer_funzione = 0;
  static unsigned long timer_lampeggio_int = 0;
  switch (stato_generale) {
    // mettere le {  } per recintare i case
    case 0: 
      {
      // attivo la lettura della cisterna per vedere se c'e' acqua, la lettura la fa nel tempo morto di apertura valvole !!
      LETTURA_ACQUA = true; // cosi si aggiorna il bool della cisterna, eventualmente parte ICE !!!
      // Stato di riposo: se c'è acqua e arriva un comando di avvio (normale o forzato)
      if (ACQUA.STATO_CISTERNA == true && (ATTIVA_IRRIGAZIONE || ATTIVA_IRRIGAZIONE_FORZATA)) {
        // faccio partire il controllo o l'avvio forzato (cambio stato)
        zona_corrente = 1; // Partiamo dalla prima zona
        stato_generale = 1; // Inizia la sequenza
      }
      break;
      }
    case 1:
      { 
      // Controllo se la zona corrente deve essere bagnata , in entrambi i casi
      bool bagnare_questa = ATTIVA_IRRIGAZIONE_FORZATA || (ZONA[zona_corrente].STORICO_UMIDITA[0] <= ZONA[zona_corrente].SOGLIA);
      if (bagnare_questa) { // se devi bagnare (secco o forzato...)
        // Accendi scheda, comune e valvola della zona_corrente
        bitWrite(ComandoShiftReg, AlimentazioneRele, HIGH);
        AggiornaShiftRegister(ComandoShiftReg);
        delay(30); // Stabilizzazione consentita per pochi ms
        bitWrite(ComandoShiftReg, ReleComune, HIGH); 
        bitWrite(ComandoShiftReg, ZONA[zona_corrente].BIT_VALVOLA_ZONA, HIGH); 
        bitWrite(ComandoShiftReg, LedExtra1V, HIGH);
        AggiornaShiftRegister(ComandoShiftReg);
        // CONTA ACQUA USATA
        noInterrupts();           // blocco gli interrupts per..
        impulsi_conta_acqua = 0; // azzerare il contatore globale dell'interrupt prima di irrigare
        interrupts();             // riattivo gli interrupts
        // CONTA ACQUA USATA
        timer_funzione = millis();
        stato_generale = 2; // Passa ad attendere l'apertura valvola
      } else {
        // Se questa zona non serve bagnarla, passa subito alla successiva
        zona_corrente++;
        if (zona_corrente > NUMERO_ZONE) { stato_generale = 7; } // Fine di tutte le zone
      }
      timer_lampeggio_int = millis();
      break;
      }
    case 2:
      { 
      // nel frattempo che si apre la valvola lampeggia blu extra ogni 250
      if (millis()-timer_lampeggio_int>=250) {
        bitWrite(ComandoShiftReg, LedExtra3B, !bitRead(ComandoShiftReg, LedExtra3B));
        AggiornaShiftRegister (ComandoShiftReg);
        timer_lampeggio_int = millis();
      }
      // Attendere il tempo di apertura valvola
      if (millis() - timer_funzione > APERTURA_VALVOLA_ZONA) {  // se il tempo è superato...
        // Attiva la pompa acqua e la luce blu extra fissa, poi spegni valvole 
        // (spengo i rele delle valvole, lasciarli accesi per niente per 10 minuti.....non mi va)
        bitWrite(ComandoShiftReg, PompaOrto, HIGH);
        bitWrite(ComandoShiftReg, ZONA[zona_corrente].BIT_VALVOLA_ZONA, LOW);
        bitWrite(ComandoShiftReg, ReleComune, LOW);
        bitWrite(ComandoShiftReg, LedExtra3B, HIGH); // luce blu extra accesa fissa
        AggiornaShiftRegister(ComandoShiftReg);
        timer_funzione = millis();
        stato_generale = 3; // Passa ad attendere il tempo di irrigazione
      }
      break;
      }
    case 3:
      {
      // Attendere il tempo di irrigazione della zona specifica (nella struct) 
      if (millis() - timer_funzione > ZONA[zona_corrente].TEMPO_IRRIGAZIONE) {
        // se il tempo è superato, spegni pompa e chiudi valvola
        bitWrite(ComandoShiftReg, PompaOrto, LOW);
        bitWrite(ComandoShiftReg, ReleComune, HIGH); // attiva per chiudere valvola
        AggiornaShiftRegister(ComandoShiftReg);
        timer_funzione = millis();
        timer_lampeggio_int = millis();
        stato_generale = 4; // Attenditela chiusura completa
      }
      break;
      }
    case 4:
      {
      // nel frattempo che si chiude la valvola lampeggia blu extra ogni 250
      if (millis()-timer_lampeggio_int>=250) {
        bitWrite(ComandoShiftReg, LedExtra3B, !bitRead(ComandoShiftReg, LedExtra3B));
        AggiornaShiftRegister (ComandoShiftReg);
        timer_lampeggio_int = millis();
      }    
      // Attendere la chiusura della valvola (lasciando il comune attivo)
      if (millis() - timer_funzione > APERTURA_VALVOLA_ZONA) { // valvola chiusa, spegni tutto
        bitWrite(ComandoShiftReg, ReleComune, LOW); 
        AggiornaShiftRegister(ComandoShiftReg);
        // CONTA ACQUA USATA
        // qui metto il conta acuqa dopo la chiusura totale della valvola (discorso della pressione statica serbatoio...)
        noInterrupts();
        unsigned long impulsi_registrati = impulsi_conta_acqua;
        interrupts();
        // Calcoliamo i litri ( divido per il fattore di conversione)
        float litri_irrigati = (float)impulsi_registrati / (float)ACQUA.FATTORE_CONVERSIONE_CONTAIMPULSI;
        ZONA[zona_corrente].LITRI_ULTIMA_IRRIGAZIONE  = round(litri_irrigati);
        ZONA[zona_corrente].LITRI_EROGATI_TOTALI     += round(litri_irrigati);
        ZONA[zona_corrente].NUMERO_IRRIGAZIONI++;
        ACQUA.LITRI_CISTERNA = ACQUA.LITRI_CISTERNA-ZONA[zona_corrente].LITRI_ULTIMA_IRRIGAZIONE;
        if (ACQUA.LITRI_CISTERNA < 1) { ACQUA.LITRI_CISTERNA = 0; }  // per evitare che il valore diventi negativo...
        // FINE CONTA ACQUA....
        // riprendo con la sequenza
        zona_corrente++; // dopo aver finito una zona possa alla sucessiva
        if (zona_corrente > NUMERO_ZONE) {
          stato_generale = 5;  // Se abbiamo fatto tutte le zone, spegniamo tutto
        } else {
          stato_generale = 1;  // Altrimenti torniamo al check della zona successiva
        }
      }      
      break;
      }
    case 5:
      { // Spegnimento finale generale della scheda, reset "flag" e ripristino "static"
      bitWrite(ComandoShiftReg, PompaOrto, LOW);
      bitWrite(ComandoShiftReg, ReleComune, LOW);
      bitWrite(ComandoShiftReg, LedExtra1V, LOW);
      bitWrite(ComandoShiftReg, LedExtra3B, LOW);
      AggiornaShiftRegister(ComandoShiftReg);
      delay(50); // attesa prima di spegnere scheda
      bitWrite(ComandoShiftReg, AlimentazioneRele, LOW);
      AggiornaShiftRegister(ComandoShiftReg);
      // reset variabili interne
      ATTIVA_IRRIGAZIONE = false;
      ATTIVA_IRRIGAZIONE_FORZATA = false;
      stato_generale = 0;
      zona_corrente = 1;
      timer_funzione = 0;
      timer_lampeggio_int = 0;
      break;
      }
  }
}


// ------------------------------------------------------------------------------------







// LETTURA LIVELLO ACQUA
// funzione che effettua la lettura del sensore livello cisterna scrivendo il risultato sulla struct relativa e nello storico
// la funzione impiega circa 300 millisec, come avvertimento visivo metto il led blu sempre acceso
void CONTROLLO_LIVELLO_CISTERNA() {
    static uint8_t stato_H2O = 1;
    static uint8_t lettureH2O = 0;    // somma delle letture digitali del sensore acqua 
    static uint8_t numero_LA = 0;     // numero delle letture da fare e da sommare
    static unsigned long timer_LA1 = 0; // timer generico (LA = lettura acqua)
    switch (stato_H2O) {
      case 1:
        bitWrite(ComandoShiftReg,AlimentazioneAcqua,HIGH);
        bitWrite(ComandoShiftReg,LedExtra3B,HIGH);
        AggiornaShiftRegister (ComandoShiftReg);
        stato_H2O = 2;
        timer_LA1 = millis();
        break;
      case 2:
        // stabilizza sensore
        if (millis() - timer_LA1 >= 20) { stato_H2O = 3; }
        break;
      case 3:
        if (numero_LA<=9) {
          lettureH2O += digitalRead(LIVELLO_H2O);
          numero_LA++;
          // il numero delle letture totali = 10 x numero cicli lettura !!!!!
          stato_H2O=2;
        }
        else {
          if (lettureH2O <3 ) { ACQUA.STATO_CISTERNA = true;  }  // tollero 2 letture fallate max
          if (lettureH2O >=3) { ACQUA.STATO_CISTERNA = false; }  
          // quando sarà implementato il misuratore acqua la riga sopra portebbe essere trasformata cosi:
          // if (lettureH2O >=3 || ACQUA.LITRI_CISTERNA<50) { ACQUA.STATO_CISTERNA = false; }  
          // aggiorno la struci spostando i dati di 1 indietro....
          for (int i = 0; i < 59; i++) { ACQUA.LETTURA_GREZZA[i] = ACQUA.LETTURA_GREZZA[i + 1]; }
          ACQUA.LETTURA_GREZZA[59] = lettureH2O; // storico per controllare lo stato dei sensori - se vuoi farlo
          ACQUA.CICLI_LETTURA++;
          // reset variabili per prossima lettura
          stato_H2O = 1; lettureH2O = 0;
          numero_LA = 0; timer_LA1 = 0;
          bitWrite(ComandoShiftReg, AlimentazioneAcqua, LOW);
          bitWrite(ComandoShiftReg, LedExtra3B, LOW);
          AggiornaShiftRegister(ComandoShiftReg);
          LETTURA_ACQUA = false; // Disattiva il flag principale
        }
        break;
    }
  } // fine lettura acqua


// FUNZIONE I.C.E. NEL CASO NON CI SIA ACQUA
// accende spia lampeggiante, mette il warning sul lcd, attende il riempimento e la pressione del pulsante
// non come reset totale del sistema ma come un "premi per continuare" (con cisterna piena!!!)
  void ICE_ACQUA() {
    static uint8_t stato_ICE = 1;
    static unsigned long timer_VA1 = 0;  // timer per chiusura valvole (VA = verifica acqua)
    static unsigned long timer_VA2 = 0;  // timer per lampeggio rosso  (VA = verifica acqua)
    static bool scritto_ICE = false;
    // macchina a stati
    switch (stato_ICE) {
      case 1:
        // spegni tutte le valvole,pompe ecc che eventualmente sono attivate
        ComandoShiftReg = 0x00;
        // attiva solo le uscite che servono per chiudere le valvole (aimentazione scheda + comune e led)
        bitWrite(ComandoShiftReg,AlimentazioneRele,HIGH); 
        bitWrite(ComandoShiftReg,ReleComune,HIGH);
        bitWrite(ComandoShiftReg,LedRosso,HIGH);
        bitWrite(ComandoShiftReg,LedExtra2R,HIGH);  // metto led extra, uno lampeggia ed uno rimane acceso
        AggiornaShiftRegister (ComandoShiftReg);
        // disabilita i flag delle irrigazioni sia forzata che ordinaria
        ATTIVA_IRRIGAZIONE = false;
        ATTIVA_IRRIGAZIONE_FORZATA = false;
        // messaggio a schermo
        lcd.clear();
        // accendi lcd ! warning visivo doppio (led + lcd)
        lcd.backlight(); timer_retroilluminazione = millis();
        strcpy_P(W_BUFF, PSTR("LIVELLO ACQUA BASSO")); SCRIVI_LCD(0, 0, W_BUFF);
        strcpy_P(W_BUFF, PSTR("-I.C.E.- ACQUA")); SCRIVI_LCD(0, 1, W_BUFF);
        strcpy_P(W_BUFF, PSTR("SPENGO POMPA, CHIUDO")); SCRIVI_LCD(0, 2, W_BUFF);
        strcpy_P(W_BUFF, PSTR("VALVOLE DI ZONA")); SCRIVI_LCD(0, 3, W_BUFF);
        timer_VA1 = millis();
        timer_VA2 = millis();
        stato_ICE=2;
        break;
      case 2:
        if (millis() - timer_VA1 >= APERTURA_VALVOLA_ZONA) { stato_ICE = 3; }
        break;
      case 3: // caso bloccante fino all'arrivo dell'operatore
        if (!scritto_ICE) {
          // riaccendi la luce....
          lcd.backlight(); timer_retroilluminazione = millis();
          strcpy_P(W_BUFF, PSTR("OPERAZIONI DA FARE"));   SCRIVI_LCD(0, 1, W_BUFF);
          strcpy_P(W_BUFF, PSTR("1)RIFORNIRE CISTERNA")); SCRIVI_LCD(0, 2, W_BUFF);
          strcpy_P(W_BUFF, PSTR("2)PREMI PER RIAVVIO ")); SCRIVI_LCD(0, 3, W_BUFF);
          scritto_ICE = true;
        }
        // prova a non fermar tutto, lampeggia il led rosso
        // ricordarsi di inserire il controllo .. && ACQUA_OK ) dentro gli if della irrigazione !!!!
        if (digitalRead(PULSANTE) == HIGH) { delay(20);
          if (digitalRead(PULSANTE) == HIGH) {
            stato_ICE = 4;
          }
        }
        break;
      case 4:
        // SEI QUI DOPO AVER PREMUTO IL PULSANTE: 
        // effettua il reset del bool e reimposta i litri in cisterna
        LETTURA_ACQUA = true;  // abilito la lettura appena esce dal ciclo
        ACQUA.LITRI_CISTERNA = 1000; // reimposto i litri nella cisterna per il calcolo della quantita
        // reimposto gli static
        stato_ICE = 1; timer_VA1 = 0; timer_VA2 = 0; scritto_ICE = false;
        lcd.clear();
        // spengo le luci rosse....
        bitWrite(ComandoShiftReg,LedRosso,LOW);
        bitWrite(ComandoShiftReg,LedExtra2R,LOW);
        AggiornaShiftRegister (ComandoShiftReg);
        break;
    } // fine switch
    // LAMPEGGIO LED ROSSO => EMERGENZA
    if (millis() - timer_VA2 > 300UL) { 
    bitWrite(ComandoShiftReg, LedRosso, !bitRead(ComandoShiftReg, LedRosso));
    AggiornaShiftRegister (ComandoShiftReg);
    timer_VA2 = millis();
    }
  }


// FUNZIONE PER LA GESTIONE DATI METEO: alimenta poi legge i dati, vede se non
// sono ISNAN poi li salva nella struttura dati
void GESTISCI_METEO() {
  // definizione variabili per la gestione della funzione
  static uint8_t        punto = 1;          // per lo stato della macchina "leggi meteo"
  static unsigned long  timer1;             // timer interno gestione tempo stabilizzazione
  static unsigned long  timer2;             // timer interno gestione tempo lampeggio
  static float          umidita = 0;        // variabile con la virgola, poi verrà arrotondato
  static float          temperatura = 0;    // variabile con la virgola, poi verrà arrotondato
  static uint16_t       letture_meteo = 0;  // contatore per il controllo errori NAN  se letture > 10 senza andare a buon fine = errore
  // macchina a stati
  switch (punto) {
    case 1:
      // accenso la luce
      bitWrite(ComandoShiftReg, LedExtra3B, HIGH);           // accendo luce
      bitWrite(ComandoShiftReg, AlimentazioneDHT22, HIGH);   // alimento sensore
      AggiornaShiftRegister(ComandoShiftReg);
      timer1 = millis(); timer2 = millis();
      punto = 2;
      break;
    case 2:
      // Attesa di 2000ms per la stabilizzazione del sensore dopo l'accensione
      if (millis()-timer1 > 2000) { 
        umidita     = dht.readHumidity();         // Lettura dell'umidità
        temperatura = dht.readTemperature();      // Lettura della temperatura
        if (isnan(umidita) || isnan(temperatura)) {  // verifica ISNAN sui risultati
          letture_meteo++;
          bitWrite(ComandoShiftReg, LedExtra3B, LOW);           // spengo luce
          bitWrite(ComandoShiftReg, AlimentazioneDHT22, LOW);   // spengo sensore
          AggiornaShiftRegister(ComandoShiftReg);
          // se fa troppi errori (5) NAN esce e ferma il controllo senza la lettura
          if (letture_meteo >= 5) {
            AMBIENTE.ERRORI_NAN++;
            Serial.println("Errore sensore DHT22!");
            punto = 4; // Salta alla chiusura/reset
          }
          else {
            punto = 1; // Riprova ricominciando il ciclo di accensione
          }
        }
        else {
          // Lettura valida, passiamo al salvataggio nello storico
          punto = 3;
        }
      }
      break;
    case 3:
      // sposta lo storico e salva i nuovi dati
      for (uint8_t ii = 59; ii > 0; ii--) {
        AMBIENTE.STORICO_TEMPERATURA[ii]    = AMBIENTE.STORICO_TEMPERATURA[ii-1];
        AMBIENTE.STORICO_UMIDITA[ii]        = AMBIENTE.STORICO_UMIDITA[ii-1];
        // AMBIENTE.STORICO_ILLUMINAZIONE[ii]  = AMBIENTE.STORICO_ILLUMINAZIONE[ii-1];  // non ancora impostato
        // AMBIENTE.STORICO_PRESSIONE[ii]      = AMBIENTE.STORICO_PRESSIONE[ii-1];      // non ancora impostato
      }
      // salva i nuovi dati
      AMBIENTE.STORICO_TEMPERATURA[0] = round(temperatura);  // round arrotonda il valore 
      AMBIENTE.STORICO_UMIDITA[0]     = round(umidita);      // round arrotonda il valore 
      punto = 4; // Fatto, andiamo allo spegnimento
      break;
    case 4:
      // reset totale e spegnimento delle periferiche
      bitWrite(ComandoShiftReg, LedExtra3B, LOW);           // spengo luce
      bitWrite(ComandoShiftReg, AlimentazioneDHT22, LOW);   // spengo sensore
      AggiornaShiftRegister(ComandoShiftReg);
      punto = 1;
      timer1 = 0;
      letture_meteo = 0;
      LEGGI_METEO = false;      // resetta il flag, al prossimo giro non entra nel ciclo...
      break;
  }
  // lampeggio led ....
  if (millis()-timer2>=200) {
    bitWrite(ComandoShiftReg, LedExtra3B, !bitRead(ComandoShiftReg,LedExtra3B));
    AggiornaShiftRegister(ComandoShiftReg);
    timer2=millis();
  }
}

