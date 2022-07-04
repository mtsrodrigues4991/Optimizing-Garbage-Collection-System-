#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "Ultrasonic.h" //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the prototype TTN
// network initially.
//ttn
static const PROGMEM u1_t NWKSKEY[16] = { 0x0B, 0xDD, 0x76, 0x43, 0xAE, 0xD8, 0xF7, 0x1E, 0x5A, 0xC8, 0xB1, 0x0C, 0xD3, 0x37, 0xAE, 0x80 };
// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the prototype TTN
// network initially.
//ttn
static const u1_t PROGMEM APPSKEY[16] = { 0xAF, 0x8D, 0x64, 0xE3, 0x88, 0xD4, 0xEE, 0x3C, 0x3B, 0xCB, 0x61, 0xA1, 0xE0, 0xF2, 0xDF, 0x1E };

//
// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// ttn
static const u4_t DEVADDR = 0x260D398E;



int contador = 0;               //variável para organização do número de mensagem enviadas
int distancia;                  //Variável que recebe o valor da distancia do sensor até o fundo da lixeira

float calc_bateria;             //armazena porcentagem de bateria atual (valor estimado)
float temp_consumido;           //armazena tempo em que o progrma está ligado
float temp_max;                //tempo máximo que a bateria suporta o arduino 
float power = 61.2;           // (6.8A*9V) = 61.2 Watts/hora, Energia para bateria de 6800mA/h
float corrente = 0.015;       //corrente consumida (valor medido) pelo arduino
                  
String s_tmpDist;
String s_tmpBatt;

char const *c_dist;
char const *c_batt;

const int echoPin = 5;           //Pino digital utilizado pelo sensor ultrassônico HC-SR04 ECHO(RECEBE)
const int trigPin = 4;           //Pino digital utilizado pelo sensor ultrassônico HC-SR04 ECHO(ENVIA)
 
Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }


static uint8_t mydata[] = { 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t mydata_buff[] = { 0, 0, 0, 0, 0, 0, 0, 0};
static osjob_t initjob, sendjob, blinkjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 10;   //intervalo entre o envio das mensagens, valor em segundos

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 9,
  .dio = {2, 6, 7},
};
void do_send(osjob_t* j) {
          
          strcpy ((char*)mydata_buff, c_dist);                   // copio valor da distancia para mydata_buff
          strcat ((char*)mydata_buff, "|");                      // concateno | com o valor da distancia q já esta no mydata_buff
          strcpy ((char*)mydata, (char*)mydata_buff);           //  copio valor da mydata_buff para mydata
          strcat ((char*)mydata, c_batt);                      //  concateno | com o valor da bateria com q já esta no mydata (c_dist|)
         
          // Check if there is not a current TX/RX job running
          if (LMIC.opmode & OP_TXRXPEND) {
            Serial.println(F("OP_TXRXPEND, not sending"));
          } else {
            
            // Prepare transmission at the next possible time.
            LMIC_setTxData2(1, mydata, strlen((char*) mydata), 0);
            Serial.println(); 
            Serial.println("Dados lidos");
            Serial.print("Envio nº: ");
            Serial.println(contador);
            contador++;
                        
            //Serial.println(LMIC.freq);
            Serial.println(LMIC.freq);
            
            Serial.print("Distancia na lixeira ");                    //Imprime o txt entre "" no monitor serial
            Serial.print(distancia);                                  //Imprime o valor da distancia mo monitor serial
            Serial.println(" cm");                                    //Imprime o txt entre "" no monitor serial

            Serial.print("Bateria Calculada");                       //Imprime o txt entre "" no monitor serial
            Serial.print(calc_bateria);                              //Imprime o valor da estimativa de bateria mo monitor serial
            Serial.println(" %");                                    //Imprime o txt entre "" no monitor serial
            
            Serial.print("Payload ");                               //Imprime o txt entre "" no monitor serial
            Serial.print((char*)mydata);                            //Imprime o payload enviado mo monitor serial
            Serial.println(" ");                                    //Imprime o txt entre "" no monitor serial          

            Serial.print("Tempo ligado ");                          //Imprime o txt entre "" no monitor serial
            Serial.print(temp_consumido);                           //Imprime o valor do tempo em que o programa está ligado mo monitor serial
            Serial.println(" ms");                                  //Imprime o txt entre "" no monitor serial     

  }
  
  }


