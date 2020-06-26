#include <Adafruit_NeoPixel.h>
// Für Infarot Sender
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// WEB

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>


/*SSID & Password*/
const char* ssid = "scanteller";  // Enter SSID here
const char* password = "";  //Enter Password here

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

/* IP Address*/
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);

bool WebTriggerBool = false;

String WebParaName;

// Infarot Ende Setup

#define PINledring    0 // Wemos-D1-mini D5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
int NEOPIXEL = 12;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PINledring, NEO_GRB + NEO_KHZ800);

// PinOut Prüfen je nach Umsetzung unterschiedlich
// PinOUT
int PINphototrigger = 13; // Wemos-D1-mini D6
int PINphototriggerStatus=0; // Wemos-D1-mini D6 Status

int ButtonZurueck = 12; // Wemos-D1-mini D8
int ButtonZurueckStatus = 0; // Wemos-D1-mini D8 Status
int ButtonVorwaerts = 15; // Wemos-D1-mini D1
int ButtonVorwaertsStatus = 0; // Wemos-D1-mini D1 Status

int PINphototriggerRelay= 14; // Wemos-D1-mini D7
int StepEnable = 16; // Enable Wemos-D1-mini D4
int StepStep = 5; // Step Wemos-D1-mini D3
int StepDir = 2 ; // Richtung Wemos-D1-mini D2



int delayval = 20; // delay for half a second


////////////////////////////////////////////////////////////////////////////////
// Settings

int motorStep = 9600; // 16(SubStep)*200(RealSteps)=3200 (1,8° Steps kann der Motor), 9600 Zahnrad
int motorStepCount = 0; // Aktuelle Schritt Position
int speeddelay = 0; // Pause zwischen den Schritten
int StepToPixel = 0; // Schritt pro Led

int motorStepPointA = 0; // Hilft bei Halbenschritten.
int motorStepPointB = 0; // Hilft bei Halbenschritten.

int photo = 20; // Wie viel Fotos für 360°
int photoCount = 0; // Wie viel Fotos geknippst wurden.
int delayphoto = 3000; // Für die Kamera eine Pause.

////////////////////////////////////////////////////////////////////////////////
// Bildeinstellung

unsigned long timer;
unsigned long lasttime;
int resettime = 5000;
bool settingsativ = false;

///////////////////////////////////////////////////////////////////////////////

void setup(){
  irsend.begin();
  #if ESP8266
    Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  #else  // ESP8266
    Serial.begin(115200, SERIAL_8N1);
  #endif  // ESP8266
    

    // Pinlayout Def.
    pinMode(PINphototrigger, INPUT);
    pinMode(ButtonZurueck , INPUT);
    pinMode(ButtonVorwaerts , INPUT);

    pinMode(PINphototriggerRelay, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(StepEnable, OUTPUT); // Enable
    pinMode(StepStep, OUTPUT); // Step
    pinMode(StepDir, OUTPUT); // Richtung

    
    pixels.begin(); // This initializes the NeoPixel library.

    // Motor Start
    digitalWrite(StepEnable,HIGH);

    setStepVariable();
    pixelred();

    WebSetup();

}



////////////////////////////////////////////////////////////////////////////////
// LOOP

void loop(){
  
  Button();
  Photoeinstellung();
  WebTrigger();
  
  
}

////////////////////////////////////////////////////////////////////////////////
// MotorLoop

void ledringstep(int stepPosition){

  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,255,168)); // set color.
      pixels.show(); // This sends the updated pixel color to the hardware.
      delay(delayval); // Delay for a period of time (in milliseconds).
    }


}


void setStepVariable(){

  float x = motorStep/photo;
  motorStepPointA = x;
  motorStepPointB = x + 0.5;
  // Serial.println(motorStepPointA);
  // Serial.println(motorStepPointB);

  StepToPixel = motorStep/NEOPIXEL;
}


////////////////////////////////////////////////////////////////////////////////
// LED-Status

