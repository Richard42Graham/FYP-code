/* This example is a simple setup example to show you how to setup the AutoHome library. */

#include <AutoHome.h>

AutoHome autohome;

#define RED_PIN 13
#define GREEN_PIN 12
#define BLUE_PIN 14
#define WHITE_PIN 2
#define ENABLE 15     // redundent

int currR = 0;
int currG = 0;
int currB = 0;
int currW = 0;

void SETRGB(int r, int g, int b, int w);

/* This function will be called every time a packet is received from the mqtt topic. */
/* This is registered in the setup() */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  if (autohome.mqtt_callback(topic, payload, length) == 0) {

    String packet = "";

    for (int i = 0; i < length; i++) {
      packet = packet + (char)payload[i];
    }


    if (autohome.getValue(packet, ':', 0).equals("RGB")) {      // compair first part of the string

      if (autohome.getValue(packet, ':', 1).equals("STATIC")) { // compair second part of the string, 
                                                                // would have added options for "DYNAMIC" as in fading coulors

        String colors = autohome.getValue(packet, ':', 2);
        SETRGB(autohome.getValue(colors, ',' , 0).toInt(), autohome.getValue(colors, ',' , 1).toInt(), autohome.getValue(colors, ',' , 2).toInt(), autohome.getValue(colors, ',' , 3).toInt());

        String packet = "RGB:IS:" + String(currR) + "," + String(currG) + "," + String(currB) + "," + String(currW);  // send reply with current (new) vales
        autohome.sendPacket( packet.c_str() );

      }

    }

    Serial.print(packet); // for debug

  }

}


void setup() {

  // put your setup code here, to run once:

  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, HIGH);

  pinMode(WHITE_PIN, OUTPUT);
  digitalWrite(WHITE_PIN, HIGH);  // this means when ever the light is power it turns on right away, making it allmost indistingishable from a normal light. 

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  Serial.begin(115200);

  /* This registers the function that gets called when a packet is recieved. */
  autohome.setPacketHandler(mqtt_callback);

  /* This starts the library and connects the esp to the wifi and the mqtt broker */
  autohome.begin();

  analogWriteFreq(500);
  analogWriteRange(255);

}

void loop() {

  /* This needs to be called in the loop as it handels the reconection to the mqtt server if it disconnects*/
  autohome.loop();      // my libary that deels with MQTT conneciotns, OTA (over the air) firm whare uploads and wifi

}

void SETRGB(int r, int g, int b, int w) {   // simple function that drives the PWM of each color based on the new values of color the lights been sent. 

  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
  analogWrite(WHITE_PIN, w);

  currR = r;
  currG = g;
  currB = b;
  currW = w;

}
