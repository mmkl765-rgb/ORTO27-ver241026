// HOME PAGE del sistema ORTO WEB. versione 1: con solo un bottone link per l'irrigazione di tutte le zone, 
// come fatto nella parte senza web, in futuro potrei fare l'irrigazione forzata per zona....

const char HOME_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <!-- aggiorna automaticamente ogni 30 sec -->
    <meta http-equiv="refresh" content="30">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Orto 2027 - Controllo Remoto</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #f0f2f5; margin: 0; padding: 15px; color: #333; }
        h1 { text-align: center; color: #2e7d32; font-size: 24px; margin-bottom: 20px; }
        .container { max-width: 600px; margin: 0 auto; }
        .card { background: white; padding: 15px; margin-bottom: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .card-title { font-size: 18px; font-weight: bold; margin-bottom: 10px; display: flex; align-items: center; gap: 8px; justify-content: center; }
        .data-row { display: flex; justify-content: space-between; padding: 6px 0; border-bottom: 1px solid #eee; }
        .data-row:last-child { border-bottom: none; }
        
        /* STILI UNIFICATI PER LE RIGHE DELLE ZONE / ACQUA */
        .zone-row { display: flex; justify-content: space-between; align-items: center; padding: 8px 0; border-bottom: 1px solid #eee; font-size: 14px; }
        .zone-row:last-child { border-bottom: none; }
        
        .zone-name { font-weight: bold; width: 25%; text-align: left; }
        .zone-data  { width: 25%; text-align: center; }
        .zone-status { width: 25%; text-align: center; }
        .zone-mis    { width: 25%; text-align: left; }
        
        /* Specifico per la tabella acqua a 4 colonne */
        .water-col { width: 24%; text-align: center; }
        .water-col:first-child { text-align: left; }

        .stato-ok { background-color: #e8f5e9; border-left: 5px solid #4caf50; color: #2e7d32; }
        .stato-allarme { background-color: #ffebee; border-left: 5px solid #f44336; color: #c62828; }
        
        .legenda-z1 { color: #2196f3; } /* Blu per Zona 1 */
        .legenda-z2 { color: #ff5722; } /* Arancione per Zona 2 */
        .legenda-z3 { color: #9c27b0; } /* Viola per Zona 3 */

        /* STILE UNICO PER TUTTI I BOTTONI BLU */
        .btn-bagna { 
            display: block; 
            text-align: center; 
            background: linear-gradient(135deg, #2196f3 0%, #1976d2 100%); 
            color: white; 
            padding: 12px 20px; 
            margin-top: 15px; 
            border-radius: 6px; 
            text-decoration: none; 
            font-weight: bold; 
            box-sizing: border-box;
            box-shadow: 0 4px 6px rgba(33, 150, 243, 0.3);
            transition: all 0.2s ease-in-out;
            letter-spacing: 0.5px;
        }
        .btn-bagna:hover { 
            background: linear-gradient(135deg, #1976d2 100%, #1565c0 100%);
            box-shadow: 0 6px 12px rgba(33, 150, 243, 0.4);
            transform: translateY(-1px);
        }
        .btn-bagna:active {
            transform: translateY(1px);
            box-shadow: 0 2px 4px rgba(33, 150, 243, 0.4);
        }
        /* Sfondo scuro a tutto schermo quando la modale è attiva */
    .modal-overlay {
        display: none; /* Di default è nascosta */
        position: fixed;
        top: 0; left: 0; width: 100%; height: 100%;
        background-color: rgba(0, 0, 0, 0.6);
        z-index: 999;
        justify-content: center;
        align-items: center;
    }

    /* La finestrella "bellina" al centro */
    .modal-box {
        background: white;
        padding: 25px;
        border-radius: 12px;
        box-shadow: 0 8px 24px rgba(0,0,0,0.3);
        text-align: center;
        max-width: 320px;
        width: 85%;
        animation: modalPop 0.3s ease-in-out;
    }

    /* Effetto comparsa dinamico */
    @keyframes modalPop {
        0% { transform: scale(0.8); opacity: 0; }
        100% { transform: scale(1); opacity: 1; }
    }

    .modal-icon {
        font-size: 40px;
        margin-bottom: 10px;
    }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌿 🌿 🌿 🌿 🌿 🌿 🌿 🌿 🌿 🌿</h1>
        <h1>ORTO 2027 - %VERSIONE_WW% - WIFI</h1>

        <!-- CARD METEO ATTUALE (UGUALE ALLE ALTRE) -->
        <div class="card">
            <div class="card-title">🌤️&nbsp;&nbsp;&nbsp;&nbsp;SITUAZIONE METEO&nbsp;&nbsp;&nbsp;&nbsp;🌤️</div>
            <div class="data-row"><span>Temperatura:</span> <strong>%TEMP_WW%</strong> °C</div>
            <div class="data-row"><span>Umidità Aria (HR):</span> <strong>%UM_WW%</strong> /100</div>
            <!-- Pronte per il futuro 
            <div class="data-row"><span>Pressione:</span> <strong>%PRESS_WW%</strong> hPa</div>
            <div class="data-row"><span>Luminosità (Lux):</span> <strong>%LUX_WW%</strong> lx</div>
            -->
        </div>
        
        <!-- CARD GESTIONE ACQUA -->
        <div class="card">
            <div class="card-title">&#128167;&#128167;&nbsp;&nbsp;&nbsp;&nbsp;GESTIONE ACQUA&nbsp;&nbsp;&nbsp;&nbsp;&#128167;&#128167;</div>
            <!-- Stato Generale -->
            <div class="data-row"><span>Cisterna:</span>
		        <strong>%STATO_CISTERNA_WW%</strong>
                <strong>Restanti: %LITRI_CISTERNA_WW% LT (Stima)</strong>
            </div>
            
            <!-- Tabella Acqua per Zona -->
            <div class="zone-row" style="font-weight: bold; color: #666;">
                <span class="water-col">Zona</span>
                <span class="water-col">Ultima irr.</span>
                <span class="water-col">Litri tot.</span>
                <span class="water-col">N. Irr.</span>
            </div>
            <div class="zone-row">
                <span class="zone-name legenda-z1">Zucchine</span> <!-- NOME DA METTERE A MANO -->
                <span class="water-col">%LT_ULT_IRR_Z1_WW% litri</span>
                <span class="water-col">%LT_TOT_Z1_WW% litri</span>
                <span class="water-col">%N_IRR_Z1_WW%</span>
            </div>
            <div class="zone-row">
                <span class="zone-name legenda-z2">Pomodori</span> <!-- NOME DA METTERE A MANO -->
                <span class="water-col">%LT_ULT_IRR_Z2_WW% litri</span>
                <span class="water-col">%LT_TOT_Z2_WW% litri</span>
                <span class="water-col">%N_IRR_Z2_WW%</span>
            </div>
            <div class="zone-row">
                <span class="zone-name legenda-z3">Verdura</span> <!-- NOME DA METTERE A MANO -->
                <span class="water-col">%LT_ULT_IRR_Z3_WW% litri</span>
                <span class="water-col">%LT_TOT_Z3_WW% litri</span>
                <span class="water-col">%N_IRR_Z3_WW%</span>
            </div>

          <!-- NOTA A SCOMPARSA 1 -->
            <details style="margin-top: 15px; background: #f8f9fa; padding: 8px 12px; border-radius: 6px; font-size: 13px; border: 1px solid #e9ecef;">
                <summary style="cursor: pointer; font-weight: bold; color: #2e7d32; outline: none; display: flex; align-items: center; gap: 6px;">
                    <span>💡</span> Info Acqua e Riavvio
                </summary>
                <p style="margin: 8px 0 0 0; color: #555; line-height: 1.4;">
                    Se appare la dicitura <strong>Acqua Finita</strong>, il sistema è entrato in un "blocco" nel quale la parte operativa (irrigazione) è
		    bloccata per evitare la rottura e la cavitazione della pompa. Se dovesse capitare che vada in blocco durante una irrigazione, il sistema si 
		    mette in sicurezza da solo bloccando la pompa e chiudendo le valvole. Per ristabilire la modalità ordinaria è richiesto la presenza di un 
		    operatore che riempia la cisterna e poi prema il pulsante di riavvio. Il sistema ripartirà dal punto dove si era fermato rileggendo prima
		    di tutto il livello dell'acqua in cisterna.
                </p>
            </details>

        </div>

        <!-- CARD STATO DELLE ZONE -->
        <div class="card">
            <div class="card-title">&#129362;&nbsp;&nbsp;&#127793;&nbsp;&nbsp;STATO DELLE ZONE&nbsp;&nbsp;&#127793;&nbsp;&nbsp;&#129362;</div>

            <!-- Zona 1 (Zucchine) -->
            <div class="zone-row">
                <span class="zone-name legenda-z1"> ■ Zucchine</span> <!-- NOME DA METTERE A MANO -->
                <span class="zone-data">Umidità: <strong>%UMIDITA_Z1_WW% /100</strong></span>
                <span class="zone-status">Stato: <strong>%STATO_Z1_WW%</strong></span>
                <span class="zone-mis">Letture: <strong>%N_MISURAZIONI_Z1_WW%</strong></span>
            </div>

            <!-- Zona 2 -->
            <div class="zone-row">
                <span class="zone-name legenda-z2"> ■ Cudumaru</span> <!-- NOME DA METTERE A MANO -->
                <span class="zone-data">Umidità: <strong>%UMIDITA_Z2_WW% /100</strong></span>
                <span class="zone-status">Stato: <strong>%STATO_Z2_WW%</strong></span>
                <span class="zone-mis">Letture: <strong>%N_MISURAZIONI_Z2_WW%</strong></span>
            </div>

            <!-- Zona 3 -->
            <div class="zone-row">
                <span class="zone-name legenda-z3"> ■ Verdura</span> <!-- NOME DA METTERE A MANO -->
                <span class="zone-data">Umidità: <strong>%UMIDITA_Z3_WW% /100</strong></span>
                <span class="zone-status">Stato: <strong>%STATO_Z3_WW%</strong></span>
                <span class="zone-mis">Letture: <strong>%N_MISURAZIONI_Z3_WW%</strong></span>
            </div>

            <br>
            <!-- NOTA A SCOMPARSA 2  -->
            <details style="margin-top: 15px; background: #f8f9fa; padding: 8px 12px; border-radius: 6px; font-size: 13px; border: 1px solid #e9ecef;">
                <summary style="cursor: pointer; font-weight: bold; color: #2e7d32; outline: none; display: flex; align-items: center; gap: 6px;">
			📈&nbsp;&nbsp; Storico Terreno e Grafico
                </summary>
                <br>
                <!-- inclusione dinamica del grafico generato al momento -->
                %GRAFICO_SVG_WW%                
                <ul style="margin: 8px 0 0 0; color: #555; line-height: 1.4;">
                    <li> Lo storico usa i dati memorizzati da Arduino e li trasforma in un grafico.</li>
		            <li> Il tratteggo indica il valore soglia tra secco e bagnato.</li>
		            <li> Le varie zone hanno lo stesso colore usato nella tabella sopra.</li>
		            <li> Sono prese in considerazione le misurazioni delle ultime 30 ore.</li>
                </ul>
            </details>
        

        <!-- CARD IRRIGAZIONE FORZATA -->
        <div class="card">
            <div class="card-title">&#128167;&#128167;&nbsp;&nbsp;&nbsp;&nbsp;IRRIGAZIONE FORZATA&nbsp;&nbsp;&nbsp;&nbsp;&#128167;&#128167;</div>
            <a class="btn-bagna" href="/bagna">&#128167; Avvia Irrigazione Forzata</a>
            <small style="display: block; text-align: center; color: #666; margin-top: 12px;">N.B.: Il sistema sembrerà bloccato durante l'irrigazione.</small>
        </div>
        
        <!-- CARD INFO SISTEMA -->
        <div class="card">
            <div class="card-title">🌿🌿&nbsp;&nbsp; INFO -FW- ORTO 2026 &nbsp;&nbsp;🌿🌿</div>
            <div class="data-row"><span>Versione SW:</span><strong>%VERSIONE_WW%</strong></div>
            <div class="data-row"><span>Acceso da:</span><strong>%UPTIME_WW%</strong></div>
        </div> 

        <div style="text-align: center; font-size: 13px; color: #888; margin-top: 20px; margin-bottom: 10px;">
            ORTO AUTOMATICO - VERSIONE %VERSIONE_WW%<br>
            &copy; 2026 MIK. Tutti i diritti riservati. <br>
            <span style="font-size: 11px; color: #aaa;">Proprietà intellettuale Mik ed Edo</span>
        </div>
    </div>

<!-- FINESTRA MODALE IRRIGAZIONE IN ATTO  -->
<div id="modalIrrigazione" class="modal-overlay" style="display: %STATO_MODALE%;">
    <div class="modal-box">
        <div class="modal-icon">⛔ 💧🚿💧 ⛔</div>
        <h3 style="color: #2e7d32; margin-top: 0; margin-bottom: 10px;">💧 <u><b>IRRIGAZIONE IN ATTO</b></u> 💧</h3>
        <p style="color: #555; font-size: 14px; margin-bottom: 20px;">
            Il sistema sta bagnando le zone dell'orto in modalità automatica o forzata.<br><br>
	    <strong>Attendere il completamento del ciclo...</strong>
        </p>
    </div>
</div>



</body>
</html>
)rawliteral";