#include <Sodaq_RN2483.h>
#include <Sodaq_wdt.h>
#include <math.h>

#define DEBUG
#ifdef ARDUINO_ARCH_AVR
  #define debugSerial Serial
  #define LED LED1
#elif ARDUINO_ARCH_SAMD
  #define debugSerial SerialUSB
  #define LED LED_BUILTIN
#endif
#define loraSerial Serial1

//global constants

//const uint8_t DEV_ADDR[4] = { 0x14, 0x20, 0x40, 0xCB };
//const uint8_t NWK_SKEY[16] = { 0x56, 0xf4, 0x83, 0xfb, 0xd9, 0x8a, 0xe2, 0xec, 0x02, 0x90, 0x70, 0xd5, 0x4a, 0x4e, 0xff, 0x78 };
//const uint8_t APP_SKEY[16] = { 0xcc, 0x88, 0x16, 0xae, 0x16, 0x51, 0xfa, 0x7f, 0x8a, 0xa2, 0x01, 0xaa, 0x87, 0xf3, 0x39, 0xfb };
const uint8_t DEV_ADDR[4] = {0x14, 0x00, 0x23, 0x9C};
const uint8_t NWK_SKEY[16] = { 0xa6, 0xef, 0xfb, 0x79, 0xe3, 0x47, 0x4d, 0x0d, 0x91, 0x16, 0x48, 0x80, 0xe8, 0xba, 0xbf, 0x46};
const uint8_t APP_SKEY[16] = { 0xe7, 0xb4, 0xef, 0x64, 0x7b, 0x11, 0x46, 0xc2, 0xcf, 0xda, 0xca, 0x66, 0x03, 0xfc, 0x38, 0x07};

//globar vars
bool softReset = false;

void setup()
{
  //disable watchdog to prevent startup loop after soft reset
  sodaq_wdt_disable();

  // Setup LED, on by default
  pinMode(LED, OUTPUT);

  //supply power to lora bee
  digitalWrite(BEE_VCC, HIGH);
  
#ifdef DEBUG
  //Wait for debugSerial or 10 seconds
  while ((!debugSerial) && (millis() < 10000));
  debugSerial.begin(9600);
  LoRaBee.setDiag(debugSerial);
#endif

  //setup LORA connection
  while(!loraSerial);
  loraSerial.begin(LoRaBee.getDefaultBaudRate());
  
  if (!LoRaBee.initABP(loraSerial, DEV_ADDR, APP_SKEY, NWK_SKEY))
  {
    logMsg("Connection to the network failed!");
    softReset = true;
  }
  else
  {
    logMsg("Connection to the network successful.");
    setDataRate(0); //HACK because no acknoledgements received
    //sendTestPacket();
  }

  onSetup();
}

void loop()
{
   doLoop();
}

void sendTestPacket() {
  const uint8_t testPayload[] = { 0x30, 0x31, 0xFF, 0xDE, 0xAD };
  logMsg("Sending test packet...");
  LoRaBee.send(1, testPayload, 5);
}

void setDataRate(int dr) {
  loraSerial.print("mac set dr 0\r\n");
  String result = loraSerial.readString();
  logMsg("Setting data rate to " + String(dr) + ": " + result);
}

void logMsg(String msg)
{
  #ifdef DEBUG
  debugSerial.println(msg);
  #endif
}
