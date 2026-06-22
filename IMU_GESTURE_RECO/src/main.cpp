/*
  IMU Classifier (fork for this project)
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU, once enough samples are read, it then uses a
  TensorFlow Lite (Micro) model to try to classify the movement as a known gesture.
  Note: The direct use of C/C++ pointers, namespaces, and dynamic memory is generally
        discouraged in Arduino examples, and in the future the TensorFlowLite library
        might change to make the sketch simpler.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino.h>
#include "USBMouseKeyboard.h"
#include "PluggableUSBHID.h"
#include <arduinoFFT.h>

//#define COLLECT_DATA //has to be removed if no collection

#ifdef COLLECT_DATA
  #warning "This code is for collecting data to train the model"
  #define SERIAL_PRINTLN(...) 
#else 
  #define SERIAL_PRINTLN(...) Serial.println(__VA_ARGS__)
#endif

#ifdef NANO33BLE_SENSE_REV2
  #include <Arduino_BMI270_BMM150.h>
#elif defined(NANO33BLE_SENSE)
  #include <Arduino_LSM9DS1.h>
#endif


#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "model.h"

const float accelerationThreshold = 2.0; // threshold of significant in G's
const uint16_t numSamples = 128;

//buffer of datas 
uint64_t timestamp[numSamples];
float ax[numSamples];
float ay[numSamples];
float az[numSamples];
float gx[numSamples];
float gy[numSamples];
float gz[numSamples];

int samplesRead = numSamples;

// global variables used for TensorFlow Lite (Micro)
//tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// FFT setup
const float signalFrequency = 1000;
const float samplingFrequency = 5000;
const uint8_t amplitude = 100;

float vImag[numSamples];

//about keayboard and mouse 
USBMouseKeyboard key_mouse; //creating instance for keyboard + mouse

const int buttonPin = 11; //we choose the pin 11 for the external button 
                          //on the breadboard

 int buttonPreviousState = LOW;  //flag for ISR 



 //DRAFT (ISR)
  // ISR interruption in case the button is pushed/released
  //the ISR to call when the interrupt occurs; 
  //this function must take no parameters and return nothing. This function is sometimes referred to as an interrupt service routine.
  //Interrupt Service Routines (ISR)
 /*
volatile bool buttonWasPressed = false; // to avoid reptiitions 
unsigned long lastDebounceTime = 0;
*/
//used for the interruption, volatile to keep the buttonPressed during the execution 
// and after 
//const unsigned long debounceDelay = 50; // ms
//used instead of delay that can block the USB/HID. 
// delay() blocks the refraishing of the USB
// so here we base our approach on milis() 

//volatile byte lastState = LOW; // low by default
// array to map gesture index to a name





const char* GESTURES[] = {
    "left-remote-2",
    "right-remote-2",
    "rest-1",
    "shake-1"
};


#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void buttonInterrupt(); 

void setup() {
  // Configuring pins 22, 23, and 24 as outputs to power the RGB LED
  pinMode(buttonPin, INPUT_PULLDOWN); //as we have a resistance we define it as pulldown (1 if pressed)
  //attachInterrupt(digitalPinToInterrupt(buttonPin),buttonInterrupt,RISING); // ISR 


  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);
  pinMode(24, OUTPUT);
  digitalWrite(22, LOW); 
  digitalWrite(23, LOW);
  digitalWrite(24, LOW);
  delay(1000);
  digitalWrite(23, HIGH);
  digitalWrite(24, HIGH);

  Serial.begin(9600);
  unsigned long t = millis();
