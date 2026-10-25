// FILE HEADER PER LE DEFINIZIONI DEI PIN, TIMERS E VALORI UMIDITA, TUTTO IN UN UNICO FILE

// ---------------------------------------------------------------------------------------------------------------------------------
// HEADER DEFINIZIONE LINEARE PIN in file separato per limitare la lunghezza del file principale
// sintassi : define Nome_Pin Numero_Pin

 // #define PIN0              0        // non usare, collegamento usb
 // #define PIN1              1        // non usare, collegamento usb
    #define FLUSSIMETRO_PIN   2       // pin digit al quale viene collegato il flussimetro gestito da interrupt
    #define PULSANTE          3       // bottone manuale
    
    const uint8_t pinData  = 4;         // PIN 4 => Data Serial per comandare il 74HC595 
    const uint8_t pinLatch = 5;         // ST_CP => Shift Register Clock Pin   (Chip 74HC595)
    const uint8_t pinClock = 6;         // SH_CP => Storage Register Clock Pin (Chip 74HC595)

     



    #define DHT_PIN           7       // pin per il collegamento del sensoe dht22

    #define LIVELLO_H2O       8       // interruttore riserva acqua, chiuso con acqua, aperto senza
//  TRAMITE 74HC595#define VALVOLA_Z1        9       // rele valvola zona 1
//  TRAMITE 74HC595    #define VALVOLA_Z2        10      // rele valvola zona 2
//  TRAMITE 74HC595    #define RELE_COMUNE       11      // rele comune valvole
//  TRAMITE 74HC595    #define RELE_POMPA        12      // attivazione pompa acqua - da usare alimentazione come attivazione rele grande
//  #define pin13             13      
//  #define S_ZONA1           14      // -- A0 -- sensore zona 1 => messi nella struci per facilità di gestione
//  #define S_ZONA2           15      // -- A1 -- sensore zona 2 => messi nella struci per facilità di gestione
//  #define S_ZONA3           16      // -- A2 -- sensore zona 3 => messi nella struci per facilità di gestione
//  #define                   17      // -- A3 -- partitiore per il controllo illuminazione
//  #define PIN18             18      // -- A4 -- ----> possibili pin per Software Serial <----  (A4 => SDA schermo !!)
//  #define PIN19             19      // -- A5 -- ----> possibili pin per Software Serial <----  (A5 => SCL schermo !!)
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------
// HEADER PER LA GESTIONE DI TUTTE LE VARIABILI TEMPORALI
// sia per l'uso del pulsante multifinzione che per i timer di irrigazione ed altro
// GESTIONE DURATA PRESSIONE PULSANTE MULTIFUNZIONE
#define TEMPO_OP1 3000UL   // definizione tempo accensione retroilluminazione = dopo 3 sec
#define TEMPO_OP2 5000UL   // definizione tempo pressione per irrigazione forzata = dopo 5 sec
#define TEMPO_OP3 12000UL  // definizione tempo pressione per reboot forzato      = dopo 11 sec
#define TEMPO_MAX 16000UL  // definizione tempo pressione per timeout output      = dopo 20 sec
// DEFINIZIONE TEMPI OPERATIVI

#define SONO_VIVO 250UL                                      // tempo di lampeggio led per avvertire che "sono vivo"
#define INTERVALLO_LETTURA_ACQUA 180000UL                     // lettura acqua ogni 3 min
#define APERTURA_VALVOLA_ZONA (25UL * 1000UL)                // tempo di apertura e chiusura valvola di zona nuovo tipo (25 sec)
#define INTERVALLO_IRRIGAZIONI (2UL * 60UL * 60UL * 1000UL)  // 2 ore tra le possibili irrigazioni
#define INTERVALLO_LETTURE (30UL * 60UL * 1000UL)            // 30 minuti tra le letture - lasciare cosi, non cambiare valore !!!
#define REFRESH_LCD_MODO_ORDINARIO (60UL * 1000UL)          // Aggiorna lo schermo ogni 60 sec
#define AUTO_RESET (10UL * 24UL * 60UL * 60UL * 1000UL)      // autoriavvio sistema ogni 10 giorni
// ---------------------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------------------
// HEADER DEFINIZIONE DEI VALORI LIMITE PER LA LETTURA DEI DATI UMIDITA' TERRENO
// SOGLIA LIMITE VALORE UMIDITA'
// la SOGLIA indica il valore al di sotto del quale parte l'irrigazione automatica
// ricordarsi che i valori sono molto influenzati dai sensori e dalla lunghezza dei cavi,
// fare dei test per il settaggio dei valori grezzi *_RAW il piu oreciso possibile
// eventualmente usare il fattore correzione (accetta solo valori positivi ! )
#define NUMERO_ZONE          3           // numero di zone da controllare
#define SOGLIA_RH_Z1         50          // % limire umiditra al di sotto del quale bagna Z1
#define SOGLIA_RH_Z2         50          // % limire umiditra al di sotto del quale bagna Z2
#define SOGLIA_RH_Z3         50          // % limire umiditra al di sotto del quale bagna Z3
#define SECCO_RAW_VALUE      900         // valore lettura analogica corrispondente allo 0 % - terreno secco del tutto
#define BAGNATO_RAW_VALUE    200         // valore lettura analogica corrispondente al 100 % - terreno bagnato in ammollo
// ---------------------------------------------------------------------------------------------------------------------------------