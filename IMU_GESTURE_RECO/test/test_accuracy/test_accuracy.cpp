/*
  IMU Classifier
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

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>

#include "../../../scratch/test_data.h"
#include "../../../scratch/model.h"
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

float predictions[num_samples][num_classes] = {0.0}; // array to store the model predictions for each sample


// array to map gesture index to a name
const char* GESTURES[] = {
    "circle-1",
    "circle-2",
    "rest-1",
    "shake-1",
    "up-down-1"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void test_accuracy() {
  for(int sample = 0; sample < num_samples; sample++){
    for(int i = 0; i < 42; i++){
      tflInputTensor->data.f[i] = test_inputs[sample][i];
    }

    // Run inference with the model on the input data
    TfLiteStatus invokeStatus = tflInterpreter->Invoke();
    if (invokeStatus != kTfLiteOk) {
      Serial.println("Invoke failed!");
      while (1);
      return;
    }

    // Loop through the output tensor values from the model
    float max = 0.0;
    int max_index = 0;
    for (uint8_t i = 0; i < NUM_GESTURES; i++) {
      // Serial.print(tflOutputTensor->data.f[i], 6);
      // Serial.print(",");
      predictions[sample][i] = tflOutputTensor->data.f[i];
      if(predictions[sample][i] > max){
        max = predictions[sample][i];
        max_index = i;
      }
    }
    if(test_outputs[sample][max_index] == 1){
      Serial.print("Test sample");
      Serial.print(sample);
      Serial.println("\t OK");
    } else {
      Serial.print("Test sample");
      Serial.print(sample);
      Serial.println("\t FAILED");
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  delay(2000);

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

  // Create an interpreter to run the model
  //tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
  
  test_accuracy();
  
}

void loop() {
  
}