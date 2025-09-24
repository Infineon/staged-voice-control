// This file is generated. Do not edit.
// Generated on: 10.01.2025 08:47:26
// Build version: 3.0.0.13974

#ifndef HEY_GOOGLE_INT8_GEN_H
#define HEY_GOOGLE_INT8_GEN_H

#include "tensorflow/lite/c/common.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define HEY_GOOGLE_INT8_MODEL_CONST_DATA_SIZE 131426
#define HEY_GOOGLE_INT8_MODEL_INIT_DATA_SIZE 576
#define HEY_GOOGLE_INT8_MODEL_UNINIT_DATA_SIZE 1396


// Sets up the model with init and prepare steps.
TfLiteStatus HEY_GOOGLE_INT8_init();
// Returns the input tensor with the given index.
TfLiteTensor *HEY_GOOGLE_INT8_input(int index);
// Returns the output tensor with the given index.
TfLiteTensor *HEY_GOOGLE_INT8_output(int index);
// Runs inference for the model.
TfLiteStatus HEY_GOOGLE_INT8_invoke();
// Resets all variable tensors
TfLiteStatus HEY_GOOGLE_INT8_reset();

// Returns the number of input tensors.
size_t HEY_GOOGLE_INT8_inputs();

// Returns the number of output tensors.
size_t HEY_GOOGLE_INT8_outputs();

// Return the buffer pointer of input tensor
void *HEY_GOOGLE_INT8_input_ptr(int index);

// Return the buffer size of input tensor
size_t HEY_GOOGLE_INT8_input_size(int index);

// Return the dimention size of input tensor
int HEY_GOOGLE_INT8_input_dims_len(int index);

// Return the dimention buffer pointer of input tensor
int *HEY_GOOGLE_INT8_input_dims(int index);

// Return the buffer pointer of output tensor
void *HEY_GOOGLE_INT8_output_ptr(int index);

// Return the buffer size of output tensor
size_t HEY_GOOGLE_INT8_output_size(int index);

// Return the dimention size of output tensor
int HEY_GOOGLE_INT8_output_dims_len(int index);

// Return the dimention buffer pointer of output tensor
int *HEY_GOOGLE_INT8_output_dims(int index);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif
