/*********************
Temperature Controlled Solder Iron

- Read the temperature of the iron PTC sensor
- Read the setpoint potentiometer (scale this to a temperature)
- Turn the heater element on if cold, off if hot
- Show the setpoint and actual temperatures on a 16x2 LCD.

Iron temperature reading is scaled by a non-inverting opamp with gain=2.22 to give us a wider dynamic range of the ADC.
Readings were calibrated using a thermocouple attached to the iron tip

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

#define setpointPin A0
#define sensorPin A1
#define heaterPin 2
#define ledPin 5
#define minTemp 19
#define minADC 222       // 0.487 * Gain (2.22) / 4.88mV (ADC step size) = 221
#define maxTemp 500
#define maxADC 670      // 1.572 * 2.22 / 4.88mV = 715, 650
#define numReadings 3  // We average over 3 readings
#define hysteresis 5

int actualStore = 0;
int setpointStore = 0;
int actual = 0;
int setpoint = 0;
int lcdUpdate = 1000;       // LCD refresh rate (milliseconds)
int readCount = 0;
bool heat = 0;
unsigned long timeNow = 0;

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

void update_screen(int temp, int preset) {
  //lcd.clear();
  char buf[16];

  // Show the measured temperature
  lcd.setCursor(0,0);
  sprintf(buf, "Actual: %3i%cC", temp, (char)223);
  lcd.print(buf);

  // Show the setpoint temperature
  lcd.setCursor(0,1);
  if (preset<=minTemp){
    sprintf(buf, "Preset: OFF");
  }
  else{
    sprintf(buf, "Preset: %3i%cC", preset, (char)223);
  }
  lcd.print(buf);
  
}

void setup() {

  // Make sure our heater and LED are off
  pinMode(heaterPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  pinMode(setpointPin,INPUT);
  pinMode(sensorPin,INPUT);
  digitalWrite(heaterPin, 0);
  digitalWrite(ledPin, 0);

  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print("    KG Rocks    ");
  lcd.setCursor(0,1);
  lcd.print("  Oh Yeah Baby  ");
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Hello");
  delay(2000);
  lcd.clear();  
}


void loop() {

  // Read the preset and actual temperature values and scale to degrees centigrade
  // Average the readings
  if (readCount < numReadings){
    // Read and scale the analog values
    actualStore += analogRead(sensorPin);
    setpointStore += analogRead(setpointPin);
    readCount ++;
  } else {
    actual = map(int((actualStore / numReadings) + 0.5),minADC,maxADC,minTemp,maxTemp);
    setpoint = map(int((setpointStore / numReadings) + 0.5),0,1024,minTemp,maxTemp);
    //Serial.println(analogRead(sensorPin));
    actualStore = 0;
    setpointStore = 0;
    readCount = 0;
  }

//  actual = map(int(analogRead(sensorPin)), minADC, maxADC, minTemp, maxTemp);
//  setpoint = map(int(analogRead(setpointPin)), 0, 1024, minTemp, maxTemp);

  // Update the screen - this sum handles the millis rollover
  // Difference of unsigned long works for this
  if ((millis() - timeNow) > lcdUpdate){
    timeNow = millis();
    update_screen(actual, setpoint);
  }

  // Control the heater
  if (setpoint<=24){
    heat = 0;
    digitalWrite(ledPin,LOW);
    digitalWrite(heaterPin,LOW);
  }
  else if(heat==0){
    if (actual<=setpoint-hysteresis){
      heat = 1;
      digitalWrite(ledPin,HIGH);
      digitalWrite(heaterPin,HIGH);
      Serial.println("on");
    }
  }
  else if(heat==1){
    if (actual>setpoint){
      heat = 0;
      digitalWrite(ledPin,LOW);
      digitalWrite(heaterPin,LOW);
      Serial.println("off");
    }
  }  
}
