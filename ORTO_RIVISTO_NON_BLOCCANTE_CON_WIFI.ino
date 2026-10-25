// --------------------------------------------
// ------ FIRMWARE PER ARDUINO R4 WIFI --------
// --------------------------------------------
// FILE PRONCIPALE DEL SISTEMA DI IRRIGAZIONE ORTO, LE FUNZIONI SONO CONTENUTE NEL FILE FINZIONI.ino GESTITE TUTTE CON VARIABILI GLOBALI
// IL COMPILATORE LE INCLUDE AUTOMATICAMENTE ALLA COMPILAZIONIE DEGLI .ino ESTERNI, IN ORDINE ALFABETICO

// INCLUSIONI VARIE..
#include <LiquidCrystal_I2C.h>  // includo libreria display i2c gestione LCD
#include <WiFiS3.h>             // includo libreria web per R4
#include "SETTAGGI.h"           // header per la definizione dei pin, timers e valori umidita
#include "VERSIONI.h"           // header versioni software
// DEFINIZIONI, VARIABILI E PAGINE WEB PER LA GESTIONE "NET" DEL SISTEMA
#include "HOME_PAGE.h"
#include "SECRET.h"
WiFiServer Server_Orto(80); 
#include <DHT.h>              // libreria per sensore meteo "DHT sensor library Adafruit"

// creazione dell'oggetto "sensore meteo"
#define DHTTYPE DHT22          // Specifichiamo che usiamo il DHT22 (AM2302)
DHT dht(DHT_PIN, DHTTYPE);

// BUFFER_SCRITTURA MULTIUSO: usato, scritto e sovrascritto per gestire i msg lcd con max 20 caratteri 
char W_BUFF[21];  // 20 caratteri su una linea + terminatore! gestione overflow ok

// CREAZIONE LCD con i parametri presi ca i2c monitor
LiquidCrystal_I2C lcd(0x27, 20, 4);  // settaggio LCD indirizzo 0x27 display 20 caratteri 4 linee

// STRUTTURA DATI PER STAZIONE METEO "DATI AMBIENTALI" - TEMPERATURA AMBIENTE > 0 !!!
struct METEO {
  // Valori istantanei e storico
  uint8_t   ERRORI_NAN;       // aumenta di uno dopo 5 letture errate (NAN)
  uint8_t   STORICO_TEMPERATURA[60];
  uint8_t   STORICO_UMIDITA[60];
  // uint16_t  STORICO_ILLUMINAZIONE[60];    // con fotoresistenza ingresso A3 <=> 17
  // float     STORICO_PRESSIONE[60];        // con BME280 ma costa 20 euro !!!!
};
// CREAZIONE ISTANZA-OGGETTO "AMBIENTE" E RESET VALORI INTERNI
METEO AMBIENTE = {0};     // richiamo le info con AMBIENTE.TEMPERATURA oppure AMBIENTE.UMIDITA....

struct CISTERNA  {
  // valori salvati per controllo sistema cisterna
  // il numero totale delle letture è 10x numero cicli di lettura (faccio 10 letture per ciclo !!!! )
  // cosi non usi una variabile quasi inutile .....
  uint8_t       LETTURA_GREZZA[60];
  uint16_t      CICLI_LETTURA;        // moltiplicalo per 10 ed hai le letture totali fatte
  uint8_t       FATTORE_CONVERSIONE_CONTAIMPULSI;  // fattore conversione impulsi <=> litri del sensore acqua 
  uint16_t      LITRI_CISTERNA;  // litri dentro la cisterna
  bool          STATO_CISTERNA;       // ok o non ok
};
// OGGETTO "ACQUA" con tutto a zero tranne fattore conversione (100) e litri (1000)
CISTERNA ACQUA = { {0}, 0, 100, 1000, true }; // <-- Nota: {0} singolo per l'array
// INTERRUPT: variabile volatile per il contaimpulsi del flussimetro FUORI DA TUTTO !
volatile unsigned long impulsi_conta_acqua = 0;


// STRUTTURA DATI PER GESTIONE DELLE ZONE
struct DATI{
	uint16_t      MISURAZIONI;
	uint16_t      NUMERO_IRRIGAZIONI;
	long          RAW;                           // contatore valori RAW in caso di lettura analogica forse posso metterlo fuori....
  uint8_t       SOGLIA;                  
	uint8_t       STORICO_UMIDITA[60];           // il [0] è la attuale. NB lettura ogni 30 minuti => 30 ore di storico, vedere se ingrandire ! 
  uint8_t       LITRI_ULTIMA_IRRIGAZIONE;      // litri erogati nella ultima irrigazuione (max 100)
  uint16_t      LITRI_EROGATI_TOTALI;          // litri totali in tutte le irrigazioni fatte (anche >1000)
  uint8_t       BIT_VALVOLA_ZONA;              // numero bit che attiva la valvola della zona prescelta
  uint8_t       PIN_SENSORE;                   // pin a cui è collegato il sensore analogico relativo alla zona 
  unsigned long TEMPO_IRRIGAZIONE;             // settato ad inizio setup
  bool          DA_BAGNARE;                    // bool modificata nel controllo irrigazione
};
// CREAZIONE ISTANZA/OGGETTO "ZONA" CON METODO AoS (Array of Structures). NON USARE L'INDICE "0" !!!
// USANDI L'ARRAY SI POSSONO GESTIRE ANCEH CON UN CICLO FOR. L'INDICE DEFINISCE LA ZONA ! 
// INSERENDO NUMERO_ZONE+1 LASCIO LIBERA LA ZONA[0] CHE MENTALMENTE E' PIU' COMPLICATA DA GESTIRE
// RICHIAMARE COSI : "ZONA[1].UMIDITA;" OPPURE "ZONA[2].STORICO_UMIDITA[30];" 
DATI ZONA[(NUMERO_ZONE+1)] ={0};  //numero zone messo come definizione nei settaggi

//---------------------------------------------------- VEDERE SE SERVONO TUTTE OPPURE NO !---------------------------------------------------------
// definizione flag per macchine a stati
bool LETTURA_ACQUA              = false;     // flag lettura acqua, true => leggi
bool LEGGI_SENSORI              = false;     // flag lettura sensori, true => leggi
bool LEGGI_METEO                = false;     // flag  lettura dati meteo, true => leggi
bool ATTIVA_IRRIGAZIONE_FORZATA = false;     // true per far partire l'irrigazione con la pressione pulsante
// bool REBOOT                     = false;     // per abilitazione reboot manuale reboot messo nel gestione pulsante
bool ATTIVA_IRRIGAZIONE         = false;     // Flag per irrigazione automatica schedulata
bool WIFINET_AP                 = false;     // Flag per definire il tipo di connessione (AP oppure STATION, se non è ap)
bool WifiOK                     = false;     // Flag che indica se c'e' o non c'e' la scheda wifi
bool CLIENT_PRESENTE            = false;     // Flag che identifica o no la presenza client dopo la richiesta pagina (nel LOOP_WEB)

// definizioni dei timers (spostato da file esterno per non rischiare conflitti in compilazione)
unsigned long timer_retroilluminazione;  // cronometro per attivare/disattivare luce lcd
unsigned long timer_display;              // cronometro per aggiornamento display
unsigned long timer_letture;              // cronometro da usare con millis() per il controllo sensori + irrigazione
unsigned long timer_irrigazione;          // cronometro da usare per controllo irrigazione ogni 6 h
unsigned long timer_sonovivo;             // cronometro per il lampeggio del led verde
unsigned long timerControlloH2O;          // cronometro per il controllo livello acqua
unsigned long PROSSIMA_LETTURA;           // countdown per la prossima lettura sensori
unsigned long PROSSIMA_IRRIGAZIONE;       // countdown per la prossima irrigazione se serve
unsigned long ULTIMO_CONTATTO_CLIENT = 0; // timer per identificare l'ora di connessione del client
// ********************************************* DATI PORAMENTE COPIATI INO PRECEDENTE **********************************************************
//---------------------------------------------------- VEDERE SE SERVONO TUTTE OPPURE NO !---------------------------------------------------------


