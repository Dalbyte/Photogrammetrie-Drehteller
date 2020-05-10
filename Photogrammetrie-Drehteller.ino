#include <Adafruit_NeoPixel.h>

#define PINledring    14 // Wemos-D1-mini D5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12
int NEOPIXEL = 12;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PINledring, NEO_GRB + NEO_KHZ800);

// PinOUT
int PINphototrigger = 12; // Wemos-D1-mini D6
int PINphototriggerStatus=0; // Wemos-D1-mini D6 Status

int ButtonZurueck = 15; // Wemos-D1-mini D8
int ButtonZurueckStatus = 0; // Wemos-D1-mini D8 Status
int ButtonVorwaerts = 5; // Wemos-D1-mini D1
int ButtonVorwaertsStatus = 0; // Wemos-D1-mini D1 Status

int PINphototriggerRelay= 13; // Wemos-D1-mini D7
int StepEnable = 2; // Enable Wemos-D1-mini D4
int StepStep = 0; // Step Wemos-D1-mini D3
int StepDir = 4 ; // Richtung Wemos-D1-mini D2



int delayval = 20; // delay for half a second


////////////////////////////////////////////////////////////////////////////////
// Settings

int motorStep = 3200; // 16*200 (1,8° Steps kann der Motor)
int motorStepCount = 0; // Aktuelle Schritt Position
int speeddelay = 0; // Pause zwischen den Schritten
int StepToPixel = 0; // Schritt pro Led

int motorStepPointA = 0; // Hilft bei Halbenschritten.
int motorStepPointB = 0; // Hilft bei Halbenschritten.

int photo = 20; // Wie viel Fotos für 360°
int photoCount = 0; // Wie viel Fotos geknippst wurden.
int delayphoto = 1000; // Für die Kamera eine Pause.

////////////////////////////////////////////////////////////////////////////////
// Bildeinstellung
unsigned long time;
unsigned long lasttime;
int resettime = 5000;
bool settingsativ = false;

void setup(){
    Serial.begin(115200);

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

}



////////////////////////////////////////////////////////////////////////////////
// LOOP
void loop(){
  
  Button();
  Photoeinstellung();
  
  
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
  delay(160); // Ausschwingen
  madePhoto();
  delay(delayphoto); // Kamera Cache => SD-Karte


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
      photo += 1;
      Serial.println(photo);
      settingsativ = true;
      lasttime = millis();
      ledPhoto();
      delay(180);
  }

}

void ButtonZu(){
  ButtonZurueckStatus=digitalRead(ButtonZurueck);

    if (ButtonZurueckStatus == HIGH){
      Serial.println("Zu");
      photo -= 1;
      Serial.println(photo);
      settingsativ = true;
      lasttime = millis();
      ledPhoto();
      delay(180);
  }

}

////////////////////////////////////////////////////////////////////////////////
// Photoeinstellung

void Photoeinstellung(){
  ButtonVor();
  ButtonZu();
  zeit();
}

void madePhoto(){
    digitalWrite(PINphototriggerRelay, HIGH);
    delay(160); // Auslöser brauch Delay
    digitalWrite(PINphototriggerRelay, LOW);
}

void zeit(){

  if(settingsativ){
    time = millis(); // Check Time
    int timedif = time-lasttime;
    Serial.println(lasttime);
    Serial.println(time);
    Serial.println(timedif);
    if (timedif > resettime) {
        pixelred();
        settingsativ = false;
    }

  }

}

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