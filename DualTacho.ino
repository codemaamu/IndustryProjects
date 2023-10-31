#include <Wire.h>
#include <SPI.h>
#include <I2C_graphical_LCD_display.h>
#define countof(a) (sizeof(a) / sizeof(a[0]))
#include <RtcDS1307.h>
RtcDS1307<TwoWire> Rtc(Wire);

I2C_graphical_LCD_display lcd;
//#define ENC_COUNT_REV 400
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const byte PulsesPerRevolution = 2;  // Set how many pulses there are on each revolution. Default: 2.


// If the period between pulses is too high, or even if the pulses stopped, then we would get stuck showing the
// last value instead of a 0. Because of this we are going to set a limit for the maximum period allowed.
// If the period is above this value, the RPM will show as 0.
// The higher the set value, the longer lag/delay will have to sense that pulses stopped, but it will allow readings
// at very low RPM.
// Setting a low value is going to allow the detection of stop situations faster, but it will prevent having low RPM readings.
// The unit is in microseconds.
const unsigned long ZeroTimeout = 5000000;  // For high response time, a good value would be 100000.
                                           // For reading very low RPM, a good value would be 300000.


// Calibration for smoothing RPM:
const byte numReadings = 4;  // Number of samples for smoothing. The higher, the more smoothing, but it's going to
                             // react slower to changes. 1 = no smoothing. Default: 2.
const byte numReadings1 = 4;




/////////////
// Variables:
/////////////

volatile unsigned long LastTimeWeMeasured;  // Stores the last time we measured a pulse so we can calculate the period.
volatile unsigned long PeriodBetweenPulses = ZeroTimeout+1000;  // Stores the period between pulses in microseconds.
                       // It has a big number so it doesn't start with 0 which would be interpreted as a high frequency.
volatile unsigned long PeriodAverage = ZeroTimeout+1000;  // Stores the period between pulses in microseconds in total, if we are taking multiple pulses.
                       // It has a big number so it doesn't start with 0 which would be interpreted as a high frequency.
unsigned long FrequencyRaw;  // Calculated frequency, based on the period. This has a lot of extra decimals without the decimal point.
unsigned long FrequencyReal;  // Frequency without decimals.
unsigned long RPM;  // Raw RPM without any processing.
unsigned int PulseCounter = 1;  // Counts the amount of pulse readings we took so we can average multiple pulses before calculating the period.

unsigned long PeriodSum; // Stores the summation of all the periods to do the average.

unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;  // Stores the last time we measure a pulse in that cycle.
                                    // We need a variable with a value that is not going to be affected by the interrupt
                                    // because we are going to do math and functions that are going to mess up if the values
                                    // changes in the middle of the cycle.
unsigned long CurrentMicros = micros();  // Stores the micros in that cycle.
                                         // We need a variable with a value that is not going to be affected by the interrupt
                                         // because we are going to do math and functions that are going to mess up if the values
                                         // changes in the middle of the cycle.

// We get the RPM by measuring the time between 2 or more pulses so the following will set how many pulses to
// take before calculating the RPM. 1 would be the minimum giving a result every pulse, which would feel very responsive
// even at very low speeds but also is going to be less accurate at higher speeds.
// With a value around 10 you will get a very accurate result at high speeds, but readings at lower speeds are going to be
// farther from eachother making it less "real time" at those speeds.
// There's a function that will set the value depending on the speed so this is done automatically.
unsigned int AmountOfReadings = 1;

unsigned int ZeroDebouncingExtra;  // Stores the extra value added to the ZeroTimeout to debounce it.
                                   // The ZeroTimeout needs debouncing so when the value is close to the threshold it
                                   // doesn't jump from 0 to the value. This extra value changes the threshold a little
                                   // when we show a 0.

// Variables for smoothing tachometer:
unsigned long readings[numReadings];  // The input.
unsigned long readIndex;  // The index of the current reading.
unsigned long total;  // The running total.
unsigned long average;  // The RPM value after applying the smoothing.

unsigned long readings1[numReadings1];  // The input.
unsigned long readIndex1;  // The index of the current reading.
unsigned long total1;  // The running total.
unsigned long average1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const byte PulsesPerRevolution1 = 2;  // Set how many pulses there are on each revolution. Default: 2.