// GESTIONE USCITE TRAMITE 74HC595 E DEFINIZIONE/SPIEGAZIONE DEI SINGOLI BIT
// sono messi tutti i pin ma vengono commentati quelli non usati
uint32_t ComandoShiftReg = 0x00;     // variabile 32 bit per comandare gli Shift Register
// -- CHIP 1 : LED --  (al momento messi solo 2 led verdi extra, se ne posson mettere 5 !)
constexpr uint8_t LedVerde         = 0;     // bit di comando associato al led verde
constexpr uint8_t LedRosso         = 1;     // bit di comando associato al led rosso
constexpr uint8_t LedBlu           = 2;     // bit di comando associato al led blu
constexpr uint8_t LedExtra1V       = 3;     // bit di comando associato al led verde extra 1
constexpr uint8_t LedExtra2R       = 4;     // bit di comando associato al led verde extra 2
constexpr uint8_t LedExtra3B       = 5;     // bit di comando associato al led verde extra 3
// constexpr uint8_t LedExtra4        = 6;     // bit di comando associato al led verde extra 4
// constexpr uint8_t LedExtra5        = 7;     // bit di comando associato al led verde extra 5
// -- CHIP 2 : RELE -- (con scheda ad 8 rele impostati tutti, se non li uso ... amen, forse per pompe fiori....)
// tutti i rele sono attivati tramite fotoaccoppiatore
// rocordarsi di settare i pin della scheda in modo da attivare la "Active High" (LOW rele a riposo, HIGH attivato)
//   -- posizione 8  --  => definizione all'inizio del setup -- alimenta fotoaccoppiatore
//   -- posizione 9  --  => definizione all'inizio del setup -- alimenta fotoaccoppiatore
//   -- posizione 10 --  => definizione all'inizio del setup -- alimenta fotoaccoppiatore
constexpr uint8_t ReleComune        = 11;   // bit di comando numero 11 - al momento non usato
// constexpr uint8_t Posizione12       = 12;   // bit di comando numero 12 - al momento non usato
// constexpr uint8_t Posizione13       = 13;   // bit di comando numero 13 - al momento non usato
// constexpr uint8_t Posizione14       = 14;   // bit di comando numero 14 - al momento non usato
constexpr uint8_t PompaOrto         = 15;   // bit di comando attivazione pompa per orto
// -- CHIP 3 : ALIMENTAZIONI -- (per la gestione centralizzata delle alimentazioni schede,sensori ecc)
constexpr uint8_t AlimentazioneRele    = 16;   // bit di comando alimentazione scheda rele via transistor !
constexpr uint8_t AlimentazioneAcqua   = 17;   // bit di comando alimentazione sensore acqua
constexpr uint8_t AlimentazioneDHT22   = 18;   // bit di comando alimentazione sensore meteo
constexpr uint8_t AlimentazioneTerreno = 19;   // bit di comando alimentazione sensori terreno (qualunque tipo)
// constexpr uint8_t bitAlimentazione5    = 20;   // bit di comando alimentazione 5
// constexpr uint8_t bitAlimentazione6    = 21;   // bit di comando alimentazione 6
// constexpr uint8_t bitAlimentazione7    = 22;   // bit di comando alimentazione 7
// constexpr uint8_t bitAlimentazione8    = 23;   // bit di comando alimentazione 8

// PROTOTIPI FUNZIONI USATE NEL FW
// area operativa non web
void SCRIVI_LCD(uint8_t colonna, uint8_t riga, const char* TXT);
void SCHERMATA_ZONA(uint8_t zona_idx);          // usata con MOSTRA_DATI_SISTEMA()
void MOSTRA_DATI_SISTEMA();
void GESTIONE_PULSANTE();
void AggiornaShiftRegister(uint32_t valore);
void GESTISCI_METEO();
// area irrigazione
void GESTISCI_SENSORI_TERRENO();
void GESTISCI_IRRIGAZIONE();
void CONTROLLO_LIVELLO_CISTERNA();
void ICE_ACQUA();
// area web
void SETUP_WEB();
void LOOP_WEB();
void GESTISCI_PAGINA_WEB(WiFiClient& client, const char* pagina);
String Converti_TAG(const String& var);


