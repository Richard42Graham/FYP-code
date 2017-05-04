/* This example is a simple setup example to show you how to setup the AutoHome library. */

#include <AutoHome.h>

AutoHome autohome;

#define MOTOR_R 13
#define MOTOR_F 14
#define MOTOR_A A0

#define HIGH_END 740
#define LOW_END 50
#define DIF 4

#define M_STOP 0
#define M_UP 1
#define M_DOWN 2

int currentMotionState = M_STOP;
int currentValue = 0;
int lastValue = 0;

void motorControl();
void switchDetect();

/* This function will be called every time a packet is received from the mqtt topic. */
/* This is registered in the setup() */
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

  if (autohome.mqtt_callback(topic, payload, length) == 0) {

    String packet = "";

    for (int i = 0; i < length; i++) {
      packet = packet + (char)payload[i];
    }

    Serial.print(packet);

  }

}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  /* This registers the function that gets called when a packet is recieved. */
  autohome.setPacketHandler(mqtt_callback);

  /* This starts the library and connects the esp to the wifi and the mqtt broker */
  autohome.begin();

  /* Setup Motor Controller */
  pinMode(MOTOR_F, OUTPUT);  // set FIN as a output
  pinMode(MOTOR_R, OUTPUT);  // set RIN as a output
  digitalWrite(MOTOR_F, LOW);  // set both the motor controle pins to low, this disables the motor driver chip
  digitalWrite(MOTOR_R, LOW);

  currentValue = analogRead(MOTOR_A);
  lastValue = currentValue;


}

void loop() {

  /* This needs to be called in the loop as it handels the reconection to the mqtt server if it disconnects*/
  autohome.loop();

  currentValue = analogRead(MOTOR_A);

  switchDetect();
  endstopDetection();
  motorControl();

  /* Temp serial Test */
  //  if (Serial.available()) {
  //
  //    char ch = Serial.read();
  //
  //    switch (ch){
  //
  //      case '0': currentMotionState = M_STOP; break;
  //      case 'u': currentMotionState = M_UP; break;
  //      case 'd': currentMotionState = M_DOWN; break;
  //
  //    }
  //
  //
  //
  //  }

  Serial.println("Current State : " + String(currentMotionState) + " Val : " + String(analogRead(MOTOR_A)));

  lastValue = currentValue;

}

void endstopDetection() {

  if (!(currentValue <= HIGH_END) && currentMotionState == M_UP) {

    currentMotionState = M_STOP;

  } else if (!(currentValue >= LOW_END) && currentMotionState == M_DOWN) {

    currentMotionState = M_STOP;

  }

}

void switchDetect() {

  if (currentMotionState == M_STOP) {

    if (currentValue <= HIGH_END && currentValue >= LOW_END) {

      if ((lastValue - DIF) > currentValue) {

        currentMotionState = M_DOWN;

      }

      if ((lastValue + DIF) < currentValue) {

        currentMotionState = M_UP;

      }

    }

  }

}

void motorControl() {

  switch (currentMotionState) {

    default:
    case M_STOP: {

        digitalWrite(MOTOR_F, LOW);
        digitalWrite(MOTOR_R, LOW);

        break;

      }

    case M_UP: {

        digitalWrite(MOTOR_F, HIGH);
        digitalWrite(MOTOR_R, LOW);

        break;

      }

    case M_DOWN: {

        digitalWrite(MOTOR_F, LOW);
        digitalWrite(MOTOR_R, HIGH);
        break;

      }

  }

}

