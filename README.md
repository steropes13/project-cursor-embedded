# Slideshow remote provided with TinyML \& USB HID control on Arduino Nano 33 BLE sense
This PlatformIO project corresponds to the final project of the 'Low-Power Embedded Systems' course. I  re-used the first part of the project we did with another student (see (here)[https://github.com/steropes13/EMBEDDED-GESTURE-RECO#]). 
More precisely, the part with the `ipynb` classifiction pipeline via Jupyter Notebook file and the writing 
of the file  `platformio.ini` and the main idea of the `main.cpp` with the diverse environments. 

In this project,  I perform gesture recognition using the IMU sensor of the Arduino Nano 33 BLE Sense to use it as a slideshow remote that can be recognized by the host OS (**I tried on Mac only**) as a keyboard.  
I used here MBED library to perform the HIDmouseKeyboard recognition for a better compatibility (_ARM architecture in my case_).  

I added on my solution (see `report/`  directory) some componenent for the user feedback and direct interaction : such as a button and LED.    

The project is divided into three main parts:
1. Collecting data from the IMU sensor;
2. Training a Neural Network model for gesture recognition using the collected data;
3. Implementing the trained model on the Arduino Nano 33 BLE Sense to perform real-time gesture recognition.
4. Sending special keys to the host and mapping it (left/right arrow, Home)



# Compiling the project

## Compiling the project (using VSCode and PlatformIO)
1. Install VSCode on you PC;
2. Install PlatformIO from the extension menu on VSCode;
3. Clone the repository on your local machine;
4. Open it using VSCode (**especially the directory where platformio.ini is located**);
5. After PlatformIO loads, select the Environment for you own board from the down bar;
6. Click on Upload on the down bar;
7. Open the Console.
> NOTE: PlatformIO needs access to USB ports, maybe it's needed to update udev rules under linux or installing some driver for different OSes 

## Compiling the project (using PlatformIO CLI)
1. Install PlatformIO CLI on your PC, following the instructions on the official (website)[https://docs.platformio.org/en/latest/core/installation/methods/index.html];
2. Allow PlatformIO to access USB ports, maybe it's needed to update udev rules under linux or installing some driver for different OSes;
3. Be sure to have acces to PlatformIO Core CLI binaries, see (here)[https://docs.platformio.org/en/latest/core/installation/shell-commands.html] for more details;
4. Clone the repository on your local machine;
5. Open a terminal and navigate to the project folder;
6. Run the command `pio run -e <environment_name> -t upload` to compile and upload the code to your board, replacing `<environment_name>` with the name of the environment you want to use (see down below for more details);
7. Open the Console or use `pio run -e <environment_name> -t monitor` to monitor the serial output.

## Environments description
Here you can find a description of the different environments available in the `platformio.ini` file:
- `nano33ble_Sense` & `nano33ble_SenseRev2`: these environments are used to compile the gesture recognition code for the Arduino Nano 33 BLE Sense and the Arduino Nano 33 BLE Sense Rev2, respectively.
- `nano33ble_Sense_DataCollection` & `nano33ble_SenseRev2_DataCollection`: these environments are used to compile the data collection code for the Arduino Nano 33 BLE Sense and the Arduino Nano 33 BLE Sense Rev2, respectively.
- `nano33ble_Sense_DataCollection_Rest` & `nano33ble_SenseRev2_DataCollection_Rest`: these environments are used to compile the data collection code for the Arduino Nano 33 BLE Sense and the Arduino Nano 33 BLE Sense Rev2, respectively. Note that these environments are used to collect data for the 'rest' gesture, which is different from the other gestures infatc this collection is done continuously without the need of passing an acceleration threshold to start the collection. 

> Note: The data collection code performs also the feature extraction, so the collected data are already in the format needed for the training of the Neural Network model.