void setup() {
  Serial.begin(9600); while (!Serial && millis() < 2000) { ; }   // Attende la seriale per un massimo di 2 sec senza usare variabili
  lcd.init(); lcd.backlight(); lcd.clear();                    // avvio LCD + barba e capelli, retroill. la spengo nel loop....
  // assegnazione bit per comando valvole di zona e tempi irrigazione specifici
  ZONA[1].BIT_VALVOLA_ZONA            = 8;                              // bit di comandi apertura valvola 1 -- alimenta fotoaccoppiatore
  ZONA[1].TEMPO_IRRIGAZIONE           = (50UL * 60UL * 1000UL / 10UL);  // tempo di effettiva irrorazione in millisecondi "area cudumars" 5,0 min
  ZONA[1].SOGLIA                      = SOGLIA_RH_Z1;                   // limite umidita impostato in "SETTAGGI.h"
  ZONA[1].PIN_SENSORE                 = 14;                             // pin 14 => analogico A0
  ZONA[1].DA_BAGNARE                  = false;                          // bool per la gestione della irrigazione (true => da bagnare)
  ZONA[2].BIT_VALVOLA_ZONA            = 9;                              // bit di comandi apertura valvola 2 -- alimenta fotoaccoppiatore
  ZONA[2].TEMPO_IRRIGAZIONE           = (20UL * 60UL * 1000UL / 10UL);  // tempo di effettiva irrorazione in millisecondi "area pomodori" 2,0 min
  ZONA[2].SOGLIA                      = SOGLIA_RH_Z2;                   // limite umidita impostato in "SETTAGGI.h"
  ZONA[2].PIN_SENSORE                 = 15;                             // pin 15 => analogico A1
  ZONA[2].DA_BAGNARE                  = false;                          // bool per la gestione della irrigazione (true => da bagnare)
  ZONA[3].BIT_VALVOLA_ZONA            = 10;                             // bit di comandi apertura valvola 3 -- alimenta fotoaccoppiatore
  ZONA[3].TEMPO_IRRIGAZIONE           = (20UL * 60UL * 1000UL / 10UL);  // tempo di effettiva irrorazione in millisecondi "area extra" 2,0 min
  ZONA[3].SOGLIA                      = SOGLIA_RH_Z3;                   // limite umidita impostato in "SETTAGGI.h"
  ZONA[3].PIN_SENSORE                 = 16;                             // pin 16 => analogico A2
  ZONA[3].DA_BAGNARE                  = false;                          // bool per la gestione della irrigazione (true => da bagnare)

  // pagina 1 - msg benvenuto 
  strcpy_P(W_BUFF, PSTR(" - ORTO '27 WIFI -"));                            SCRIVI_LCD(0, 0, W_BUFF);
  snprintf_P(W_BUFF, sizeof(W_BUFF), PSTR("VERSIONE SW : %s"), VERSIONE); SCRIVI_LCD(0, 1, W_BUFF);
  strcpy_P(W_BUFF, PSTR("SETUP SISTEMA..."));                             SCRIVI_LCD(0, 2, W_BUFF);
  // set pin vari
  strcpy_P(W_BUFF, PSTR("SET PIN E TIMERS"));                             SCRIVI_LCD(0, 3, W_BUFF);
  pinMode(PULSANTE, INPUT);
  pinMode(LIVELLO_H2O, INPUT);
  pinMode(FLUSSIMETRO_PIN,INPUT);  // vedere se mettere INPUT_PULLUP ..... vedi come mettere
  pinMode(DHT_PIN,INPUT);
  for (int k=14; k<=16; k++) { pinMode (k,INPUT); } // set input ad A0,A1,A2 => sensori zona
  // pinMode (A3, INPUT);   // per l'eventuale sensore luminosita/LUX al momento non usato
  // pin 74HC595
  pinMode(pinData, OUTPUT); pinMode(pinLatch, OUTPUT); pinMode(pinClock, OUTPUT);
  // timers
  timer_retroilluminazione = millis();                   // set timer per illuminazione lcd
  timer_letture = millis() - INTERVALLO_LETTURE;          // faccio la prima lettura appena parte il loop...
  timer_irrigazione = millis();                           // settaggio crono per verifica terreni
  timer_display = millis() - REFRESH_LCD_MODO_ORDINARIO;  // settaggio crono per aggiornamento display
  timer_sonovivo = millis();                              // settaggio crono per lampeggio led verde
  timerControlloH2O = millis();                           // settaggio crono per controllo livello acqua  
  delay(350);
  lcd.clear();
  // pagina 2
  strcpy_P(W_BUFF, PSTR("TEST LED"));  SCRIVI_LCD(0, 0, W_BUFF);
  // test led on
  bitWrite(ComandoShiftReg, LedVerde, HIGH);
  bitWrite(ComandoShiftReg, LedRosso, HIGH);
  bitWrite(ComandoShiftReg, LedBlu,   HIGH);
  // aggiungere gli altri led se li inserisci
  AggiornaShiftRegister (ComandoShiftReg);
  delay(300);
  // test led off
  ComandoShiftReg = 0x00;     // rimetti tutta la variabile di comando a 0
  AggiornaShiftRegister (ComandoShiftReg);
  // controllo chiusura valvole zona 
  // NB le attivazioni, come programmate soto, devono essere impostate su scheda ad "Active High"
  strcpy_P(W_BUFF, PSTR("CHIUDO VALVOLE"));  SCRIVI_LCD(0, 1, W_BUFF);
  bitWrite(ComandoShiftReg, LedExtra1V, HIGH);
  bitWrite(ComandoShiftReg, AlimentazioneRele, HIGH);
  bitWrite(ComandoShiftReg, ReleComune, HIGH);
  AggiornaShiftRegister (ComandoShiftReg);


  delay (APERTURA_VALVOLA_ZONA /10); // togliereo il /10 !!!
  
  
  ComandoShiftReg = 0x00;     // rimetti tutta la variabile di comando a 0
  AggiornaShiftRegister (ComandoShiftReg);
  strcpy_P(W_BUFF, PSTR("VALVOLE CHIUSE"));  SCRIVI_LCD(0, 1, W_BUFF);
  strcpy_P(W_BUFF, PSTR("AVVIO DHT METEO"));  SCRIVI_LCD(0, 2, W_BUFF);  // inizializzazione sensore meteo
  // interrupt
  pinMode(2, INPUT_PULLUP); // O INPUT a seconda del sensore <---- VEDERE CHE SENCORE !!!
  attachInterrupt(digitalPinToInterrupt(FLUSSIMETRO_PIN), contaImpulsi, RISING);
  // interrupt
  dht.begin();
  strcpy_P(W_BUFF, PSTR("AVVIO WEB..."));  SCRIVI_LCD(0, 3, W_BUFF);
  delay (400);  // barba e capelli
  // pagina 3 - settaggio parametri web con funzione dedicata
  SETUP_WEB();
  strcpy_P(W_BUFF, PSTR("- FINE SETUP -")); SCRIVI_LCD(0, 3, W_BUFF);
  delay(350);
  lcd.clear();
}
// --------------------------- FINE SETUP  ------------------------------

