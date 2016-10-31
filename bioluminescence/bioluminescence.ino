// Highly Sensitive Arduino Light Sensor for Measuring Bioluminescence
// Sketch based on http://www.instructables.com/id/Highly-sensitive-Arduino-light-sensor
// Data logger code based on http://www.ladyada.net/make/logshield/lighttempwalkthru.html
// We're using the Taos TSL237S-LF light-to-frequency sensor: http://www.taosinc.com/getfile.aspx?type=press&file=tsl237-e37.pdf

#include "SD.h"
#include <Wire.h>
#include "RTClib.h"

int timing = 100; // in milliseconds
volatile unsigned long pulse_cnt = 0;

//These define the pin connections of the Arduino.  
//They can be changed but only use digital in 2 or 3 for the Freq pin
#define TSL_FREQ_PIN 2 // output use digital pin2 for interrupt

float fD = 0.1; // Dark frequency
float Re = 2.3; // Irradiance responsivity
float eff475 = 0.113; // Luminous efficiency at 475nm, for dinoflagellate bioluminescence

RTC_DS1307 RTC; // define the Real Time Clock object

#define WRITE_TO_LOG     1 // write data to log file
#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// the digital pins that connect to the LEDs
#define redLEDpin 3
#define greenLEDpin 4

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  
  while(1);
}

void setup() {
  
  Serial.begin(9600);
  Serial.println();
  Wire.begin();
  RTC.begin();

#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START


#if WRITE_TO_LOG
  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      Serial.println("Opening file...");
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);
#endif

  Wire.begin();
  if (!RTC.begin()) {
#if WRITE_TO_LOG
      logfile.println("RTC failed");
#endif

#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif
  }

#if WRITE_TO_LOG
  logfile.println("millis,date,time,pulse_count,output_freq,uWattpercm2");    
#endif

#if ECHO_TO_SERIAL
  Serial.println("millis,date,time,pulse_count,output_freq,uWattpercm2");
#endif // attempt to write out the header to the file
  
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);

  pinMode(TSL_FREQ_PIN, INPUT);
}

void loop() {
  float Ee;
  float fO;

  // attach interrupt to pin2, send output pin of TSL230R to arduino 2
  // call handler on each rising pulse

  // High light intensity will trigger interrupts faster than the Arduino can handle, 
  // which may screw up timing of millis() and delay(), which are also interrupt based.
  // To flag excessive interrupts, we keep track of both millis() and the RTC time 
  // before and after our sampling interval.
  
  pulse_cnt=0;
  uint32_t millis1 = millis();
  DateTime time1 = RTC.now();

  attachInterrupt(0, add_pulse, RISING);
  delay(timing);
  detachInterrupt(0);

  uint32_t millis2 = millis();
  DateTime time2 = RTC.now();
  unsigned long finalcnt=pulse_cnt;
  
  // Flash green LED at end of sampling interval.
  digitalWrite(greenLEDpin, HIGH);
  delay(500);
  digitalWrite(greenLEDpin, LOW);

  fO= finalcnt/(timing/1000.0); // output frequency (Hz)
  Ee = (fO - fD)/1000.0/Re;     // irradiance (uW/cm2), fO = fD + (Re)(Ee)

#if WRITE_TO_LOG
  logfile.print(millis2);
  logfile.print(", ");    

  if (time2.month() < 10) logfile.print("0");
  logfile.print(time2.month(), DEC);
  logfile.print("/");
  if (time2.day() < 10) logfile.print("0");
  logfile.print(time2.day(), DEC);
  logfile.print("/");
  logfile.print(time2.year(), DEC);
  logfile.print(",");
  if (time2.hour() < 10) logfile.print("0");
  logfile.print(time2.hour(), DEC);
  logfile.print(":");
  if (time2.minute() < 10) logfile.print("0");
  logfile.print(time2.minute(), DEC);
  logfile.print(":");
  if (time2.second() < 10) logfile.print("0");
  logfile.print(time2.second(), DEC);
  logfile.print(", ");

  logfile.print(finalcnt, 10);
  logfile.print(", ");
  
  logfile.print(fO, 10);
  logfile.print(", ");

  logfile.print(Ee, 10);

  logfile.println();
  logfile.flush();
#endif
  
#if ECHO_TO_SERIAL 
  Serial.print(millis2);
  Serial.print(", ");    

  if (time2.month() < 10) Serial.print("0");
  Serial.print(time2.month(), DEC);
  Serial.print("/");
  if (time2.day() < 10) Serial.print("0");
  Serial.print(time2.day(), DEC);
  Serial.print("/");
  Serial.print(time2.year(), DEC);
  Serial.print(",");
  if (time2.hour() < 10) Serial.print("0");
  Serial.print(time2.hour(), DEC);
  Serial.print(":");
  if (time2.minute() < 10) Serial.print("0");
  Serial.print(time2.minute(), DEC);
  Serial.print(":");
  if (time2.second() < 10) Serial.print("0");
  Serial.print(time2.second(), DEC);
  Serial.print(", ");

  Serial.print(finalcnt, 10);
  Serial.print(", ");
  
  Serial.print(fO, 10);
  Serial.print(", ");
  
  Serial.print(Ee, 10);
  
  Serial.println();
#endif
      
  delay(0);
}

void add_pulse() {
  // increase pulse count
  pulse_cnt++;
  return;
}
