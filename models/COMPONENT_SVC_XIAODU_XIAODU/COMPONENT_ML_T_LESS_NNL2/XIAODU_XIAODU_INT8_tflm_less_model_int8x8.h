// This file is generated. Do not edit.
// Generated on: 31.03.2026 09:44:00
// Build version: 3.2.0.16398

#ifndef XIAODU_XIAODU_INT8_GEN_H
#define XIAODU_XIAODU_INT8_GEN_H

#include "tensorflow/lite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define XIAODU_XIAODU_INT8_MODEL_CONST_DATA_SIZE 131442
#define XIAODU_XIAODU_INT8_MODEL_INIT_DATA_SIZE 4708
#define XIAODU_XIAODU_INT8_MODEL_UNINIT_DATA_SIZE 1396


// Sets up the model with init and prepare steps.
TfLiteStatus XIAODU_XIAODU_INT8_init();
// Returns the input tensor with the given index.
TfLiteTensor *XIAODU_XIAODU_INT8_input(int index);
// Returns the output tensor with the given index.
TfLiteTensor *XIAODU_XIAODU_INT8_output(int index);
// Runs inference for the model.
TfLiteStatus XIAODU_XIAODU_INT8_invoke();
// Resets all variable tensors
TfLiteStatus XIAODU_XIAODU_INT8_reset();

// Returns the number of input tensors.
size_t XIAODU_XIAODU_INT8_inputs();

// Returns the number of output tensors.
size_t XIAODU_XIAODU_INT8_outputs();

// Return the buffer pointer of input tensor
void *XIAODU_XIAODU_INT8_input_ptr(int index);

// Return the buffer size of input tensor
size_t XIAODU_XIAODU_INT8_input_size(int index);

// Return the dimention size of input tensor
int XIAODU_XIAODU_INT8_input_dims_len(int index);

// Return the dimention buffer pointer of input tensor
int *XIAODU_XIAODU_INT8_input_dims(int index);

// Return the buffer pointer of output tensor
void *XIAODU_XIAODU_INT8_output_ptr(int index);

// Return the buffer size of output tensor
size_t XIAODU_XIAODU_INT8_output_size(int index);

// Return the dimention size of output tensor
int XIAODU_XIAODU_INT8_output_dims_len(int index);

// Return the dimention buffer pointer of output tensor
int *XIAODU_XIAODU_INT8_output_dims(int index);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif
