// Solarwechselrichter MK2 de dd3et
// mit 1111 seriellen Monitor freigeben, mit 1000 deaktivieren
// https://www.etechnophiles.com/how-to-change-the-pwm-frequency-of-arduino-nano/
// TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz on D3 & D11
#include <EEPROM.h>
// Widerstand
byte widerstandspin = 12;
bool widerstand;
bool widerstandvorher;
// Relais
byte relpin = 9;
byte haltepwmwert = 102;
bool anzug;
bool anzugvorher;
bool anzugflankesteigend;
bool anzugflankefallend;
bool relpwm;
bool relpwmvorher;
bool relpwmflanke;
unsigned long anzugflankeist;
unsigned long anzugflankestart;
byte dcdcanpin = 3;
bool dcdcan;

bool buckanflipflop;
bool buckan;
bool buckanvorher;
bool buckansteigend;
bool buckanfallend;

byte pvspgpin = A0;
byte netzoptopin = A1;
int pvwert;
int istspannung;
int sollspannung;
int mppwert; // Wert = Spannung * 22,2/V,  640 bis 900 
int mppsollwert;   // wird aus der Temperatur berechnet
int mppoberwert;
int  mppunterwert;
int mpphysteresewert = 22; // 22 = 1V * 22,2/V nach oben und nach unten
byte buckspgpin = A2;
int buckUwert;
byte buckstrompin = A3;
int buckIwert;
int buckIwertvorher;
int buckIlimit;
int buckImaxwert; // für Leistungsbegrenzung 450_140W; 500_160W; 600_190W; 700_225W; 800_255W; 900_295W; 920_300W
int mleistung;
int aleistung;
long mPwert;
long aPwert;
byte tempmessen;
byte gehtemppin = A4;
int gehtempwert;
int gehtemp;
bool thfuse;
byte dcdctemppin = A5;
int dcdctempwert;
int dcdctemp;
byte modultemppin = A6;
int modultemp;
int modultempwert;
int modultempprozent;
byte bucktemppin = A7;
int bucktempwert;
int bucktemp;

// eeprom
int getmPwert; // aus eeprom
int getmppwert; // aus eeprom
int eingabewert;
int pruefsummand;
int getpruefsummand;
int pruefsumme = 999;
int pruefzahl;
int mppspg; // spannungseingabe für mppwert in den seriellen Moniotor
bool putflag;

// Nulldurchgang
// Variablen für die Einstellungen
bool nulldurchgang;
bool nulldurchgangvorher;
bool nulldurchgangspuls;
bool nulldurchgangsflipflop;
int flankenkorrektur = 100;       // Steigende Flanke des Scheitelmonoflops genau in den Nulldurchgang legen (Oszilloskop!)
byte nulldurchgangspin = 2; 
unsigned long startzeit;

unsigned long nulldurchgangsabstand;
unsigned long netzfrequenz;
unsigned long netzFrequenz;
bool netzfrequenzgut;

// Scheitelpunkt
bool scheitelmonoflop;
bool scheitelmonoflopvorher;
bool scheitelpunkt;
bool enscheitelmono;
unsigned long act;
long mod;
byte teilerwert = 0;
byte teilerzahl = 8;  // heraufahren 
bool teiler;

// Netzspannung
int netzwert = 1020;
bool netz;
unsigned long modulationsgrad;
unsigned long modbegrenzung;

// Modulation (Pin 11)
int modulationswert;
int modulationswertvorher;
int lztp = 500;            // Laufzeitkorrektur 445 TP

// Frequenzumschaltung
byte dcdcfrequenzpin = 8;
bool dcdcfrequenz;
bool dcdcfrequenzvorher;

// ovmonoflop 
byte scrpin = 7;
bool scrsperrt;
bool ocbuck;
unsigned long ovstart;
unsigned long ovact;
unsigned long ovvz = 6000; // 6 Sek // Mindestzeit, wartet ab bis buckUwert ENS unterschritten wird
bool ovmonoflop;
bool enovmono;

// Thyristor Lösch Monoflop
unsigned long scrlstart;
unsigned long scrlact;
unsigned long scrlvz = 1000; // 1 Sek Löschzeit für den Thyristor, muss kürzer als ovvz sein.
bool scrlmonoflop; // Löschmonoflop für Thyristor
bool enscrlmono;
bool scrlvorher;
bool scrlfallend;
bool scrlsteigend;

// Anzeigepins

byte boardledpin = 13;
bool test;
bool testvorher;
byte blaupin = 4;
bool blau;
bool blauvorher;
byte gruenpin = 5;
bool gruen;
bool gruenvorher;
byte rotpin = 6;
bool rot;
bool rotvorher;