while (!Serial && millis() - t < 3000);
  digitalWrite(22, HIGH);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  while (!IMU.accelerationAvailable() || !IMU.gyroscopeAvailable()) {
    SERIAL_PRINTLN("Waiting for IMU to be available...");
    delay(100);
  }
  IMU.readAcceleration(ax[0], ay[0], az[0]);
  IMU.readGyroscope(gx[0], gy[0], gz[0]);
  SERIAL_PRINTLN("IMU initialized successfully!");

  #ifndef COLLECT_DATA
    // print out the rates of the IMUs
    Serial.print("Accelerometer sample rate = ");
    Serial.print(IMU.accelerationSampleRate());
    Serial.println(" Hz");
    Serial.print("Gyroscope sample rate = ");
    Serial.print(IMU.gyroscopeSampleRate());
    Serial.println(" Hz");

    Serial.println();

    // get the TFL representation of the model byte array
    tflModel = tflite::GetModel(model);
    if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
      Serial.println("Model schema mismatch!");
      while (1);
    }

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        tflModel, tflOpsResolver, tensorArena, tensorArenaSize);
    tflInterpreter = &static_interpreter;


    // Allocate memory for the model's input and output tensors
    tflInterpreter->AllocateTensors();

    // Get pointers for the model's input and output tensors
    tflInputTensor = tflInterpreter->input(0);
    tflOutputTensor = tflInterpreter->output(0);
  #else
    // Print out the different features (statistical and spectral) 
    //we want to collect for the 
    // accelerometer and the gyroscope for each coordinate (x,y,z)
      Serial.println("aX_mean,aX_stddev,aX_rms,aX_min,aX_max,aX_psdMean,aX_psdMax,aY_mean,aY_stddev,aY_rms,aY_min,aY_max,aY_psdMean,aY_psdMax,aZ_mean,aZ_stddev,aZ_rms,aZ_min,aZ_max,aZ_psdMean,aZ_psdMax,gX_mean,gX_stddev,gX_rms,gX_min,gX_max,gX_psdMean,gX_psdMax,gY_mean,gY_stddev,gY_rms,gY_min,gY_max,gY_psdMean,gY_psdMax,gZ_mean,gZ_stddev,gZ_rms,gZ_min,gZ_max,gZ_psdMean,gZ_psdMax"); 
  #endif
}

/** 
* Computes the mean value in the float array (x, y or z)
* used for the caractersitic extraction. 
* @param data: array of float of a coordinate (x,y or z)
* @param size: unsigned int corresponding on the size of the array 
*
*/
float mean(float *data, uint16_t size = numSamples) {
  float sum = 0;
  for (int i = 0; i < size; i++) sum += data[i];
  return sum / size;
}

float stddev(float *data, uint16_t size = numSamples) {
  float m = mean(data, size);
  float sum = 0;
  for (int i = 0; i < size; i++) {
    float d = data[i] - m;
    sum += d * d;
  }
  return sqrt(sum / size);
}

/** 
* Computes the max value in the float array,
* used for the caractersitic extraction. 
* @param data: array of float of a coordinate (x,y or z)
* @param size: unsigned int corresponding on the size of the array 
*
*/
float rms(float *data, uint16_t size = numSamples) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i] * data[i];
  }
  return sqrt(sum / size);
}

/** 
 * Computes the min value in the float array,
 * used for the caractersitic extraction. 
* @param data: array of float of a coordinate (x,y or z)
* @param size: unsigned int corresponding on the size of the array 
*
*/
float minVal(float *data, uint16_t size = numSamples) {
  float m = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] < m) m = data[i];
  }
  return m;
}

/** 
 * Computes the max value in the float array,
 * used for the caractersitic extraction. 
* @param data: array of float of a coordinate (x,y or z)
* @param size: unsigned int corresponding on the size of the array 
*
*/
float maxVal(float *data, uint16_t size = numSamples) {
  float m = data[0];
  for (int i = 1; i < size; i++) {
    if (data[i] > m) m = data[i];
  }
  return m;
}


// ----------- SPECTRAL PART ----------------------


/**
 * Computes the mean Power Spectral Density (PSD) value.
 * This feature represents the average spectral power over all frequency
 * components and provides an estimate of the overall energy distribution
 * of the signal in the frequency domain.
 *
 * @param data: array of float containing the PSD values of one IMU axis
 *              (x,y or z axis)
 * @param size: unsigned integer corresponding to the size of the PSD array
 * @return the mean PSD value
 */
float psdMean(float *data, uint16_t size = numSamples/2) {
  float sum = 0;
  for (int i = 1; i < size; i++) {
    sum += data[i];
  }
  // average power over the whole window
  return sum / size;
}

/**
 * Computes the maximum value of the Power Spectral Density (PSD).
 * This feature represents the highest spectral power observed in the signal
 * and is useful for identifying dominant high-energy frequency components.
 *
 * @param data: array of float containing the PSD values of one coordinate
 *              (x, y or z axis)
 * @param size: unsigned integer corresponding to the size of the PSD array
 * @return the maximum PSD value found in the array
 */
float psdMax(float *data, uint16_t size = numSamples/2) {
  float maxp = 0;
  for (int i = 1; i < size; i++) {
    if (data[i] > maxp) maxp = data[i];
  }
  return maxp;
}

/**
 * Computes the magnitude of a complex number.
 * This function is used after the FFT computation to obtain the amplitude
 * of each frequency component from its real and imaginary parts.
 *
 * @param re: real part of the complex number
 * @param im: imaginary part of the complex number
 * @return the magnitude of the complex number
 */
