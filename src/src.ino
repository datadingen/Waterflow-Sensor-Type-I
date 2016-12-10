#include <Sodaq_RN2483.h>

#define DEBUG
#define debugSerial SerialUSB
#define loraSerial Serial1
#define LED LED_BUILTIN

const int WAKEUP_INTERVAL_S = 900;
const byte sensorPin = 1;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
const float calibrationFactor = 450; //pulses/L
volatile unsigned short pulseCount = 0;

unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;
unsigned long oldTime = 0;

void onSetup()
{
  // The Hall-effect sensor is connected to pin  which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);
  attachInterrupt(sensorPin, pulseCounter, FALLING);
}

void doLoop()
{
  //if(millis() < oldTime + WAKEUP_INTERVAL_S * 1000) return;
  //oldTime = millis();
  
  //start LED flash
  digitalWrite(LED, HIGH);

  //retreave and reset pulseCount
  unsigned short currentPulseCount = pulseCount;
  noInterrupts();
  pulseCount = 0;
  interrupts();
  
  // Calculate the milliliters passed during the last interval
  flowMilliLitres = currentPulseCount * 1000 / calibrationFactor;
  
  // Add the millilitres passed in this second to the cumulative total
  totalMilliLitres += flowMilliLitres;

  // Print the cumulative total of litres flowed since starting
  logMsg("Liquid Quantity since last time: " + String(flowMilliLitres) + "mL");
  logMsg("Total Liquid Quantity: " + String(totalMilliLitres) + "mL");
  
  sendMsg();

  digitalWrite(LED, LOW);

  delay(WAKEUP_INTERVAL_S * 1000);
}

//Insterrupt Service Routine
void pulseCounter()
{
  pulseCount++;
}
  
void sendMsg() {
  //construct payload from totalMilliLitres
  char payload[4];
  for(int i = 0; i <= 3; i++)
  {
    char c = 0xFF & totalMilliLitres >> 8*i;
    payload[3 - i] = c;
  }
  
  logMsg("Sending total millilitres: " + String(totalMilliLitres));
  LoRaBee.send(1, (uint8_t*)payload, 4);
}
