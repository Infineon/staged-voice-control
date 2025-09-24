/******************************************************************************
 * File Name: cy_lpwwd_ml.h
 *
 * Description: Header file with APIs & Data structures for wake word detection.
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
#ifndef __CY_LPWWD_ML_H
#define __CY_LPWWD_ML_H

#include "cy_result.h"
#include "cy_lpwwd_common.h"
/******************************************************
 *                      Typedefs
 ******************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RUN_FLOAT_FE
typedef float  LPWWD_ML_IP_DATA_TYPE_T;
typedef float  LPWWD_ML_OP_DATA_TYPE_T;
#else
typedef int8_t LPWWD_ML_IP_DATA_TYPE_T;
typedef int16_t LPWWD_ML_OP_DATA_TYPE_T;
#endif

/**
 * low power wake word inference engine configuration parameters
 */
typedef struct {
	int16_t num_features;
	const char *ml_model_meta_buf;
	const char *ml_model_binary_buf;
    /**
     * Feature scale for the model
     */
    float feature_scale;

    /**
     * Feature offset for the model
     */
    int16_t feature_offset;

} cy_lpwwd_ml_ctrl_params_t;

typedef struct {
    uint16_t number_of_feature_frames;
    uint16_t number_of_tokens;
} cy_lpwwd_ml_output_info_t;


/* Output score buffer */
typedef struct {
	LPWWD_ML_OP_DATA_TYPE_T *output_score_buffer;
	uint16_t output_score_buffer_size;
} cy_lpwwd_output_score;

/******************************************************
 *                      Function Prototypes
 ******************************************************/
/**
 * Initialized pre wake-word components.
 *
 * @param[in] config_params  Configuration parameters needed for the ML components
 *
 * @param[out] output_info   Output information such as number of tokens & number of features frame required for the ML model
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_ml_init(cy_lpwwd_ml_ctrl_params_t *config_params,
        cy_lpwwd_ml_output_info_t *output_info);

/**
 * Feed generated feature frame to ML component
 *
 * @param[in] feature_frame  Pass generated feature frame from Pre wake-word component
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_ml_feed(LPWWD_ML_IP_DATA_TYPE_T *feature_frame);

/**
 * Feed the audio data to input pre wake-word components.
 *
 * @param[out] output_score               Pointer to generated output score. number_of_classes * ML data type
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_ml_generate_output_score( cy_lpwwd_output_score *output_score);

/**
 * De-Initialized pre wake-word components.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_ml_deinit( );


int8_t* cy_lpwwd_ml_get_feature_matrix_dbg( );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /*__CY_LPWWD_ML_H */


/* [] END OF FILE */
