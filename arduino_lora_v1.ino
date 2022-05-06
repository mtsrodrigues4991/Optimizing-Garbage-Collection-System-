#include <avr/sleep.h>               //this AVR library contains the methods that controls the sleep modes
#define interruptPin 2              //28 digital Pin we are going to use to wake up the Arduino
#include <DS3232RTC.h>               //RTC Library https://github.com/JChristensen/DS3232RTC
#include "Ultrasonic.h" //INCLUSÃO DA BIBLIOTECA NECESSÁRIA PARA FUNCIONAMENTO DO CÓDIGO
 
const int echoPin = 7; //PINO DIGITAL UTILIZADO PELO HC-SR04 ECHO(RECEBE)
const int trigPin = 6; //PINO DIGITAL UTILIZADO PELO HC-SR04 TRIG(ENVIA)
 
Ultrasonic ultrasonic(trigPin,echoPin); //INICIALIZANDO OS PINOS DO ARDUINO
 
int distancia; //VARIÁVEL DO TIPO INTEIRO
String result; //VARIÁVEL DO TIPO STRING
    
//RTC Module global variables
const int time_interval=1;          // Sets the wakeup interval 2 in minutes

void setup() {
  
  Serial.begin(9600);//Start Serial Comunication
  
  pinMode(echoPin, INPUT); //DEFINE O PINO COMO ENTRADA (RECEBE)
  pinMode(trigPin, OUTPUT); //DEFINE O PINO COMO SAIDA (ENVIA)
  
  pinMode(LED_BUILTIN,OUTPUT);            //We use the led on pin 13 to indecate when Arduino is A sleep
  pinMode(interruptPin,INPUT_PULLUP);     //Set pin d2 to input using the buildin pullup resistor
  digitalWrite(LED_BUILTIN,HIGH);         //turning LED on
  
  // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
    RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
    RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
    RTC.alarm(ALARM_1);
    RTC.alarm(ALARM_2);
    RTC.alarmInterrupt(ALARM_1, false);
    RTC.alarmInterrupt(ALARM_2, false);
    RTC.squareWave(SQWAVE_NONE);
    
    /* Uncomment the block block to set the time on your RTC. Remember to comment it again 
    // * otherwise you will set the time at everytime you upload the sketch
     
    // Begin block
    tmElements_t tm;
    tm.Hour = 11;               // set the RTC to an arbitrary time
    tm.Minute = 12;
    tm.Second = 00;
    tm.Day = 06;
    tm.Month = 05;
    tm.Year = 2030 - 1994;      // tmElements_t.Year is the offset from 1970
    RTC.write(tm);              // set the RTC from the tm structure
//    Block end */

    
    time_t t; //create a temporary time variable so we can set the time and read the time from the RTC
    t=RTC.get();//Gets the current time of the RTC
    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);// Setting alarm 1 to go off 1 minutes from now
    
    // clear the alarm flag
    RTC.alarm(ALARM_1);
    
    // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
    RTC.squareWave(SQWAVE_NONE);
    
    // enable interrupt output for Alarm 1
    RTC.alarmInterrupt(ALARM_1, true);
    
    //dht.begin();        //Start the DHT sensor
    Serial.println("Iniciando em 0.5s, aguarde");
    delay(500);
}

void loop() {
 delay(500);//wait 5 seconds before going to sleep. In real senairio keep this as small as posible
 Going_To_Sleep();

}

void Going_To_Sleep(){
    Serial.println("Indo dormir");
    delay(500);
    
    sleep_enable();//Enabling sleep mode
    attachInterrupt(0, wakeUp, LOW);          //attaching a interrupt to pin d2
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);      //Setting the sleep mode, in our case full sleep
    digitalWrite(LED_BUILTIN,LOW);            //turning LED off
    
    time_t t;                                 // creates temp time variable
    t=RTC.get();                              //gets current time from rtc
    Serial.println("Hora que dormiu: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));    //prints time stamp on serial monitor
    delay(1000);                            //wait a second to allow the led to be turned off before going to sleep
    
    sleep_cpu();                                            //activating sleep mode
    Serial.println("Acordei agora! esse horário: " );        //next line of code executed after the interrupt 
    digitalWrite(LED_BUILTIN,HIGH);                          //turning LED on
//    temp_Humi();//function that reads the temp and the humidity
    hcsr04();                               //This function reads the temperature and humidity from the HCSR04 sensor
    delay(1000);
    Serial.print("Distancia "); //IMPRIME O TEXTO NO MONITOR SERIAL
    Serial.print(result); ////IMPRIME NO MONITOR SERIAL A DISTÂNCIA MEDIDA
    Serial.println("cm"); //IMPRIME O TEXTO NO MONITOR SERIAL
    
    t=RTC.get();
    Serial.println("Hora que acordou: "+String(hour(t))+":"+String(minute(t))+":"+String(second(t)));//Prints time stamp 
    
    //Set New Alarm
    RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t)+time_interval, 0, 0);
  
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  }

void wakeUp(){
  Serial.println("Interrupção detectada");//Print message to serial monitor
   sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
 }

//This function reads the temperature and humidity from the HCSR04 sensor
void hcsr04(){
    digitalWrite(trigPin, LOW); //SETA O PINO 6 COM UM PULSO BAIXO "LOW"
    delayMicroseconds(2);         //INTERVALO DE 2 MICROSSEGUNDOS
    digitalWrite(trigPin, HIGH); //SETA O PINO 6 COM PULSO ALTO "HIGH"
    delayMicroseconds(10);      //INTERVALO DE 10 MICROSSEGUNDOS
    digitalWrite(trigPin, LOW); //SETA O PINO 6 COM PULSO BAIXO "LOW" NOVAMENTE
    
    //FUNÇÃO RANGING, FAZ A CONVERSÃO DO TEMPO DE
    //RESPOSTA DO ECHO EM CENTIMETROS, E ARMAZENA
    //NA VARIAVEL "distancia"
    distancia = (ultrasonic.Ranging(CM)); //VARIÁVEL GLOBAL RECEBE O VALOR DA DISTÂNCIA MEDIDA
    result = String(distancia); //VARIÁVEL GLOBAL DO TIPO STRING RECEBE A DISTÂNCIA(CONVERTIDO DE INTEIRO PARA STRING)
    delay(500); //INTERVALO DE 500 MILISSEGUNDOS
 }
 
