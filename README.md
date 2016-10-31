Arduino Sensor for Measuring Bioluminescence
============================================

Based on: [Highly sensitive Arduino light sensor by Patrik on Instructables](http://www.instructables.com/id/Highly-sensitive-Arduino-light-sensor)

## Hardware Set Up

Connect sensor to Arduino:

- red: VCC 5V
- black: GND
- green: Data Pin 2

## Software Set Up

### Set RTC Date/Time

Open `examples/RTCLib/ds1307` and move `rtc.adjust()` call outside if statement within `setup()`.

```cpp

void setup () {

#ifndef ESP8266
  while (!Serial); // for Leonardo/Micro/Zero
#endif

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("Setting RTC Date/Time...");
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}
```

## Light Sensor Theory

Light Sensor: [TSL237S](https://www.mouser.com/Search/ProductDetail.aspx?R=TSL237S-LFvirtualkey57530000virtualkey856-TSL237S-LF) ([Datasheet](http://www.jameco.com/Jameco/Products/ProdDS/2151435.pdf))

The TSL237S light sensor translates light to square wave pulses. The frequency of the square wave (number of pulses per second) is proportional to the amount of light.

The dark frequency for this sensor is 0.1 Hz meaning that in the absence of light, the sensor will emit a 0.1 Hz square wave (1 pulse per 10 seconds).

To convert the light sensor output to a measure of light, the Arduino simply counts the number of pulses per second.

The irradiance responsitivity is 2.3 kW/(uW/cm2).

```
fO = fD + (Re)(Ee)
Ee = (fO - fD)/Re

fO = output frequency (Hz)
fD = dark frequency (Hz) = 0.1 Hz
Re = device responsivity for given wavelength = 2.3 kHz/(uW/cm2)
Ee = incident irradiance (uW/cm2)
```

Note that the sketch code includes 1000.0 factor to convert frequencies from Hz to kHz:

```
Ee = (fO - fD)/1000.0/Re;
```