// Multivibrator / Taktgeber für serielle Ausgabe
unsigned long mvstart;               
unsigned long mvist;
unsigned long mvinterval = 1000;  // Alle 1 Sek eine serielle Ausgabe
bool mv;
bool mvvorher;
bool mvpuls;
bool seriellausgeben;
byte sawert;

bool seriellausgebenflipflop;
bool seriellausgebenflipflopvorher;
bool seriellausgebenpuls;
bool seriellfreigabe;

// Abgleich
bool abgleich;
byte abgleichjumperpin = 10;
bool abgleichjumper;
unsigned long jumperstart;
unsigned long jumperist;
unsigned long jumperverz = 1000; // Einverz zur Entprellung


void setup() {
  //EEPROM.put(6, 0324); // Beim ersten hochladen aktivieren um die ID zu speichern
  Serial.begin(9600);
  int storedID;
  EEPROM.get(6, storedID);
  if (storedID != 0324) {
    while (HIGH) { // Endlosschleife, um den Sketch anzuhalten
     Serial.println("Falsche Sketch-Kennung. Beende Sketch.");
     delay(1000);
    } 
  }
  TCCR2B = TCCR2B & B11111000 | B00000001; // 31KHz  // auf D3 und D11
  //31KHz PWM an Pin11 einschalten und auf HIGH stellen
  analogWrite(11, 0);   // Buck ausschalten
  pinMode(nulldurchgangspin,INPUT);
  netz = LOW;
  pinMode(boardledpin,OUTPUT);
  pinMode(rotpin,OUTPUT);
  pinMode(gruenpin,OUTPUT);
  pinMode(blaupin,OUTPUT);
  blau = HIGH;
  digitalWrite(blaupin,blau);
  blauvorher = blau;
  gruen = LOW;
  digitalWrite(gruenpin,gruen);
  gruenvorher = gruen;
  rot = LOW;
  digitalWrite(rotpin,rot);
  rotvorher = rot;
  test = LOW;
  digitalWrite(boardledpin,test); 
  testvorher = test;
  pinMode(scrpin,INPUT_PULLUP);
  scrlmonoflop = LOW;
  scrlvorher = scrlmonoflop;
  ovmonoflop = LOW;
  buckan = LOW;
  buckanvorher = buckan;

// eeprom
  EEPROM.get(0, getmPwert);
  EEPROM.get(2, getmppwert);
  EEPROM.get(4, getpruefsummand);
  pruefzahl = getmPwert + getmppwert + getpruefsummand;
  if(pruefsumme == (getmPwert + getmppwert + getpruefsummand)) {
    mleistung = getmPwert;
    mppspg = getmppwert;
    mppwert = (mppspg * 222) / 10;
  }
  else {
    getmPwert = 266; // Vorgabe Pmax in Watt
    mleistung = getmPwert;
    getmppwert = 32; // Vorgabe der mpp Spannung in Volt
    mppspg = getmppwert;
    mppwert = (mppspg * 222) / 10;
  }
  leistungsbegrenzung();
  seriellfreigabe = LOW;
// dcdc Frequenz
  pinMode(dcdcfrequenzpin,OUTPUT);
  dcdcfrequenz = HIGH;        // Frequenz hoch
  dcdcfrequenzvorher = dcdcfrequenz;
  digitalWrite(dcdcfrequenzpin, !dcdcfrequenz);  
  // abgleich 0-Wert und Scheitelwert
  pinMode(abgleichjumperpin,INPUT_PULLUP);
  abgleich = analogRead(pvspgpin) < 60;  
// Widerstand abklemmen
  pinMode(widerstandspin,OUTPUT);
  widerstand = LOW;
  widerstandvorher = widerstand;
  digitalWrite(widerstandspin,widerstand);
// Relais einschalten
  pinMode(relpin,OUTPUT)  ;
  digitalWrite(relpin,HIGH);
  anzugvorher = anzug;
// dcdc einschalten
  pinMode(dcdcanpin,OUTPUT);
  digitalWrite(dcdcanpin,HIGH); 
  modulationsgrad = 0;
  sawert = 0;
  seriellausgeben = LOW;
  enovmono = LOW;
  enscheitelmono = LOW;
  enscrlmono = LOW;
  thfuse = HIGH;
  netzfrequenzgut = HIGH;
  delay(200);
  analogWrite(relpin,haltepwmwert);
  //analogWrite(relpin,0);
  delay(2000);
}

void amscheitelpunktlesen() { 
  tempmessen ++; 
  if(tempmessen > 3) 
   {
    tempmessen = 0;
   }
   switch(tempmessen) 
        {
          case 0:
           dcdctempwert = analogRead(dcdctemppin); 
              break;
          case 1:
           bucktempwert = analogRead(bucktemppin); 
              break; 
          case 2:
           modultempwert = analogRead(modultemppin);
              break;
          case 3:
           gehtempwert = analogRead(gehtemppin); 
              break;       
        }    
// Modulspannung von der Modultemperatur abhängig machen  
  modultempprozent = int(map(modultempwert, 127, 874, 85, 114)); // 80Celsius == 127Digits ; -30Celsius == 874Digits
  modultempprozent = constrain(modultempprozent, 85, 114); 
  mppsollwert = int(((long)mppwert * modultempprozent) / 100);
  mppunterwert = mppsollwert - mpphysteresewert;
  mppoberwert = mppsollwert + mpphysteresewert;
// Bei Übertemperatur Leistung herunterfahren
  thfuse =  (dcdctempwert < 86) || (bucktempwert < 86);  // kommt bei 95 Grad mit NTC3740 22K
  // Buck Ausgangsspannung 
  buckUwert = analogRead(buckspgpin);
// Buck Ausgangsstrom 
  buckIwert = analogRead(buckstrompin);
// Strombegrenzung bei Übertemperatur
  int kleinerertempwerta = min(dcdctempwert, bucktempwert); // je kleiner der Wert, desto höher die Temperatur
  int gehtempwertkorrigiert = gehtempwert - 136; // ab 55Grad 10K==68Dig für NTC3740 
  int kleinerertempwertb = min(gehtempwertkorrigiert, kleinerertempwerta); 
  buckIlimit = map(kleinerertempwertb, 94, 160, 475, 950); // ab 75Grad 80Grad == 127Dig   10K==68Dig für NTC3740 475==50% Strom
  long buckImaxwert_long = (mPwert / (buckUwert + 1));
  buckImaxwert = int(constrain(buckImaxwert_long, 0, 950));
  buckIlimit = constrain(buckIlimit, 0, buckImaxwert); 
  netz = (analogRead(netzoptopin) > netzwert);  
// PV-Modul Spannung  22,24/V
  pvwert = analogRead(pvspgpin);
  scrsperrt = digitalRead(scrpin) || !buckan;
// triggern des Überspannungsmonoflops   
  if((buckUwert > 588) || !netz || (!scrsperrt && !scrlmonoflop))  // 577 = 26,5V * 22,2
   {
    enovmono = HIGH; 
    ovstart = millis();
   } 
  ocbuck = buckIwert > 1021;

// Frequenzumschaltung dcdc converter  
  if(buckIwert > 250)
    {
      dcdcfrequenz = LOW;   // 25KHz
    }
  if(buckIwert < 143)
    {
      dcdcfrequenz = HIGH;    // 35KHz
    }   
// Widerstand abklemmen    
  widerstand = !netz;
// Netzfrequenz prüfen
   if((netzfrequenz > 515) || (netzfrequenz < 485)) // modulation abschalten
    {
      netzfrequenzgut = LOW;
    }
   else
    { 
      if((netzfrequenz <= 502) || (netzfrequenz >= 498))  // modulation wieder starten
       {
         netzfrequenzgut = HIGH;
       }
    }  
// automatische MPP Einstellung
  teilerwert ++; // Moduloteiler für Takt um langsam den modulationsgrad zu ändern
  if(teilerwert >= teilerzahl) // teilerzahl stellt die schnelligkeit der Runterregelung
   {
    teilerwert = 0;
    teiler = HIGH;
   }
  else
   {
    teiler = LOW; 
   }
  if(!ovmonoflop && netz && netzfrequenzgut)
   {
     if((pvwert > mppoberwert) && (buckIwert < buckIlimit)) 
      { 
        rot = HIGH;   // pvspg wird verringert 
        gruen = HIGH; // Einspeisung
        blau = LOW;
        if(teiler)  // mit jedem Takt den Modulationsgrad um eins erhöhen
         {
          modulationsgrad ++;
         }
          if(pvwert < (mppoberwert + 22)) // 1 Volt
            {
             teilerzahl = 20;
            } 
          else
            {
             teilerzahl = 4;
            } 
      }
     else if(((pvwert <= mppunterwert) || (buckIwert > (buckIlimit + 20))) && (modulationsgrad >= 1)) 
      {
        rot = LOW;
        gruen = HIGH; // Einspeisung
        blau = HIGH;  // pvspg wird erhöht
        if(teiler)  // mit jedem Takt den Modulationsgrad um eins verringern
         {
          modulationsgrad --;
         }
          if(pvwert < (mppunterwert - 22)) // 1 Volt
            {
             teilerzahl = 0;
            } 
          else
            {
              teilerzahl = 4;
            }  
      }
     else if(((pvwert <= mppunterwert) || (buckIwert > 1020)) && (modulationsgrad == 0))
      {
        rot = HIGH;
        gruen = LOW; // keine Einspeisung, modulationsgrad null
        blau = HIGH; // trotz modulationsgrad null ist die Spannung vom Solarmodul zu klein
      }      
     else  // optimaler MPP für Einspeisung erreicht
      {
        rot = LOW;
        gruen = HIGH; // Einspeisung
        blau = LOW; 
        teilerzahl = 0;   
      }   
   }
  else
   {
    modulationsgrad = 0;
    rot = LOW;
    gruen = LOW;
    blau = LOW; 
    teilerzahl = 4;
   }
  modulationsgrad = constrain(modulationsgrad, 0, 1000); // Angabe in Promille
}