float complexAbs(float re, float im) {
  return sqrt(re * re + im * im);
}


/**
 * Computes the Power Spectral Density (PSD) from the FFT coefficients.
 * For each frequency bin, the PSD is obtained by squaring the magnitude
 * of the complex FFT coefficient and normalizing it by the product of
 * the number of frequency bins and the sampling frequency.
 *
 * The resulting PSD values overwrite the real-part array (dataRe).
 *
 * @param dataRe: array containing the real part of the FFT coefficients;
 *                overwritten with the computed PSD values
 * @param dataIm: array containing the imaginary part of the FFT coefficients
 * @param samplingFreq: sampling frequency of the original signal (Hz)
 * @param size: number of frequency bins considered
 */
void computePSD(float *dataRe, float *dataIm, float samplingFreq, uint16_t size = numSamples/2) {
  for(int i = 0; i < size; ++i){
    dataRe[i] = complexAbs(dataRe[i], dataIm[i]) * complexAbs(dataRe[i], dataIm[i]) / (size * samplingFreq);
  }
}

/**
 * Computes the Fast Fourier Transform (FFT) of a real-valued signal.
 * The imaginary part is first initialized to zero because the IMU data
 * are purely real samples. The FFT is then computed in the forward
 * direction and the frequency-domain coefficients overwrite the input arrays.
 *
 * @param data: array containing the time-domain samples; overwritten with
 *              the real part of the FFT coefficients
 * @param samplingFreq: sampling frequency of the signal (Hz)
 * @param size: number of samples used to compute the FFT
 */
void computeFFT(float *data,float samplingFreq ,uint16_t size = numSamples) {
  memset(vImag, 0, sizeof(vImag)); // zero out the imaginary part
  ArduinoFFT<float> FFT = ArduinoFFT<float>(data, vImag, size, samplingFreq);
  // Compute FFT
  // FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
}

/**
 * Sends a raw HID keyboard report directly to the host (Mac),
 * bypassing the keymap table. This is necessary for special keys
 * (arrows, page up/down, etc.) whose index exceeds the keymap size (152).
 * 
 * @param hidCode : the HID usage code of the key to press
 *                  (see USB HID Usage Tables spec, section 10)
 *                  e.g. 0x50 = LEFT_ARROW, 0x4F = RIGHT_ARROW, 0x4B = PAGE_UP
 */
void sendKey(uint8_t hidCode) {
    HID_REPORT report;
    report.data[0] = 0x01;  // REPORT_ID_KEYBOARD
    report.data[1] = 0;     // modifier
    report.data[2] = 0;
    report.data[3] = hidCode; // keycode HID direct
    report.data[4] = 0;
    report.data[5] = 0;
    report.data[6] = 0;
    report.data[7] = 0;
    report.data[8] = 0;
    report.length = 9;
    key_mouse.send_nb(&report);
    delay(10);
    report.data[3] = 0;  // released
    key_mouse.send_nb(&report);
}
/* (ISR -> DRAFT)
void buttonInterrupt() {
  buttonPressed = true; 
}
*/
uint32_t t0 = 0;



/**
 * Verifies at each call if the button has been pushed by the user 
 * if that is the case add a delay of 500 miliseconds and then we send 
 * the right-arrow key to the USB HID mouseKeyboard to the host. 
 * the
 * if that is not the case we simply return false. 
 * This function is called at three different levels in our program in order 
 * to get the user feedback as fast as possible at any time.. 
 * First at the start of the loop, then every one second during inferance. 
 * Then during each sample. 
 *
 * @return The button has been pushed or not by the user. 
 */
bool handleButton() {
  int buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH && buttonPreviousState == LOW ) {   
      sendKey(0x4F); // Envoie Flèche Droite
      Serial.println("Button pressed -> Right Arrow!");
      
      // Waits that the user released the button to avoid "spamming"

      delay(500); // Anti-rebund at the loosening 
      return true; 
    }
  buttonPreviousState = buttonState; 
  return false;
}

void loop() {


if (handleButton()) return; 
    //Serial.println(digitalRead(buttonPin)); // debug
   // delay(100);
#if defined(COLLECT_DATA) && !defined(COLLECT_REST) 
  while (1) {
    
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(ax[0], ay[0], az[0]);

      // sum up the absolutes
      float aSum = fabs(ax[0]) + fabs(ay[0]) + fabs(az[0]);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead = 0;
        break;
      }
      
    }
  }

