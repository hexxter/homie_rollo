#include <Arduino.h>

/* WARNING: untested */

#include <Homie.h>

#define FW_NAME "itead-sonoff_dual"
#define FW_VERSION "1.0.5"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */


const int PIN_LED = 13;
const int PIN_BUTTON = 0;

bool relay1 = false;
bool relay2 = false;

HomieNode rolloNode("drive", "drive");

bool bset;
bool rolloHandler(String value) {
  Serial.println(value);
  bset = false;
  if (value == "up") {
    relay( 1, true );
  } else if (value == "off") {
    relay( 1, false );
    relay( 2, false );
  } else if (value == "down") {
    relay( 2, true );
  } else {
    return false;
  }
  return true;
}

void relay( int num, bool val ){
  bool doit = false;
  if( num == 1 ){
    if( relay1 != val ){
      if( val ){
        relay2 = false;
        relay1 = true;
        Homie.setNodeProperty(rolloNode, "direction", "up");
        Serial.println("Rollo UP");
      }else{
        relay1 = false;
        Homie.setNodeProperty(rolloNode, "direction", "off");
        Serial.println("Rollo OFF");
      }
      doit = true;
    }
  }else{
    if( relay2 != val ){
      if( val ){
        relay1 = false;
        relay2 = true;
        Homie.setNodeProperty(rolloNode, "direction", "down");
        Serial.println("Rollo DOWN");
      }else{
        relay2 = false;
        Homie.setNodeProperty(rolloNode, "direction", "off");
        Serial.println("Rollo OFF");
      }
      doit = true;
    }
  }
  if( doit ) {
    setrelays();
    setrelays();
  }
}

void setup() {
  Homie.enableLogging( false );
  Homie.setFirmware(FW_NAME, FW_VERSION);
  Homie.setLedPin(PIN_LED, LOW);
  Homie.setResetTrigger(PIN_BUTTON, LOW, 5000);
  rolloNode.subscribe("direction", rolloHandler);
  Homie.registerNode(rolloNode);
  Homie.setup();
  Serial.begin(19200);
  setrelays();
}

void loop() {
  Homie.loop();
  readButtons();
}

void setrelays() {
  byte b = 0;
  if (relay1) b++;
  if (relay2) b += 2;
  Serial.write(0xA0);
  Serial.write(0x04);
  Serial.write(b);
  Serial.write(0xA1);
  Serial.flush();
}

int incomingByte = 0;
int iStep = 0;
int iNewState = 0;
void readButtons(){
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    if (incomingByte == 0xA0) {
      iStep = 1;
    }
    else if ((iStep == 1) && (incomingByte == 0x04)) {
      iStep = 2;
    }
    else if ((iStep == 2) && (incomingByte >= 0) && (incomingByte <= 3)) {
      iStep = 3;
      iNewState = incomingByte;
    } else if ((iStep == 3) && (incomingByte == 0xA1)) {
      iStep = 0;
      if (iNewState == 0) {
        relay( 1, false );
        relay( 2, false );
      }else if (iNewState == 1) {
        relay( 1, true );
      }else if (iNewState == 2) {
        relay( 2, true );
      }else if (iNewState == 3) {
        relay( 1, false );
        relay( 2, false );
      }
    } else iStep = 0;
  }
}