void serielleAusgabe() {  // Achtung! keine zusätzlichen Werte in den Monitor Eintragen!
  sawert ++; 
  if(sawert > 12) 
   {
    sawert = 0;
   }
   if(netz)
    {     
      switch(sawert) 
        {
           case 1: // eingabewert, tnx ChatGPT :)
             static String inputString = "";
             if (Serial.available() > 0) // Überprüfe, ob Daten im seriellen Puffer verfügbar sind
              {
               char incomingChar = Serial.read(); // Lies das nächste verfügbare Zeichen
                if (incomingChar != '\n' && incomingChar != '\r')   // Wenn das eingelesene Zeichen keine Zeilenumbruchzeichen ist, füge es der Eingabezeichenkette hinzu
                 {
                  inputString += incomingChar;
                  sawert = 0; // Bleibt in case 1 bis der Wert gelesen ist
                  mvinterval = 100; //stellt den Lesetakt schneller
                 } 
                else 
                 {
                  eingabewert = inputString.toInt();
                  inputString = ""; // Eingabepuffer zurücksetzen
                  mvinterval = 1000; // stellt den Ausgabetakt wieder auf eine Sekunde
                  switch (eingabewert) // unterscheidet die Eingaben zwischen Spannung und Leistung
                     {
                      case 50 ... 300:
                         if (seriellfreigabe) {
                         putflag = HIGH;
                         mleistung = int(eingabewert); // Maximale Leistung in Watt von 50 bis 300 im Monitor eingeben
                         leistungsbegrenzung();
                         pruefsummand = int(pruefsumme - (mppspg + mleistung));
                         }
                           break;
                      case 30 ... 40: // mpp Eingabewert von 30V bis 40V
                         if (seriellfreigabe) {
                         putflag = HIGH;
                         mppwert = (eingabewert * 222) / 10; // mpp Spannung in Volt von 30 bis 40 im Monitor eingeben
                         mppspg = int(eingabewert);
                         pruefsummand = int(pruefsumme - (mppspg + mleistung));
                         }
                           break;      
                      case 1111: // serielle Ausgabe freigeben
                         seriellfreigabe = HIGH;
                           break;
                      case 1000: // serielle Ausgabe nicht freigeben
                         seriellfreigabe = LOW;
                           break;            
                     }
                 }
              }        
              break;
           case 2:  // ohne Netz wird amscheitelpunktlesen() nicht aufgerufen
            if(putflag && seriellfreigabe) {
             switch (eingabewert) // unterscheidet die Eingaben zwischen Spannung und Leistung
                     {
                      case 50 ... 300:
                         EEPROM.put(0, mleistung);
                           break;
                      case 30 ... 40: // mpp Eingabewert von 30V bis 40V
                          EEPROM.put(2, mppspg);
                           break;
                     }      
            }
            else {
             Serial.print("MK2 ");
            }
              break;     
          case 3:  // ohne Netz wird amscheitelpunktlesen() nicht aufgerufen
            if(putflag && seriellfreigabe) {
             EEPROM.put(4, pruefsummand);
             putflag = LOW;
             sawert = 0;
            }
            else {
             Serial.print(mleistung);
             Serial.print("M ");
            }
              break;
          case 4:
            gehtemp    = (240 - ((gehtempwert  - 127) * 30) / 68) / 3;   // Berechnung siehe Anhang
            Serial.print(gehtemp);  
            Serial.print("G ");
              break;
          case 5:
            dcdctemp    = (240 - ((dcdctempwert  - 127) * 30) / 68) / 3; 
            Serial.print(dcdctemp);  
            if(dcdcfrequenz == LOW)
              {
                Serial.print("d ");   // 25KHz
              }
            else 
              {
                Serial.print("D ");   // 35KHz 
              } 
              break;
           case 6:
            bucktemp    = (240 - ((bucktempwert  - 127) * 30) / 68) / 3; 
            Serial.print(bucktemp);  
            Serial.print("B ");
              break;   
          case 7:
            modultemp   = (240 - ((modultempwert - 127) * 30) / 68) / 3; 
            Serial.print(modultemp);  //pvtemp
            //Serial.print(modulationsgrad);
            Serial.print("P ");
              break;
          case 8:
            Serial.print(netzfrequenz);
            Serial.print("Hz ");
              break;    
          case 9:
            Serial.print(buckIwert);
            Serial.print("I ");
              break;
          case 10:
            sollspannung = (mppsollwert* 10) / 222;
            Serial.print(sollspannung);
            Serial.print("Vs ");
              break; 
          case 11:
            istspannung = (pvwert* 10) / 222;
            Serial.print(istspannung);
            Serial.print("Vp ");
              break;
          case 12:
            aPwert = (long)buckUwert * buckIwert;
            aktuelleleistung();
            if(aleistung >= 50) {
             Serial.print(aleistung);
             Serial.println("W");
            } 
            else {
             Serial.println(" kA");
            } 
            sawert = 0;  // damit startet er bei case 1
              break;         
        } 
      seriellausgeben = LOW;
      if(!seriellfreigabe) 
       {
        sawert = 0; // liest nur case 1 serial.available
       } 
    }  
   else
    { 
      switch(sawert) 
        {
          case 5:
            if (Serial.available() > 0) 
             {
              eingabewert = Serial.parseInt();
               switch (eingabewert) // unterscheidet die Eingaben zwischen Spannung und Leistung
                     {
                      case 50 ... 300:
                         mleistung = int(eingabewert);
                         leistungsbegrenzung();
                         pruefsummand = int(pruefsumme - (mppspg + mleistung));
                         EEPROM.put(0, mleistung); 
                         EEPROM.put(4, pruefsummand);
                           break;
                      case 30 ... 40: // mpp Eingabewert von 30V bis 40
                         mppwert = (eingabewert * 222) / 10; // Beispiel: 31V*22,2/V=688 
                         mppspg = int(eingabewert);
                         pruefsummand = int(pruefsumme - (mppspg + mleistung));
                         EEPROM.put(2, mppspg);
                         EEPROM.put(4, pruefsummand);
                           break; 
                      case 1111:
                         seriellfreigabe = HIGH;
                           break;
                      case 1000:
                         seriellfreigabe = LOW;
                           break;       
                     }  
             } 
              break;
          case 6:  // ohne Netz wird amscheitelpunktlesen() nicht aufgerufen
            Serial.print("MK2 ");
              break;  
          case 7:  // ohne Netz wird amscheitelpunktlesen() nicht aufgerufen
            Serial.print(mleistung);
            Serial.print("M ");
              break;
          case 8:  // ohne Netz wird amscheitelpunktlesen() nicht aufgerufen
            modultempwert = analogRead(modultemppin);  
              break;
          case 9:
            modultemp = (240 - ((modultempwert - 127) * 30) / 68) / 3;
            Serial.print(modultemp);
            Serial.print("P ");
              break;
          case 10:
            modultempprozent = map(modultempwert, 127, 874, 85, 114); // 80Celsius == 127Digits ; -30Celsius == 874Digits
            modultempprozent = constrain(modultempprozent, 85, 114); 
            mppsollwert = ((mppwert / 4) * modultempprozent) / 25;
            sollspannung = (mppsollwert* 10) / 222;
            Serial.print(sollspannung);
            Serial.print("Vs ");
              break; 
          case 11:
            istspannung = (pvwert* 10) / 222;
            //Serial.print(istspannung);
            Serial.print(istspannung);
            Serial.print("Vp ");
              break;
          case 12:
            if(istspannung > 22)  
              { 
                Serial.println("kein Netz");
              }
            else  
              {
                Serial.println("USB");
              }
            sawert = 3;  // damit startet er bei case 4
              break;
        }     
      seriellausgeben = LOW;
      if(!seriellfreigabe) 
       {
        sawert = 4; // liest nur case 5 serial.available
       }
    }
}