// If the period between pulses is too high, or even if the pulses stopped, then we would get stuck showing the
// last value instead of a 0. Because of this we are going to set a limit for the maximum period allowed.
// If the period is above this value, the RPM will show as 0.
// The higher the set value, the longer lag/delay will have to sense that pulses stopped, but it will allow readings
// at very low RPM.
// Setting a low value is going to allow the detection of stop situations faster, but it will prevent having low RPM readings.
// The unit is in microseconds.
const unsigned long ZeroTimeout1 = 5000000;  // For high response time, a good value would be 100000.
                                           // For reading very low RPM, a good value would be 300000.




/////////////
// Variables:
/////////////

volatile unsigned long LastTimeWeMeasured1;  // Stores the last time we measured a pulse so we can calculate the period.
volatile unsigned long PeriodBetweenPulses1 = ZeroTimeout1+1000;  // Stores the period between pulses in microseconds.
                       // It has a big number so it doesn't start with 0 which would be interpreted as a high frequency.
volatile unsigned long PeriodAverage1 = ZeroTimeout1+1000;  // Stores the period between pulses in microseconds in total, if we are taking multiple pulses.
                       // It has a big number so it doesn't start with 0 which would be interpreted as a high frequency.
unsigned long FrequencyRaw1;  // Calculated frequency, based on the period. This has a lot of extra decimals without the decimal point.
unsigned long FrequencyReal1;  // Frequency without decimals.
unsigned long RPM1;  // Raw RPM without any processing.
unsigned int PulseCounter1 = 1;  // Counts the amount of pulse readings we took so we can average multiple pulses before calculating the period.

unsigned long PeriodSum1; // Stores the summation of all the periods to do the average.

unsigned long LastTimeCycleMeasure1 = LastTimeWeMeasured1;  // Stores the last time we measure a pulse in that cycle.
                                    // We need a variable with a value that is not going to be affected by the interrupt
                                    // because we are going to do math and functions that are going to mess up if the values
                                    // changes in the middle of the cycle.
unsigned long CurrentMicros1 = micros();  // Stores the micros in that cycle.
                                         // We need a variable with a value that is not going to be affected by the interrupt
                                         // because we are going to do math and functions that are going to mess up if the values
                                         // changes in the middle of the cycle.

// We get the RPM by measuring the time between 2 or more pulses so the following will set how many pulses to
// take before calculating the RPM. 1 would be the minimum giving a result every pulse, which would feel very responsive
// even at very low speeds but also is going to be less accurate at higher speeds.
// With a value around 10 you will get a very accurate result at high speeds, but readings at lower speeds are going to be
// farther from eachother making it less "real time" at those speeds.
// There's a function that will set the value depending on the speed so this is done automatically.
unsigned int AmountOfReadings1 = 1;

unsigned int ZeroDebouncingExtra1;  // Stores the extra value added to the ZeroTimeout to debounce it.
                                   // The ZeroTimeout needs debouncing so when the value is close to the threshold it
                                   // doesn't jump from 0 to the value. This extra value changes the threshold a little
                                   // when we show a 0.
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Encoder output to Arduino Interrupt pin
#define ENC_IN 2 //RPM 
#define ENC_IN2 3 //Linear speed
#define irPin 8
/*Changes log, this code is for 1 ppr but the 1ppr is changed to 400 for reading purposes.
 * added a serial print to reset button in button press logic
*/ 
int count=0;  
boolean state = true;

String temp_str;

// One-second interval for measurements
int interval = 1000;
 
// Counters for milliseconds during interval
long previousMillis = 0;
long currentMillis = 0;

long DpreviousMillis = 0;
long DcurrentMillis = 0;
// Variable for RPM measuerment
int rpm = 0;
int rpm2 = 0;
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
       // Serial.print("[");
       // Serial.print(errorTopic);
        //Serial.print("] WIRE communications error (");
        //Serial.print(error);
        //Serial.print(") : ");

        switch (error)
        {
        case Rtc_Wire_Error_None:
            //Serial.println("(none?!)");
            break;
        case Rtc_Wire_Error_TxBufferOverflow:
            //Serial.println("transmit buffer overflow");
            break;
        case Rtc_Wire_Error_NoAddressableDevice:
            //Serial.println("no device responded");
            break;
        case Rtc_Wire_Error_UnsupportedRequest:
            //Serial.println("device doesn't support request");
            break;
        case Rtc_Wire_Error_Unspecific:
            //Serial.println("unspecified error");
            break;
        case Rtc_Wire_Error_CommunicationTimeout:
            //Serial.println("communications timed out");
            break;
        }
        return true;
    }
    return false;
}

void setup()

{
  Serial.begin(9600);
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
  attachInterrupt(digitalPinToInterrupt(ENC_IN), updateEncoder,FALLING);
  attachInterrupt(digitalPinToInterrupt(ENC_IN2), updateEncoder1,FALLING);
  
  // Setup initial values for timer
  previousMillis = millis();
  DpreviousMillis = millis();
  
  count = readMemory();
  pinMode(7, INPUT_PULLUP);
  
}
 
void loop()
{ 
  tachometer();
  tachometer1();
  int sensorVal = digitalRead(7);
  if(sensorVal == LOW){
     //Serial.println("Reset memory");
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
    
    
    
  }
}
void tachometer(){
   LastTimeCycleMeasure = LastTimeWeMeasured;  // Store the LastTimeWeMeasured in a variable.
  CurrentMicros = micros();  // Store the micros() in a variable.





  // CurrentMicros should always be higher than LastTimeWeMeasured, but in rare occasions that's not true.
  // I'm not sure why this happens, but my solution is to compare both and if CurrentMicros is lower than
  // LastTimeCycleMeasure I set it as the CurrentMicros.
  // The need of fixing this is that we later use this information to see if pulses stopped.
  if(CurrentMicros < LastTimeCycleMeasure)
  {
    LastTimeCycleMeasure = CurrentMicros;
  }





  // Calculate the frequency:
  FrequencyRaw = 10000000000 / PeriodAverage;  // Calculate the frequency using the period between pulses.


  

  
  // Detect if pulses stopped or frequency is too low, so we can show 0 Frequency:
  if(PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra)
  {  // If the pulses are too far apart that we reached the timeout for zero:
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;  // Change the threshold a little so it doesn't bounce.
  }
  else
  {
    ZeroDebouncingExtra = 0;  // Reset the threshold to the normal value so it doesn't bounce.
  }





  FrequencyReal = FrequencyRaw / 10000;  // Get frequency without decimals.
                                          // This is not used to calculate RPM but we remove the decimals just in case
                                          // you want to print it.





  // Calculate the RPM:
  RPM = FrequencyRaw / PulsesPerRevolution * 60;  // Frequency divided by amount of pulses per revolution multiply by
                                                  // 60 seconds to get minutes.
  RPM = RPM / 10000;  // Remove the decimals.





  // Smoothing RPM:
  total = total - readings[readIndex];  // Advance to the next position in the array.
  readings[readIndex] = RPM;  // Takes the value that we are going to smooth.
  total = total + readings[readIndex];  // Add the reading to the total.
  readIndex = readIndex + 1;  // Advance to the next position in the array.

  if (readIndex >= numReadings)  // If we're at the end of the array:
  {
    readIndex = 0;  // Reset array index.
  }
  
  average = total / numReadings;  // The average value it's the smoothed result.
  //Serial.print(RPM);
 // Serial.print("\tTachometer: ");
  //Serial.println(average);

}

