/******************************************************************************
 * File Name: cy_lpwwd_common.h
 *
 * Description: Common header file for low power wake word detection.
 *
 *******************************************************************************
 * (c) 2021, Infineon Technologies Company. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Infineon Technologies Company (Infineon) or one of its
 * subsidiaries and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Infineon hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Infineon's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Infineon.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Infineon
 * reserves the right to make changes to the Software without notice. Infineon
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Infineon does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Infineon product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Infineon's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Infineon against all liability.
 *******************************************************************************/
#ifndef __CY_LPWWD_COMMON_H
#define __CY_LPWWD_COMMON_H

/******************************************************
 *                      Typedefs
 ******************************************************/


//#include "cy_ml_accuracy.h"

#ifdef ENABLE_IFX_INFERENCE
#include "cy_ml_inference.h"
#else
#include "ifx_sp_common.h"
#endif

#ifdef ENABLE_IFX_INFERENCE
typedef CY_ML_DATA_TYPE_T  LPWWD_ML_DATA_TYPE_T;
typedef CY_ML_DATA_TYPE_T LPWWD_OUTSCR_DATA_TYPE_T;
#else
typedef IFX_FE_DATA_TYPE_T LPWWD_ML_DATA_TYPE_T;
typedef int16_t LPWWD_OUTSCR_DATA_TYPE_T;
#endif


/**
 * Pre wake-word configuration parameters
 */
typedef struct {
    const char *hmm_model1_keyword_binary_buf;
    const char *hmm_model1_garbage_binary_buf;
    const char *hmm_model1_noise_binary_buf;
    const char *hmm_model2_keyword_binary_buf;
    const char *hmm_model2_garbage_binary_buf;
    const char *hmm_model2_noise_binary_buf;
    int sampling_rate; // Sampling rate
    int number_of_tokens; // Number of classes for post processing
    int frame_rate; // frame rate per second
    int lookback_buffer_length; // loockback buffer length, in samples
    int stacked_frame_delay; // NN stacked frame delay, in samples
    int detection_threshold; // upper 16bit is detection_threshold, lower 16bit is set (update) flag
} cy_lpwwd_postwwd_config_params_t;

/**
 * Pre wake-word configuration parameters
 */
typedef struct {
    int sampling_rate; // Sampling rate
    int frame_size;    // audio frame size for feature extraction
    int frame_shift;   // frame shift
    int number_of_filter_banks; // number of filter banks
    int number_of_dct_coefficients; // number of coefficients
    int audio_input_frame_size; // incoming audio frame size

} cy_lpwwd_prewwd_config_params_t;

/**
 * low power wake word inference engine configuration parameters
 */
typedef struct {
    const char *ml_model_meta_buf;
    const char *ml_model_binary_buf;
} cy_lpwwd_ml_config_params_t;


/******************************************************
 *                      Function Prototypes
 ******************************************************/
#endif /*__CY_LPWWD_COMMON_H */

/* [] END OF FILE */