void loop() {
  
  
  // SPEGNI RETROILLUMINAZIONE LCD dopo 1 minuto
 // if (millis()-timer_retroilluminazione > (60UL*1000UL)) { lcd.noBacklight(); } 

  // LAMPEGGIO SONO VIVO: leggo il bit nella variabile e lo riscrivo invertito, poi lo invio....
  if (millis() - timer_sonovivo > SONO_VIVO) { 
    bitWrite(ComandoShiftReg, LedVerde, !bitRead(ComandoShiftReg, LedVerde));
    AggiornaShiftRegister (ComandoShiftReg);
    timer_sonovivo = millis();
  }
  
  // verifica livello acqua
  if (millis() - timerControlloH2O > INTERVALLO_LETTURA_ACQUA) {
    LETTURA_ACQUA = true; // faccio partire la funzione di controllo (300 millis)
    timerControlloH2O = millis(); // e poi resetto il timer...
  }
  if (LETTURA_ACQUA) { CONTROLLO_LIVELLO_CISTERNA(); }
  // SE NON C'E' ACQUA .... In Case of Emergency (ICE)
  if (ACQUA.STATO_CISTERNA == false) { ICE_ACQUA(); } // attiva ICE acqua....

// legge sensori terreno e sensore meteo con lo stesso intervallo temporale
// ma bool di attivazione diverso altrimenti la lettura meteo (piu lenta dell'altra) si bloccherebbe a meta
  if (millis() - timer_letture >= INTERVALLO_LETTURE) {
    LEGGI_SENSORI = true;
    LEGGI_METEO   = true;
    timer_letture = millis();
  }
  // ogni mezzora legge i sensori e i dati meteo
  if (LEGGI_SENSORI) { GESTISCI_SENSORI_TERRENO(); }
  if (LEGGI_METEO)   { GESTISCI_METEO();           }

  // controllo temporizzato timer possibili irrigazioni, se è passato il tempo attiva il flag
  // che poi nella funzione farà partire l'irrigazione vera e propria
  if (millis() - timer_irrigazione >= INTERVALLO_IRRIGAZIONI) {
    ATTIVA_IRRIGAZIONE = true;
    timer_irrigazione = millis();  // Resetta il timer per il prossimo controllo
  }
  // entra in qualsiasi caso nella funzione, se non è attivato nulla esce subito, altrimenti prosegue con l'irrigazione
  GESTISCI_IRRIGAZIONE();

  // Controlla la pressione del pulsante multifinzione
  GESTIONE_PULSANTE();

  // Visualizza i dati del sistema nelle pagine lcd
  MOSTRA_DATI_SISTEMA();


  // avvia parte web tramite fuzunzione dedicata
  LOOP_WEB();

Serial.println ("sono nel loop"); delay (1500);
} // fine loop...