void tachometer1(){
   LastTimeCycleMeasure1 = LastTimeWeMeasured1;  // Store the LastTimeWeMeasured in a variable.
  CurrentMicros1 = micros();  // Store the micros() in a variable.





  // CurrentMicros should always be higher than LastTimeWeMeasured, but in rare occasions that's not true.
  // I'm not sure why this happens, but my solution is to compare both and if CurrentMicros is lower than
  // LastTimeCycleMeasure I set it as the CurrentMicros.
  // The need of fixing this is that we later use this information to see if pulses stopped.
  if(CurrentMicros1 < LastTimeCycleMeasure1)
  {
    LastTimeCycleMeasure1 = CurrentMicros1;
  }





  // Calculate the frequency:
  FrequencyRaw1 = 10000000000 / PeriodAverage1;  // Calculate the frequency using the period between pulses.


  

  
  // Detect if pulses stopped or frequency is too low, so we can show 0 Frequency:
  if(PeriodBetweenPulses1 > ZeroTimeout1 - ZeroDebouncingExtra1 || CurrentMicros1 - LastTimeCycleMeasure1 > ZeroTimeout1 - ZeroDebouncingExtra1)
  {  // If the pulses are too far apart that we reached the timeout for zero:
    FrequencyRaw1 = 0;  // Set frequency as 0.
    ZeroDebouncingExtra1 = 2000;  // Change the threshold a little so it doesn't bounce.
  }
  else
  {
    ZeroDebouncingExtra1 = 0;  // Reset the threshold to the normal value so it doesn't bounce.
  }





  FrequencyReal1 = FrequencyRaw1 / 10000;  // Get frequency without decimals.
                                          // This is not used to calculate RPM but we remove the decimals just in case
                                          // you want to print it.





  // Calculate the RPM:
  RPM1 = FrequencyRaw1 / PulsesPerRevolution1 * 60;  // Frequency divided by amount of pulses per revolution multiply by
                                                  // 60 seconds to get minutes.
  RPM1 = RPM1 / 10000;  // Remove the decimals.





  // Smoothing RPM:
  total1 = total1 - readings1[readIndex1];  // Advance to the next position in the array.
  readings1[readIndex1] = RPM1;  // Takes the value that we are going to smooth.
  total1 = total1 + readings1[readIndex1];  // Add the reading to the total.
  readIndex1 = readIndex1 + 1;  // Advance to the next position in the array.

  if (readIndex1 >= numReadings1)  // If we're at the end of the array:
  {
    readIndex1 = 0;  // Reset array index.
  }
  
  average1 = total1 / numReadings1;  // The average value it's the smoothed result.

}
void updateEncoder()
{

  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros();
  if(PulseCounter >= AmountOfReadings)  
  {
    PeriodAverage = PeriodSum / AmountOfReadings;  
    PulseCounter = 1;  
    PeriodSum = PeriodBetweenPulses;  
    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);  
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  }
  else
  {
    PulseCounter++;  // Increase the counter for amount of readings by 1.
    PeriodSum = PeriodSum + PeriodBetweenPulses;  // Add the periods so later we can average.
  }
}
void updateEncoder1()
{
  // Increment value for each pulse from encoder
    PeriodBetweenPulses1 = micros() - LastTimeWeMeasured1;  // Current "micros" minus the old "micros" when the last pulse happens.
                                                        // This will result with the period (microseconds) between both pulses.
                                                        // The way is made, the overflow of the "micros" is not going to cause any issue.

  LastTimeWeMeasured1 = micros();  // Stores the current micros so the next time we have a pulse we would have something to compare with.





  if(PulseCounter1 >= AmountOfReadings1)  // If counter for amount of readings reach the set limit:
  {
    PeriodAverage1 = PeriodSum1 / AmountOfReadings1;  // Calculate the final period dividing the sum of all readings by the
                                                   // amount of readings to get the average.
    PulseCounter1 = 1;  // Reset the counter to start over. The reset value is 1 because its the minimum setting allowed (1 reading).
    PeriodSum1 = PeriodBetweenPulses1;  // Reset PeriodSum to start a new averaging operation.
    int RemapedAmountOfReadings1 = map(PeriodBetweenPulses1, 40000, 5000, 1, 10);  // Remap the period range to the reading range.
    RemapedAmountOfReadings1 = constrain(RemapedAmountOfReadings1, 1, 10);  // Constrain the value so it doesn't go below or above the limits.
    AmountOfReadings1 = RemapedAmountOfReadings1;  // Set amount of readings as the remaped value.
  }
  else
  {
    PulseCounter1++;  // Increase the counter for amount of readings by 1.
    PeriodSum1 = PeriodSum1 + PeriodBetweenPulses1;  // Add the periods so later we can average.
  }
  
}
void updateDisplay(){
if (rpm >= 0) {
       if (DcurrentMillis - DpreviousMillis > Dinterval) {
        DpreviousMillis = DcurrentMillis;
      
       lcd.clear(95,20,120,63,0);
       }
       
      lcd.gotoxy(92,20);
      lcd.string(crpm,false);
      lcd.gotoxy(92,35);
      lcd.string(cspeed,false);
      lcd.gotoxy(92,50);
      lcd.string(citems,false);
     
    }
}
void initDisplay(){
  //Serial.begin(9600); 
  lcd.begin (0x20); 
  TWBR = 20;
  lcd.string ("    MICROLOGSPRAY    ", true);
  lcd.gotoxy(0,20);
  lcd.string ("SLIDER-str/min:", false);
  lcd.gotoxy(0,38);
  lcd.string ("CONVEYOR-m/min:", false);
  lcd.gotoxy(0,48);
  lcd.string ("SKIN COUNT nos:", false); //NEW code has to be tested with padding 
}
void calculations(){
    speeed = (float)3.141*0.1*average1;
    temp_str = String(average);
    temp_str.toCharArray(crpm,5); 
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
            /*Serial.print("data read (");
            Serial.print(gotten);
            Serial.println(")");
            Serial.print("    value1 = ");
            Serial.print(retrievedCount);
            Serial.println();
            */
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
  }  
  if (digitalRead(irPin))  
  {  
    state = true;  

  }  
  }
}