#else
    float aSum = 0;
    while (1) { // every 1 second
    if (handleButton()) return;
    if(millis() - t0 > 10000){ //every 10 seconds 
      t0 = millis();
      Serial.println("Timeout reached, starting new sample...");
      break;
    }

    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(ax[0], ay[0], az[0]);
      
      // sum up the absolutes
      aSum = fabs(ax[0]) + fabs(ay[0]) + fabs(az[0]);
      // check if it's above the threshold
      if (aSum >= accelerationThreshold && millis() - t0 > 1500) {
        t0 = millis();
        break;
      }
    }
  }
#endif
  // Sampling the data from the IMU until we have enough samples to fill our buffers
while (samplesRead < numSamples) { 
  if (handleButton()) return;
      // check if new acceleration AND gyroscope data is available
      if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
        // read the acceleration and gyroscope data
        IMU.readAcceleration(ax[samplesRead], ay[samplesRead], az[samplesRead]);
        IMU.readGyroscope(gx[samplesRead], gy[samplesRead], gz[samplesRead]);
        // Save the timestamp of the sample
        timestamp[samplesRead] = millis();
        samplesRead++;
      }
  }
  samplesRead = 0;


  // Post processing the data by
  // normalizing the IMU data between 0 to 1 and calculating the mean sampling interval
  uint64_t interval = 0;
  for (int i = 0; i < numSamples; i++) {
    // Normalize acceleration data from -4g to +4g
    ax[i] = (ax[i] + 4.0) / 8.0;
    ay[i] = (ay[i] + 4.0) / 8.0;
    az[i] = (az[i] + 4.0) / 8.0;
    // Normalize gyroscope data from -2000 dps to +2000 dps
    gx[i] = (gx[i] + 2000.0) / 4000.0;
    gy[i] = (gy[i] + 2000.0) / 4000.0;
    gz[i] = (gz[i] + 2000.0) / 4000.0;
    // Calculate the mean sampling interval
    if (i > 0) {
      interval += timestamp[i] - timestamp[i - 1];
    }
  }
  double samplingFreq = (1.0E3 * (double)numSamples) / interval; // mean sampling interval in microseconds

  // Extracting features from the raw data
  float features[42];
  int k = 0;

  float* axes[6] = {ax, ay, az, gx, gy, gz};

  for(int a = 0; a < 6; a++){
    features[k++] = mean(axes[a]);
    features[k++] = stddev(axes[a]);
    features[k++] = rms(axes[a]);
    features[k++] = minVal(axes[a]);
    features[k++] = maxVal(axes[a]);
    computeFFT(axes[a], samplingFreq);
    computePSD(axes[a], vImag, samplingFreq); // Assuming a sampling frequency of 100 Hz
    //vImag here is filled via the computeFFT function in the FFT of the arduino
    features[k++] = psdMean(axes[a]);
    features[k++] = psdMax(axes[a]);
  }

#ifndef COLLECT_DATA
  // Filling the input tensor for the model
  for(int i = 0; i < 42; i++){
    tflInputTensor->data.f[i] = features[i];
  }

  // Run inference with the model on the input data
  TfLiteStatus invokeStatus = tflInterpreter->Invoke();
  if (invokeStatus != kTfLiteOk) {
    Serial.println("Invoke failed!");
    while (1);
    return;
  }

  // Loop through the output tensor values from the model
  float maxFeature = 0; 
  int indexFeature = 0; 
  for (uint8_t i = 0; i < NUM_GESTURES; i++) {
    Serial.print(GESTURES[i]);
    Serial.print(": ");
    Serial.println(tflOutputTensor->data.f[i], 6);
    if (maxFeature < tflOutputTensor->data.f[i] ) {
      maxFeature = tflOutputTensor->data.f[i]; 
      indexFeature = i; 
    }
  }
  Serial.print("Detected  : "); 
  Serial.print(GESTURES[indexFeature]); 
  Serial.println();
  
  switch(indexFeature) {
    case 0: //left-remote 
      sendKey(0x50); //left arrow
      break; 
    case 1 : //right-remote
      sendKey(0x4F); //right arrow
      break; 
    case 3 : //shake 
          sendKey(0x4A); //go to the first page, "" key
          break; 
    default: 
      break; 

  }
  // samplesRead = numSamples; //reinitisialize (avoid some bugs)
#else
  for(int i = 0; i < 42; i++){
    Serial.print(features[i], 6);
    if(i < 41) Serial.print(",");
  }
  Serial.println();
#endif

}
     