#include <Wire.h>
#include <SPI.h>
#include <I2C_graphical_LCD_display.h>
#define countof(a) (sizeof(a) / sizeof(a[0]))
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

I2C_graphical_LCD_display lcd;
#define ENC_COUNT_REV 400
 
// Encoder output to Arduino Interrupt pin
#define ENC_IN 2
#define ENC_IN2 3
#define irPin 8

 
int count=0;  
boolean state = true;

String temp_str;
// Pulse count from encoder
volatile long encoderValue = 0;
volatile long encoderValue1 = 0;
 
// One-second interval for measurements
int interval = 1000;
 
// Counters for milliseconds during interval
long previousMillis = 0;
long currentMillis = 0;

long DpreviousMillis = 0;
long DcurrentMillis = 0;
// Variable for RPM measuerment
float rpm = 0;
float rpm2 = 0;
int Dinterval = 100;
int items = 0;
float speeed = 0;
char crpm[6];
char cspeed[6];
char citems[6];


//memory constants
int memoryCount = 0;

const uint8_t SettingsAddress = 0;
bool wasError(const char* errorTopic = "")
{
    uint8_t error = Rtc.LastError();
    if (error != 0)
    {
        // we have a communications error
        // see https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
        // for what the number means
        Serial.print("[");
        Serial.print(errorTopic);
        Serial.print("] WIRE communications error (");
        Serial.print(error);
        Serial.print(") : ");

        switch (error)
        {
        case Rtc_Wire_Error_None:
            Serial.println("(none?!)");
            break;
        case Rtc_Wire_Error_TxBufferOverflow:
            Serial.println("transmit buffer overflow");
            break;
        case Rtc_Wire_Error_NoAddressableDevice:
            Serial.println("no device responded");
            break;
        case Rtc_Wire_Error_UnsupportedRequest:
            Serial.println("device doesn't support request");
            break;
        case Rtc_Wire_Error_Unspecific:
            Serial.println("unspecified error");
            break;
        case Rtc_Wire_Error_CommunicationTimeout:
            Serial.println("communications timed out");
            break;
        }
        return true;
    }
    return false;
}

void setup()
{
  // Setup Serial Monitor
  Rtc.Begin();
  initDisplay();
  #if defined(WIRE_HAS_TIMEOUT)
    Wire.setWireTimeout(3000 /* us */, true /* reset_on_timeout */);
  #endif        
  // Set encoder as input with internal pullup  
  pinMode(ENC_IN, INPUT_PULLUP); 
  pinMode(ENC_IN2, INPUT_PULLUP); 
  pinMode(irPin, INPUT_PULLUP); 
  // Attach interrupt 
  attachInterrupt(digitalPinToInterrupt(ENC_IN), updateEncoder,RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_IN2), updateEncoder1,RISING);
  
  // Setup initial values for timer
  previousMillis = millis();
  DpreviousMillis = millis();
  
  count = readMemory();
  pinMode(7, INPUT_PULLUP);
  
}
 
void loop()
{ 
  int sensorVal = digitalRead(7);
  if(sensorVal == LOW){
     eraseEeprom();
     count = 0;
  }
  countObject();
  writeMemory(count);
  
  currentMillis = millis();
  DcurrentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
 
 
    calculations();
    updateDisplay();
    
    
    encoderValue = 0;
    encoderValue1 = 0;
  }
}
 
void updateEncoder()
{
  // Increment value for each pulse from encoder
  encoderValue++;
}
void updateEncoder1()
{
  // Increment value for each pulse from encoder
  encoderValue1++;
}
void updateDisplay(){
if (rpm >= 0) {
       if (DcurrentMillis - DpreviousMillis > Dinterval) {
        DpreviousMillis = DcurrentMillis;
      
       lcd.clear(95,20,120,63,0);
       }
       
      lcd.gotoxy(95,20);
      lcd.string(crpm,false);
      lcd.gotoxy(95,35);
      lcd.string(cspeed,false);
      lcd.gotoxy(95,50);
      lcd.string(citems,false);
      
      
      /*Serial.print(crpm);
      Serial.print(" PULSES: ");
      Serial.print(encoderValue);
      Serial.print('\t');
      Serial.print(" SPEED: ");
      Serial.print(rpm);
      Serial.println(" RPM");
      */
    }
}
void initDisplay(){
Serial.begin(9600); 
  lcd.begin (0x20); 
  TWBR = 20;
  /*lcd.string (" RPM and ITEM COUNTER", true);
  lcd.gotoxy(0,20);
  lcd.string ("MOTOR RPM    :", false);
  lcd.gotoxy(0,35);
  lcd.string ("LINEAR SPEED :", false);
  lcd.gotoxy(0,50);
  lcd.string ("ITEM COUNT   :", false);
  */ //working code
  lcd.gotoxy(0,0);
  lcd.clear(0,0,127,0,1);
  lcd.gotoxy(0,4);
  lcd.string ("    MICROLOGSPRAY    ", true);
  lcd.gotoxy(0,20);
  lcd.string ("SLIDER-str/min:", false);
  lcd.gotoxy(0,38);
  lcd.string ("CONVEYOR-m/min:", false);
  lcd.gotoxy(0,48);
  lcd.string ("SKIN COUNT nos:", false); //NEW code has to be tested with padding 
}
void calculations(){
    rpm = (float)(encoderValue * 60 / ENC_COUNT_REV);
    rpm2 = (float)(encoderValue1 * 60 / ENC_COUNT_REV);
    speeed = (float)0.1047*0.045*rpm2;
    temp_str = String(rpm);
    temp_str.toCharArray(crpm,6); 
    temp_str = String(speeed);
    temp_str.toCharArray(cspeed,6);

    temp_str = String(count);
    temp_str.toCharArray(citems,6);
}
int readMemory(){
 int retrievedCount = 0;// init to zero

        // get our data from the address with the given size
        uint8_t gotten = Rtc.GetMemory(SettingsAddress,
            reinterpret_cast<uint8_t*>(&retrievedCount),
            sizeof(retrievedCount));
        if (!wasError("loop getMemory settings"))
        {
            Serial.print("data read (");
            Serial.print(gotten);
            Serial.println(")");
            Serial.print("    value1 = ");
            Serial.print(retrievedCount);
            Serial.println();
        }
  return retrievedCount;
    
}
void writeMemory(int Count){
 uint8_t written = Rtc.SetMemory(SettingsAddress,
            reinterpret_cast<const uint8_t*>(&Count),
            sizeof(Count));
        wasError("setup setMemory settings");
}
void eraseEeprom(){
  for(int i = 0; i<53;i++){
 Rtc.SetMemory(i, 0);
  }
}
void countObject(){
  if(count >= 9999){
    count = 0;
  }
  else{
  if (!digitalRead(irPin) && state){  
    count++;  
    state = false;  
    Serial.print("Count: ");  
    Serial.println(count);  
    
    delay(1);  
  }  
  if (digitalRead(irPin))  
  {  
    state = true;  
    delay(1);  
  }  
  }
}

