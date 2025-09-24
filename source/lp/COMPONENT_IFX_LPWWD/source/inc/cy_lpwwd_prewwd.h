/******************************************************************************
 * File Name: cy_lpwwd_prewwd.h
 *
 * Description: Header file with APIs & Data structures for pre wake-word.
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
#ifndef __CY_LPWWD_PREWWD_H
#define __CY_LPWWD_PREWWD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cy_log.h"
#include "cy_result.h"
#include "cy_lpwwd_common.h"
#include "cy_lpwwd.h"
/******************************************************
 *                      Typedefs
 ******************************************************/

typedef void *cy_lpwwd_prewwd_context_t;

#ifdef ENABLE_IFX_INFERENCE
typedef CY_ML_DATA_TYPE_T  LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T;
#else
typedef IFX_FE_DATA_TYPE_T LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T;
#endif

/* Output score buffer */
typedef struct {
	LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T *feature_frame_buffer;
	uint16_t feature_frame_buffer_size;
} cy_lpwwd_feature_buffer;

/******************************************************
 *                      Function Prototypes
 ******************************************************/

/**
 * Initialized pre wake-word components.
 *
 * @param[in] config_params       Configuration parameters needed for the pre wake word components
 *
 * @param[out] num_of_features    Number of features required for pre wake-word
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_prewwd_init(cy_lpwwd_prewwd_config_params_t *config_params,
       uint16_t *num_of_features);

/**
 * Feed the audio data to input pre wake-word components.
 *
 * @param[in] input_data               Feed audio data (10ms frame)
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_prewwd_feed( int16_t *input_data);

/**
 * Feed the audio data to input pre wake-word components.
 *
 * @param[out] feature_buffer          Generated feature frame
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_prewwd_generate_feature_frame( cy_lpwwd_feature_buffer *feature_buffer);

/**
 * Reset internal input audio buffer
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_reset_audio_buffer( );

/**
 * De-Initialized pre wake-word components.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_prewwd_deinit( );

#endif /*__CY_LPWWD_PREWWD_H */

/* [] END OF FILE */
