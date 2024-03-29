/**
 * Test sinewave neural network model
 * 
 * Author: Pete Warden
 * Modified by: Shawn Hymel
 * Date: March 11, 2020
 * 
 * Copyright 2019 The TensorFlow Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Import TensorFlow stuff
#include "TensorFlowLite.h"
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

// Our model
#include "quant_model.h"

// Figure out what's going on in our model
#define DEBUG 1


// TFLite globals, used for compatibility with Arduino-style sketches
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  TfLiteTensor* model_output = nullptr;

  // Create an area of memory to use for input, output, and other TensorFlow
  // arrays. You'll need to adjust this by combiling, running, and looking
  // for errors.
  constexpr int tensor_arena_size = 5 * 1024;
  uint8_t tensor_arena[tensor_arena_size];
} // namespace

void setup() {

  // Wait for Serial to connect
#if DEBUG
  while(!Serial);
#endif

  // Set up logging (will report to Serial, even within TFLite functions)
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;

  // Map the model into a usable data structure
  model = tflite::GetModel(quant_8_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report("Model version does not match Schema");
    while(1);
  }

  // Pull in only needed operations (should match NN layers)
  // Available ops:
  //  https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/micro/kernels/micro_ops.h
  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
  micro_mutable_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_FULLY_CONNECTED,
    tflite::ops::micro::Register_FULLY_CONNECTED(),
    1, 3);

  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());

  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_AVERAGE_POOL_2D, tflite::ops::micro::Register_AVERAGE_POOL_2D());

  // Build an interpreter to run the model
  static tflite::MicroInterpreter static_interpreter(
    model, micro_mutable_op_resolver, tensor_arena, tensor_arena_size,
    error_reporter);
  interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    error_reporter->Report("AllocateTensors() failed");
    while(1);
  }

  // Assign model input and output buffers (tensors) to pointers
  model_input = interpreter->input(0);
  model_output = interpreter->output(0);

  // Get information about the memory area to use for the model's input
  // Supported data types:
  // https://github.com/tensorflow/tensorflow/blob/master/tensorflow/lite/c/common.h#L226
#if DEBUG
  Serial.print("Number of dimensions: ");
  Serial.println(model_input->dims->size);
  Serial.print("Dim 1 size: ");
  Serial.println(model_input->dims->data[0]);
  Serial.print("Dim 2 size: ");
  Serial.println(model_input->dims->data[1]);
  Serial.print("Input type: ");
  Serial.println(model_input->type);
#endif
}

void loop() {

#if DEBUG
  unsigned long start_timestamp = micros();
#endif

//  // Get current timestamp and modulo with period
//  unsigned long timestamp = micros();
//  timestamp = timestamp % (unsigned long)period;
//
//  // Calculate x value to feed to the model
//  float x_val = ((float)timestamp * 2 * pi) / period;
//
//  // Copy value to input buffer (tensor)
//  model_input->data.f[0] = x_val;
//
//  // Run inference
//  TfLiteStatus invoke_status = interpreter->Invoke();
//  if (invoke_status != kTfLiteOk) {
//    error_reporter->Report("Invoke failed on input: %f\n", x_val);
//  }
//
//  // Read predicted y value from output buffer (tensor)
//  float y_val = model_output->data.f[0];
//
//  // Translate to a PWM LED brightness
//  int brightness = (int)(255 * y_val);
//  analogWrite(led_pin, brightness);
//
//  // Print value
//  Serial.println(y_val);

#if DEBUG
  Serial.print("Time for inference (us): ");
  Serial.println(micros() - start_timestamp);
#endif
}