void loop() {
  nulldurchgang = digitalRead(nulldurchgangspin);
  nulldurchgangspuls = nulldurchgang && !nulldurchgangvorher; // steigende Flanke
  nulldurchgangvorher = nulldurchgang;
  if(nulldurchgangspuls)
    {
      nulldurchgangsabstand = micros() - startzeit;
      enscheitelmono = HIGH;
// Netzfrequenz Ermittlung
      netzfrequenz = 495500 / (nulldurchgangsabstand / 10); // Kalibrierung der Frequenzmessung
      startzeit = micros();
    }
  
  act = micros()-startzeit - flankenkorrektur; 
  
  // Scheitelpunkt
  scheitelmonoflop = (act >= 0) && (act <= 4650) && enscheitelmono; // 4650ms = 1/4 Periodendauer - 1/2 Amscheitelpunktlesendauer (700us)
  nulldurchgangsflipflop = (act >= 9500) && (act <= 10500);  // Zeitfenster für Nulldurchgang
  
    test = scheitelmonoflop && seriellfreigabe;  // Die Boardled wird im Monitor durch 1111 aktiviert und 1000 deaktiviert
  if(test != testvorher)
    {
      digitalWrite(boardledpin, test);
      testvorher = test;
    } 

  if(nulldurchgangsflipflop && netz)
    {
      return;
    }  
     
// Scheitelpunkt bei fallender Flanke des Scheitel-Monoflops
  scheitelpunkt = !scheitelmonoflop && scheitelmonoflopvorher; // fallende Flanke
  scheitelmonoflopvorher = scheitelmonoflop;    

  if(scheitelpunkt)
  {
    amscheitelpunktlesen();
  }

// ovmonoflop retriggerbar
  ovact = millis() - ovstart;
  ovmonoflop = (ovact >= 0) && (ovact <= ovvz) && enovmono;

 //buckan    
  buckanflipflop = (act >= 1000) && (act <= 9000);
  buckan = (buckanflipflop && netz && !ovmonoflop && !ocbuck && !thfuse) || (abgleich && !netz);
  buckanfallend = buckanvorher && !buckan;
  buckansteigend = !buckanvorher && buckan;
  buckanvorher = buckan;
 
// Thyristor löschen 
  scrlact = millis() - scrlstart;  
  scrlmonoflop = (scrlact >= 0) && (scrlact <= scrlvz) && enscrlmono; 
  scrlfallend = scrlvorher && !scrlmonoflop;
  scrlsteigend = !scrlvorher && scrlmonoflop;
  scrlvorher = scrlmonoflop;  
  if(scrlsteigend || buckanfallend) 
    {
      pinMode(scrpin,OUTPUT);       // für den Löschvorgang wind der Pin als Ausgang definiert
      digitalWrite(scrpin,LOW);     // und auf 0V gesetzt
    }
  if(scrlfallend || buckansteigend) 
    {
      pinMode(scrpin,INPUT_PULLUP); // danach wieder als Eingang definiert
    }  
    
 
// Modulation
  if((!netz || ovmonoflop || ocbuck || thfuse) && !abgleich)
   {
    modulationsgrad = 0;
    rot = HIGH;
    gruen = LOW;
    blau = LOW;
    dcdcfrequenz = HIGH;
    modultempwert = analogRead(modultemppin);
// Thermosicherung kontrollieren
    dcdctempwert = analogRead(dcdctemppin);
    thfuse =  dcdctempwert < 86;  // kommt bei 95 Grad mit NTC3740 22K
// Begrenzung der Leerlaufspannung mit Bremschopper    
    pvwert = analogRead(pvspgpin); 
     if(pvwert > 1023)  // 46V  Leerlaufoberwert = Leerlaufoberspannung * 22,2
      {
        widerstand = HIGH;  // Widerstand anklemmen
      }
     if(pvwert < 976)  // 44V  Leerlaufunterwert = Leerlaufunterspannung * 22,2
      {
        widerstand = LOW;   // Widerstand abklemmen
      }
// ENS      
    buckUwert = analogRead(buckspgpin);
     if(buckUwert > 67)  // 3V * 22,2
      {
        enovmono = HIGH;
        ovstart = millis(); // ovmonoflop wird gesetzt bis buckUwert < 3V   
      } 
   }

// Relais PWM-Steuerung
  anzug = !ovmonoflop;
  anzugflankesteigend = anzug && !anzugvorher;
  anzugflankefallend = !anzug && anzugvorher;
  anzugvorher = anzug;
  if(!anzugflankesteigend)
    {
      anzugflankeist = millis();
      if(anzugflankeist - anzugflankestart > 200) // 200ms starker Anzug
        {
          relpwm = HIGH;
        }
    }
  else
    {
      anzugflankestart = millis();
      relpwm = LOW;
    } 
  relpwmflanke = (relpwmvorher != relpwm) && anzug;
  relpwmvorher = relpwm;  
  if(anzugflankesteigend)
      {
        digitalWrite(relpin,HIGH);
        digitalWrite(dcdcanpin,HIGH);
      }
  else if(relpwmflanke)  
      {
        analogWrite(relpin, haltepwmwert);
      }   
  else if(anzugflankefallend)
      {
        digitalWrite(relpin,LOW);
        digitalWrite(dcdcanpin,LOW);
      } 
      
  else if(!abgleich && netz && !ovmonoflop && !ocbuck && !thfuse)
   {
    modulation();
    modulationswert = int(((long)modulationsgrad * modulationswert) / 1000); // Modulationsgrad in Promille, keine Modulation bei 0V
    //modulationswert = int(modulationswert / (1001 / (modulationsgrad + 1)));
    if(modulationswert != modulationswertvorher)
      {
       analogWrite(11, modulationswert);
       modulationswertvorher = modulationswert;
      } 
   }

// Abgleich Modulationsbereich Buck (ohne Spannung am Solarmoduleingang!)
// Und Test der Frequenzumschaltung des dcdc-converters
// mit Labornetzteil 24V an den Abgleichpunkt legen. 
  else if(abgleich && !ovmonoflop && !ocbuck /* && !thfuse*/)
   {
    abgleichjumper = !digitalRead(abgleichjumperpin);
// Testbetrieb mit Jumper, dieser soll im Normalbetrieb entfernt werden.    
    if(abgleichjumper)  // Einschaltverzögerung zur Entprellung
     {
      jumperist = millis();
      if(jumperist - jumperstart > jumperverz)
      { 
      dcdcfrequenz = LOW;  // 25KHz
      if(netz)
       {
        modulation();
        modulationswert = (modulationswert / 2); // Hälfte weil keine Stromgegenkopplung beim Test vorhanden
        if(modulationswert != modulationswertvorher)
         {
          analogWrite(11, modulationswert);
          modulationswertvorher = modulationswert;
         }
       }
// Testbetrieb mit Jumper, Lowside aus, Highside an     
      else 
       {
        analogWrite(11, 255); 
       }      
      }
     }
// Testbetrieb ohne Jumper
    else                        
     {
      jumperstart =millis();
      analogWrite(11, 0); //  Lowsidegate an, Highsidegate aus
      dcdcfrequenz = HIGH;   // 35KHz
     }
   }
  else 
   {
    analogWrite(11, 0);       
   }
  // Thyristor setzt ovmonoflop und danach Löschmonoflop
  scrsperrt = digitalRead(scrpin) || !buckan; 
    if(!scrsperrt && !scrlmonoflop) 
      {
       enovmono = HIGH; 
       ovstart = millis();
       enscrlmono = HIGH;
       scrlstart = millis(); 
      }  

  if(dcdcfrequenz != dcdcfrequenzvorher)
    {   
      digitalWrite(dcdcfrequenzpin, !dcdcfrequenz);
      dcdcfrequenzvorher = dcdcfrequenz; 
    } 
  if(blauvorher != blau) 
    {
      digitalWrite(blaupin,blau);
      blauvorher = blau;
    }
  if(rotvorher != rot)  
    {
      digitalWrite(rotpin,rot);
      rotvorher = rot;
    }
  if(gruenvorher != gruen)
    {  
  digitalWrite(gruenpin,gruen);
  gruenvorher = gruen;
    }
  if(widerstandvorher != widerstand)
    {  
  digitalWrite(widerstandspin,widerstand); 
  widerstandvorher = widerstand;
    }

   // Multivibrator Taktgeber
  mvist = millis();                                    // Multivibrator für serielle Ein- und Ausgabe
  if(mvist - mvstart >= mvinterval)
     {
       mvstart = mvist;
       if(mv == LOW)
         {
          mv = HIGH;
         }
       else 
         {
          mv = LOW;
         }
     }                                                 // Multivibrator Ende 
  mvpuls = (mv != mvvorher);                           // Multivibratorflanken
  mvvorher = mv;

    if(mvpuls == HIGH)
     {
      seriellausgeben = HIGH;
     }
  seriellausgebenflipflop =  (act >= 0) && (act <= 100); // serielle Ausgabe kurz nach Nulldurchgang
  seriellausgebenpuls = !seriellausgebenflipflop && seriellausgebenflipflopvorher;
  seriellausgebenflipflopvorher = seriellausgebenflipflop;  
    if(((seriellausgebenpuls && seriellausgeben) || (!netz && seriellausgeben)) && ((pvwert > 49) || abgleich))
     {
       serielleAusgabe(); // dauert etwa 200µs pro Block
     }   
}

