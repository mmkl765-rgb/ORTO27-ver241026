// FILE CONTENENTE LE VARIE CONFIGURAZIONI DELLE RETI

  const char ssid_router[] = "al_momento_no";   // al momento non arriva il segnale
  const char pass_router[] = "al_momento_no";   // al momento non arriva il segnale
  const char ssid_AP[]     = "Orto_Smart_AP";   // nome della rete dell'orto in AP
  const char pass_AP[]     = "12345678";        // pwd rete in AP (almeno 8 caratteri) (metti una + difficile poi !!!!)

  // --- IMPOSTAZIONI IP FISSO PER ROUTER (station STA) ---
  IPAddress sta_IP(192, 168, 1, 247);      // IP fisso che vuoi dare ad Arduino
  IPAddress sta_gateway(192, 168, 1, 1);   // IP del tuo router di casa
  IPAddress sta_subnet(255, 255, 255, 0);  // Subnet mask standard
  IPAddress sta_dns(192, 168, 1, 1);       // Server DNS (pari al router)

  // --- IMPOSTAZIONI PER L'ACCESS POINT (AP) ---
  // indirizzo generico-automatico di arduino : 192.168.4.1
  // non serve impostarlo come variabile!!!
  // IPAddress ap_IP(192, 168, 4, 1);        // IP fisso per la rete creata da Arduino 
  