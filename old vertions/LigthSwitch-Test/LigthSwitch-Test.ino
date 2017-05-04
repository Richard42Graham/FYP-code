#include <AutoHome.h>   // a libary from here https://github.com/ESPAutoHome/AutoHomeESP-Library me and a frined have developed it. 
AutoHome autohome;

#define MOTOR_R 13  // motor controle pin up   (1)
#define MOTOR_F 14  // motor controle pin down (2)
#define MOTOR_A A0  // analog input from potentomitor

#define HIGH_END 500  // upper arm limit
#define LOW_END 300    // lower arm limit
#define DIF 2 // 2         // threshold for

#define M_STOP 0      // define states of motor movment
#define M_UP 1
#define M_DOWN 2

// chennel name and shiz

int currentMotionState = M_STOP;
int currentValue = 0;
int lastValue = 0;
long lastMills = 0;

void motorControl();
void switchDetect();

/* This function will be called every time a packet is received from the mqtt topic. */
/* This is registered in the setup() */
void mqtt_callback(char* topic, byte* payload, unsigned int length)
{

  if (autohome.mqtt_callback(topic, payload, length) == 0)
  {
        String packet = "";
        for (int i = 0; i < length; i++) 
          {
             packet = packet + (char)payload[i];
          }
      Serial.print(packet);               // debug print incomeing MQTT messages to serial.


if (autohome.getValue(packet, ':', 0).equals("TO"))
              { 
                 int poss;    
                 poss = (autohome.getValue(packet, ':', 1)).toInt();    // the mode the buzzer is in.
                      switch(poss)
                          {
                            case 0 : 
                                { currentMotionState = M_DOWN;}
                                break;

                           case 255 :
                               { currentMotionState = M_UP; }
                               break;
                          }
              }
                 
         
  }
}


void setup() 
{
  Serial.begin(115200);

  /* This registers the function that gets called when a packet is recieved. */
  autohome.setPacketHandler(mqtt_callback);

  /* This starts the library and connects the esp to the wifi and the mqtt broker */
  autohome.begin();

  /* Setup Motor Controller */
  pinMode(MOTOR_F, OUTPUT);  // set FIN as a output
  pinMode(MOTOR_R, OUTPUT);  // set RIN as a output
  digitalWrite(MOTOR_F, LOW);  // set both the motor controle pins to low, this disables the motor driver chip, making it floppy.
  digitalWrite(MOTOR_R, LOW);

  currentValue = analogRead(MOTOR_A);
  lastValue = currentValue;

  // setup motor to a know position and state ? 
}



void loop() 
{

  /* This needs to be called in the loop as it handels the reconection to the mqtt server if it disconnects*/
  autohome.loop();
  delay(8);

  currentValue = analogRead(MOTOR_A);
   Serial.println(currentValue);

// if( (currentValue <= 10) || (currentValue >= 245) )
{ // test
 //  switchDetect();     // run function to detect a movement, and if it should be flicked
}
//   endstopDetection(); // make sure the arm has not reached the end stops
 //  motorControl();     // acutate the arm. 

  if ((millis() - lastMills) > 100)     // sneeky way to do timed events in seconds without hanging the CPU. 
    {
    
          //      if (currentValue != lastValue && currentMotionState == M_STOP && Waiter) // have a cool down period    
      if (currentValue != lastValue && currentMotionState == M_STOP )  // detect change in position
          //  if ( (currentValue != lastValue) && (currentValue != (lastValue -1)) && (currentValue != (lastValue +1)) && currentMotionState == M_STOP)  // detect change in position
                // and maybe add in a cool down counter ? to prevent spamming the server  ? 
        {                // +- 1 last value
            int valOut = currentValue;

           if (valOut > HIGH_END) 
           {
              valOut = HIGH_END;
           } 
              else if (valOut < LOW_END) 
                {
                   valOut = LOW_END;
                }
           if(valOut == currentValue)  // +- 2? 
           {
          
              String out = "SWITCH:IS:" + String(map(valOut, HIGH_END, LOW_END, 255, 0));  // lose MQTT fuction useing a libary to send data out.
              autohome.sendPacket(out.c_str());
              Serial.println("switch is");         // debug things
              Serial.print(out);
           }
           
        }
   
           else 
            {
              switch (currentMotionState)
                {
                 case M_DOWN: autohome.sendPacket("SWITCH:IS:0"); break;
                 case M_UP: autohome.sendPacket("SWITCH:IS:255"); break;
                 Serial.println("motor down or up");
                }

            }



    
          lastMills = millis();
    }
    
                  if (currentValue == lastValue && (currentMotionState == M_DOWN || currentMotionState == M_UP))
          { Serial.println("dimmin ?"); 
             currentMotionState = M_STOP;         
             motorControl();     // acutate the arm. 
             delay(20);
          }
     delay(66);
 lastValue = currentValue;
}




void endstopDetection()     // set motor state to stop, if its highter than the HIGH_END levle, or lower than the LOW_end levle.
{
  if ((currentValue >= HIGH_END) && currentMotionState == M_UP) 
    {
        currentMotionState = M_STOP;
    } 
      else if ((currentValue <= LOW_END) && currentMotionState == M_DOWN) 
        {
          currentMotionState = M_STOP;
        }

  if (currentMotionState == M_STOP)
            {   motorControl();  }

        // add if motor mode moveing && Var not changed by morethan 3 stop.
}




void switchDetect() 
{
  if (currentMotionState == M_STOP) 
    {
       if (currentValue <= HIGH_END && currentValue >= LOW_END) 
        {
             if ((lastValue) == currentValue) 
               {
                  currentMotionState = M_STOP;
                  Serial.println("flickin");
                  lastMills = millis();
               }
          
             if ((lastValue + DIF) > currentValue) 
               {
                  currentMotionState = M_DOWN;
               }
            
             if ((lastValue - DIF) < currentValue) 
              {
                 currentMotionState = M_UP;
              }
        }
  }
}




void motorControl() 
{   // ive put this all in a function to optomise timeing, and leave room for adding complexity.

  switch (currentMotionState)
    {
      
        default:          // this just makes the defult for it to not power the motor
        case M_STOP: 
            {
              digitalWrite(MOTOR_F, LOW);
              digitalWrite(MOTOR_R, LOW);
              break;
            }


    case M_UP: 
        {
            digitalWrite(MOTOR_F, HIGH);
            digitalWrite(MOTOR_R, LOW);
            break;
        }


    case M_DOWN: 
        {
            digitalWrite(MOTOR_F, LOW);
            digitalWrite(MOTOR_R, HIGH);
            break;
        }
    }
}