void modulation()
 {
  mod = act + lztp;
  mod = constrain(mod, 0, 10000);
  if(mod > 5000) {       // spiegelt die Kurve
    mod = 10000 - mod;
  }  
  switch (mod)
  {
    case 0 ... 1100:     // Lücke um den Nulldurchgang
      modulationswert = 0;
        break;
    case 1101 ... 1500:  // Anstieg zu bzw. Abfall von der Halbwelle
      modulationswert = map(mod, 1100, 1500, 0, 128);
        break; 
    case 1501 ... 2100:  // Sinus
      modulationswert = map(mod, 1500, 2100, 128, 173);
        break;
    case 2101 ... 2600:  // Sinus
      modulationswert = map(mod, 2100, 2600, 173, 205);
        break;
    case 2601 ... 3100:  // Sinus
      modulationswert = map(mod, 2600, 3100, 205, 233);
        break; 
    case 3101 ... 3600:  // Sinus
      modulationswert = map(mod, 3100, 3600, 233, 255);
        break;             
    case 3601 ... 5000:  // Scheitelplateau
      modulationswert = 255;
        break;
  } 
 }   
void leistungsbegrenzung() 
 {
  switch (mleistung)
   {
    case 50 ... 100:
      mPwert = map(mleistung, 50,  100,  90000, 165000);
        break;
    case 101 ... 150:
      mPwert = map(mleistung, 100, 150, 165000, 243000);
        break; 
     case 151 ... 200:
      mPwert = map(mleistung, 150, 200, 243000, 319000);
        break;  
      case 201 ... 250:
      mPwert = map(mleistung, 200, 250, 319000, 399000);
        break; 
      case 251 ... 300:
      mPwert = map(mleistung, 250, 300, 399000, 475000); 
        break;        
   }
 }  
 void aktuelleleistung() 
 {
  switch (aPwert)
   {
    case 0 ... 89000:
      aleistung = 0;
        break; 
    case 90000 ... 165000:
      aleistung = map(aPwert, 90000, 165000, 50, 100);
        break;
    case 165001 ... 243000:
      aleistung = map(aPwert, 165000, 243000, 100, 150);
        break; 
    case 243001 ... 319000:
      aleistung = map(aPwert, 243000, 319000, 150, 200);
        break;  
    case 319001 ... 399000:
      aleistung = map(aPwert, 319000, 399000, 200, 250);
        break; 
    case 399001 ... 480000:
      aleistung = map(aPwert, 399000, 475000, 250, 300); 
        break;        
   }
 }                                                                     
 

 // Übertrager DCDC ETD44 3C94 6Wdg:85Wdg primär 8*0.7mm CuL. parallel
 // Drossel Buck  ETD44 3F3 2mm Spalt also 1mm Hartpapier 14Wdg 8*0.7mm CuL. parallel ergibt 30uH 
 
 // rot == kein Netz oder Sicherheitsabschaltung
 // grün == Einspeisung im MPP
 // grün + rot == Einspeisung, Regelung zum MPP von oben
 // grün + blau == Einspeisung, Regelung zum MPP von unten
 // blau == 12V da, arduino im Reset Solarspannung zu klein
 // blau + rot == Netz da, arduino arbeitet, Solarspannung zu klein

 // Widerstand des NTC für 80Celsius und -30Celsius berechnen lassen: 
 // Für NTC 22K B=3740 analoge Linearisierung mit  Rv=22K Rp=220K
 // https://www.electronicdeveloper.de/MesstechnikNTCTR.aspx
 // 3180R @ 80Grad == 127 Digits     
 // 312323R @ -30Grad == 874 Digits 
 // 874Dig -127Dig = 747Digits
 // 80C - -30C = 110Kelvin
 // 747Dig / 110Kelvin = 6,8 Digits/Kelvin
 // um die volle integer-Auflösung zu nutzen werden die Digits mit 30 multipliziert
 // die höchste vorkommende Zahl ist dann 30 * (1023 - 127) = 26880 von möglichen 32767
 // die 80C mit 3 was 240C ergibt und die 6,8 Digits/Kelvin mit 10 was 68 ergibt
 // temp = (240 - ((tempwert - 127) * 30) / 68) / 3;

 // modultempprozent ist die relative Abweichung der optimalen Modulspannung in Abhängigkeit zur Modultemperatur
 // 80Celsius == 127Digits ; -30Celsius == 874Digits ; für 80C == 85%; für -30C == 114% laut Datenblatt des Solar-Moduls 
 // modultempprozent = map(modultempwert, 127, 874, 85, 114); // 80Celsius == 127Digits ; -30Celsius == 874Digits 

