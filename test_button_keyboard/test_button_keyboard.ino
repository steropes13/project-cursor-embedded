// we used mbed.h for arm architecture due to the nrf52xx of the arduino 33 BLE Sense 
//for further informations see:  https://github.com/arduino/ArduinoCore-nRF528x-mbedos/blob/master/cores/arduino/mbed/drivers/USBKeyboard.h


#include "PluggableUSBHID.h" //HID FUNCTIONNALITY LOW LEVEL
#include "USBMouseKeyboard.h" // SIMPLIFIED INTERFACE FOR KEYBOARD + MOUSE 

const int buttonPin = 11; //we choose the pin 11 for the external button 
                          //on the breadboard


USBMouseKeyboard key_mouse; //creating instance for keyboard + mouse
int lastState = LOW; //starts with a by default state

void setup() {
  pinMode(buttonPin, INPUT_PULLDOWN); //as we have a resistance we define it as pulldown (1 if pressed)
  pinMode(LED_BUILTIN, OUTPUT); //we define it as output to activate it at each time we press the button 

  delay(5000); //in order for the computer to configure it easilly 
}


void loop() {
  int state = digitalRead(buttonPin); //at each time in the loop it takes the state of the button 

  if (lastState == LOW && state == HIGH) { //if it is pressed and it was not already the case 
                                          // allows us to run activate the spacebar only one time when the button is pressed 
                                          // we do not count on the button is pressed "longly"

    digitalWrite(LED_BUILTIN, HIGH);   // the built-in led is turned-on  
    key_mouse.printf(" "); //we write the character " " which corresponds to the spacebar on the user keyboard
    //key_mouse.printf("HELLO\n");
    //key_mouse.move(50, 0);

    delay(500); // we wait 0.5 seconds

    digitalWrite(LED_BUILTIN, LOW);    //the built-in led is turned-off 
  }

  lastState = state; //we keep the last state 
}


/*
SOME BUGS CAN OCCUR : 
I haven't had a lot of success with trying to make it more "Arduino".

IMPORTANT: this sketch will cause your Nano 33 BLE to no longer have a port, meaning you can't upload to it. 
The solution is to press and release the reset button on the Nano 33 BLE twice quickly. After that, 
you should see the "L" LED pulsing to indicate it's in bootloader mode. If you don't see the LED pulsing,
 try the double reset again until you get
 the timing right. Once the board is in bootloader mode, you will see a port and be able 
 to upload again. Note that the port number may change when it's in bootloader mode, so make sure 
 to select the correct port from the Arduino IDE's Tools > Port menu before uploading.


*/