void ledringrest(){
  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0)); // set color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
}

void pixelgreen(){
  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,255,40)); // set color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
}

void pixelred(){
  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(255,0,0)); // set color.
      pixels.show(); // This sends the updated pixel color to the hardware.
    }
}




////////////////////////////////////////////////////////////////////////////////
// Steppmotor setzt Rotationsposition

void stepMotor(){

  digitalWrite(StepDir,HIGH); // im Uhrzeigersinn
  digitalWrite(StepStep,HIGH);
  delayMicroseconds(500);
  digitalWrite(StepStep,LOW);
  delayMicroseconds(500);

}

void ledbar(int stepinloop){

  int steprotation = motorStepCount + stepinloop;
  float pixr = (float) steprotation / (float) StepToPixel;
  Serial.print("pixr");
  Serial.println(pixr);
  
  int pix = pixr;
  pixr = (float) pixr - (float) pix;
  int color = 255 * pixr;

  for(int i=0;i<pix;i++){
      pixels.setPixelColor(i, pixels.Color(0,255,40));
    }

  pixels.setPixelColor(pix, pixels.Color(color,color,0));
  pixels.show();

}

void stepSchleife(){

  digitalWrite(StepEnable,LOW);
  setStepVariable();
  pixelred();

 while(photoCount<photo){
    if (photoCount < photo-1){
      Serial.println("PunktA");
      goStepToPoint(motorStepPointA);
      motorStepCount += motorStepPointA;
      photoCount += 1;
    }
    if (photoCount < photo-1){
      Serial.println("PunktB");
      goStepToPoint(motorStepPointB);
      motorStepCount += motorStepPointB;
      photoCount += 1;
    }
    if (photoCount == photo-1){
      Serial.println("PunktEnde");
      int n = motorStep-motorStepCount;
      Serial.println(n);
      Serial.println(motorStepCount);
      goStepToPoint(n);
      photoCount += 1;
    }
  }

  // Final Animation
    delay(120);
    ledringrest();
    delay(60);
    pixelgreen();
    delay(60);
    ledringrest();
    delay(60);
    pixelgreen();
    delay(120);
    pixelred();

  digitalWrite(StepEnable,HIGH);
  // Reset
  photoCount = 0;
  motorStepCount = 0;

}

void goStepToPoint(int x){
  // Geht Schritt Anzahl for
  for(int i=0;i<=x;i++){
    ledbar(i);
    delay(speeddelay);
    stepMotor(); 
  }
  delay(delayphoto); // Ausschwingen / Kamera Cache => SD-Karte
  madePhoto();
  delay(1000); // Auslösen (Langzeitbelichtung ist nicht vorgesehen)

}


////////////////////////////////////////////////////////////////////////////////
// Button

void Button(){
  PINphototriggerStatus=digitalRead(PINphototrigger);

    if (PINphototriggerStatus == HIGH){
      
      // Feedback Taste ist gedrückt
      Serial.print("Tast01");
      digitalWrite(LED_BUILTIN, LOW); // Status Led Wemos-D1-mini (invert) 
      
      // Trigger Funktion
      // stepMotor();
      stepSchleife();  
  }else{
      // Feedback Reset
      digitalWrite(LED_BUILTIN, HIGH); // Status Led Wemos-D1-mini (invert)
  
    }  

}

void ButtonVor(){
  ButtonVorwaertsStatus=digitalRead(ButtonVorwaerts);

    if (ButtonVorwaertsStatus == HIGH){
      Serial.println("Vor");
      photo += 1; // Plus ein Bild
      ButtonVorZu(); // Status + Ledring + Block
  }

}

void ButtonZu(){
  ButtonZurueckStatus=digitalRead(ButtonZurueck);

    if (ButtonZurueckStatus == HIGH){
      Serial.println("Zu");
      photo -= 1; // Minus ein Bild
      ButtonVorZu(); // Status + Ledring + Block
  }

}