void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  Serial.println(ev);
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println("EV_SCAN_TIMEOUT");
      break;
    case EV_BEACON_FOUND:
      Serial.println("EV_BEACON_FOUND");
      break;
    case EV_BEACON_MISSED:
      Serial.println("EV_BEACON_MISSED");
      break;
    case EV_BEACON_TRACKED:
      Serial.println("EV_BEACON_TRACKED");
      break;
    case EV_JOINING:
      Serial.println("EV_JOINING");
      break;
    case EV_JOINED:
      Serial.println("EV_JOINED");
      break;
    case EV_RFU1:
      Serial.println("EV_RFU1");
      break;
    case EV_JOIN_FAILED:
      Serial.println("EV_JOIN_FAILED");
      break;
    case EV_REJOIN_FAILED:
      Serial.println("EV_REJOIN_FAILED");
      break;
    case EV_TXCOMPLETE:
      Serial.println("EV_TXCOMPLETE (includes waiting for RX windows)");
      if (LMIC.dataLen) {
        // data received in rx slot after tx
        Serial.print("Data Received: ");
        Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        Serial.println();
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println("EV_LOST_TSYNC");
      break;
    case EV_RESET:
      Serial.println("EV_RESET");
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println("EV_RXCOMPLETE");
      break;
    case EV_LINK_DEAD:
      Serial.println("EV_LINK_DEAD");
      break;
    case EV_LINK_ALIVE:
      Serial.println("EV_LINK_ALIVE");
      break;
    default:
      Serial.println("Unknown event");
      break;
  }
}

void setup() {
  pinMode(echoPin, INPUT); //DEFINE O PINO COMO ENTRADA (RECEBE)
  pinMode(trigPin, OUTPUT); //DEFINE O PINO COMO SAIDA (ENVIA
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  
  analogReference(INTERNAL);
  while (!Serial);
  Serial.println("Iniciando");
#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  //LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF10, 14);

  for (int i = 1; i < 64; i++)
  {
    LMIC_disableChannel(i);  // only the first channel 902.3Mhz works now.
  }
  
  do_send(&sendjob);   // Start job distancia
  
}

void loop() {
  
  os_runloop_once();
  hcsr04();                                               //função que faz a leitura da distancia até o funda da lixeira
  
  unsigned long t1 = millis();                            //registra o tempo em que o programa foi iniciado. Vaor é inteiro, porém é registrado em milisegundos
  temp_consumido = float(t1/100000);                      // tempo em que o programa foi iniciado, valor em segundos
  
  for(int i=0; i<250; i++) {                             //Função que faz a projeção do consumo de bateria
   temp_max = power/corrente;                            //tempo maximo que a bateria aguenta  18 dias ou 433.3h                                 
   calc_bateria=  ((temp_consumido/temp_max)*100);      //porcentagem da bateria(valor estimado)
   }
}

void hcsr04(){
  
    //processo de leitura da distancia, realizado pelo sensor  ultrassônico HC-SR04 ECHO
    digitalWrite(trigPin, LOW);            //SETA O PINO 6 COM UM PULSO BAIXO "LOW"
    delayMicroseconds(2);                    //INTERVALO DE 2 MICROSSEGUNDOS
    digitalWrite(trigPin, HIGH);             //SETA O PINO 6 COM PULSO ALTO "HIGH"
    delayMicroseconds(10);                  //INTERVALO DE 10 MICROSSEGUNDOS
    digitalWrite(trigPin, LOW);             //SETA O PINO 6 COM PULSO BAIXO "LOW" NOVAMENTE

    distancia = (ultrasonic.Ranging(CM));   //converte valor lido pelo sensor para valor em cm (variável int)

    int i_dist = distancia;
    int i_batt = int(calc_bateria);        //converte porcentagem da estimativa de bateria calculada para int

    
    s_tmpDist = String(i_dist);           //converte valor da distancia para string
    s_tmpBatt = String(i_batt);           //converte valor da estimativa de bateria para string
    
    c_dist = s_tmpDist.c_str();           //converte valor da string de distancia para char const 
    c_batt = s_tmpBatt.c_str();           //converte valor da string de estimativa de bateria para char const 

    delayMicroseconds(10);               //Intervalor de 10 microsegundos

 }