void ButtonVorZu(){
      Serial.println(photo);
      settingsativ = true; // setzt Status damit man in Settingsmodus bleibt.
      lasttime = millis(); // inaktivität Marker um Interface in Normal Modus zu setzen.
      ledPhoto();
      delay(180); // Doppelte Eingabe wird verhintert
}

////////////////////////////////////////////////////////////////////////////////
// Photoeinstellung

void Photoeinstellung(){
  ButtonVor();
  ButtonZu();
  zeit();
}

void madePhoto(){
    madePhotoCanon();
    madePhotoSony();
}

void madePhotoCanon(){
  // Optokoppler wird hier geschaltet. Methode funktioniert auch bei Nikon Pinlayout beachten.
    digitalWrite(PINphototriggerRelay, HIGH);
    delay(160); // Auslöser brauch Delay
    digitalWrite(PINphototriggerRelay, LOW);
}

void madePhotoSony(){
  // Es muss 3 mal gesendet werden das Signal
  for (int i=0; i<3; i++){
  irsend.sendSony(740239, 20);
  delay(40);
  }  
}

void zeit(){

  if(settingsativ){
    timer = millis(); // Check Time
    int timedif = timer-lasttime;
    Serial.println(lasttime);
    Serial.println(timer);
    Serial.println(timedif);
    if (timedif > resettime) {
        pixelred();
        settingsativ = false;
    }

  }

}

// LedSettings
void ledPhoto(){

  for(int i=0;i<NUMPIXELS;i++){
      pixels.setPixelColor(i, pixels.Color(0,0,0));
  }

  int r = photo / NEOPIXEL;
  int re = photo - (r*NEOPIXEL);

  if(re == 0 && r > 0){
    r -= 1;
    re = NEOPIXEL;
  }

  for(int i=0;i<re;i++){

      pixels.setPixelColor(i, pixels.Color(0,0,120));

  }

  for(int i=0;i<r;i++){

    if (re >= i+1) {
      pixels.setPixelColor(i, pixels.Color(120,0,120));
    }else
    {
      pixels.setPixelColor(i, pixels.Color(120,0,0));
    }
    
  }
  pixels.show();

}

////////////////////////////////////////////////////////////////////////////////
//// WEB

void WebSetup(){
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  if(!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount failed");
    while(true) yield(); //Stop here
  }

  // Sendet die Start Webseite aus den SPIFFS, musst AsyncMethode nehmen weil wegen SoftAcessesPoint irgend was nicht funktioniert hat mit SPIFFS.
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String());
  });

  // Title Bild
    server.on("/pixelart.gif", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/pixelart.gif", String());
  });

  // Font
      server.on("/PressStart2P-Regular.ttf", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/PressStart2P-Regular.ttf", String());
  });

    // Webrequest setzt nur eine Variable die durchen den Normalen Loop dann ausgelöst wird.
    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/index");
    WebTriggerBool = true;
  });


    // Webrequest setzt nur eine Variable die durchen den Normalen Loop dann ausgelöst wird.
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request){
    
    int paramsNr = request->params();
    Serial.println(paramsNr);

    for(int i=0;i<paramsNr;i++){

        AsyncWebParameter* p = request->getParam(i);

        // Schickt alle Parameter zurück
        Serial.print("Name: ");
        Serial.println(p->name());
        Serial.print("Value: ");
        Serial.println(p->value());
        Serial.println("---Nächster Parameter/Ende---");

        WebParaName = p->name();

        if(WebParaName == "frame"){
          // Serial.println("frame");
          photo = p->value().toInt();
          ButtonVorZu();
        }

        if(WebParaName == "delay"){
          // Serial.println("delay");
          delayphoto = p->value().toInt()*1000;
        }
    

    }
    request->redirect("/index");
  });
  

  server.begin();
  Serial.println("HTTP server started");
}

// Hat jemand über das Webinterface Start gedrückt?
void WebTrigger(){
if(WebTriggerBool){
    stepSchleife();
    WebTriggerBool = false;
  }
};
