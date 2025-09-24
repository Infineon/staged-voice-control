/******************************************************************************
 * File Name: cy_lpwwd_ml.c
 *
 * Description: Implements functions for inference machine learning.
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

#ifdef ENABLE_SVC_ML_MW_SUPPORT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef COMPONENT_MEMORY_CHECK_UTILS
#include "cy_mem_check_utils.h"
#endif
#include "cy_lpwwd_error.h"
#include "cy_log.h"
#include "stdio.h"
#include "string.h"
#include "cy_lpwwd_ml.h"
#include "cy_errors.h"
#include "mtb_ml.h"
#include MTB_ML_INCLUDE_MODEL_FILE(MODEL_NAME)
#include "cy_staged_voice_control_lp.h"
#include <limits.h>


/* MTB ML Block priority if using NPU */
#ifndef MTB_ML_PRIORITY
#define MTB_ML_PRIORITY               (3)
#endif

#ifndef MTB_ML_MODEL_PROFILE
#define MTB_ML_MODEL_PROFILE (MTB_ML_PROFILE_DISABLE)
#endif

#define INFERENCE_BUF_SIZE (340)

/******************************************************
 *                      Typedefs
 ******************************************************/

extern cy_svc_lp_alloc_memory_callback_t svc_alloc_memory;
extern cy_svc_lp_free_memory_callback_t  svc_free_memory;


/******************************************************
 *                      Macros
 ******************************************************/
#if ENABLE_LPWWD_LOGS == 2
#define cy_lpwwd_ml_log_info(format,...)  printf(format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_ml_log_err(ret_val,format,...)  printf (format" [Err:0x%lx, %s:%d] \r\n",##__VA_ARGS__, ret_val,__FUNCTION__,__LINE__);
#define cy_lpwwd_ml_log_dbg(format,...)  printf (format" \r\n",##__VA_ARGS__);
#elif ENABLE_LPWWD_LOGS
#define cy_lpwwd_ml_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_ml_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,format " [Err:0x%lx, %s:%d] \r\n",##__VA_ARGS__,ret_val,__FUNCTION__,__LINE__);
#define cy_lpwwd_ml_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,format" \r\n",##__VA_ARGS__);
#else
#define cy_lpwwd_ml_log_info(format,...)
#define cy_lpwwd_ml_log_err(ret_val,format,...)
#define cy_lpwwd_ml_log_dbg(format,...)
#endif

/******************************************************
 *                      Variables
 ******************************************************/

/******************************************************
 *                      Function Prototypes
 ******************************************************/

typedef struct {
    int num_features;
    int num_feature_frames;

    LPWWD_ML_DATA_TYPE_T *feature_buffer;
} cy_lpwwd_ml_handle_t;

cy_lpwwd_ml_handle_t *ml_handle = NULL;

mtb_ml_model_bin_t model_bin = {MTB_ML_MODEL_BIN_DATA(MODEL_NAME)};
#define PROFILE_CONFIGURATION  (MTB_ML_MODEL_PROFILE)

static mtb_ml_model_t *model_obj = NULL;
/* Output/result buffers for the inference engine */
static MTB_ML_DATA_T *result_buffer = NULL;

/* Model Output Size */
static int model_output_size = 0;
float* output_score = NULL;


IFX_FE_DATA_TYPE_T* features_buffer_fixed = NULL;  /* Only used for fixed-point case */
uint16_t infer_out_size = 4;
MTB_ML_DATA_T input_reference[INFERENCE_BUF_SIZE] = {0};


cy_rslt_t cy_lpwwd_ml_init(cy_lpwwd_ml_ctrl_params_t *config_params,
        cy_lpwwd_ml_output_info_t *output_info)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int num_feature_frames = 0;

    if ((NULL == config_params) || (NULL == output_info))
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Invalid Args: %p %p",
                config_params, output_info);
        return result;
    }

    cy_lpwwd_ml_log_info("[LPWWD-ML] ML Init Prms:%p %p %d",
            config_params->ml_model_binary_buf, config_params->ml_model_meta_buf,
            config_params->num_features);

    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                (uint32_t) sizeof(cy_lpwwd_ml_handle_t), (void**) &ml_handle);

        if (NULL != ml_handle)
        {
            memset(ml_handle, 0, sizeof(cy_lpwwd_ml_handle_t));
        }
    }
    else
    {
        /* Allocate memory for ML handle */
        ml_handle = (cy_lpwwd_ml_handle_t*) calloc(1,
                sizeof(cy_lpwwd_ml_handle_t));
    }
    if (NULL == ml_handle)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Mem Alloc fail");
        cy_lpwwd_ml_deinit();
        return result;
    }

    int infer_input_byte_size = INFERENCE_BUF_SIZE;

    num_feature_frames = infer_input_byte_size / config_params->num_features;

    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_FEATURE_BUFFER,
                (uint32_t)(
                        num_feature_frames * config_params->num_features
                                * sizeof(IFX_FE_DATA_TYPE_T)),
                (void**) &features_buffer_fixed);
        if (NULL != features_buffer_fixed)
        {
            memset(features_buffer_fixed, 0,
                    (uint32_t)(
                            num_feature_frames * config_params->num_features
                                    * sizeof(IFX_FE_DATA_TYPE_T)));
        }
    }
    else
    {
        features_buffer_fixed = (IFX_FE_DATA_TYPE_T*) calloc(1,
                num_feature_frames * config_params->num_features
                        * sizeof(IFX_FE_DATA_TYPE_T));
    }

    if (features_buffer_fixed == NULL)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] MemAlloc fail");
        cy_lpwwd_ml_deinit();
        return result;
    }

    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                (uint32_t)(infer_out_size * sizeof(float)),
                (void**) &output_score);
        if (NULL != output_score)
        {
            memset(output_score, 0, infer_out_size * sizeof(float));
        }
    }
    else
    {
        // allocate memory for output score
        output_score = (float*) calloc(1, infer_out_size * sizeof(float)); //probability is in float type
    }
    if (output_score == NULL)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] MemAlloc fail");
        cy_lpwwd_ml_deinit();
        return result;
    }

#ifndef DISABLE_SVC_ML_INIT
    /* Initialize the ModusToolbox ML middleware */
    mtb_ml_init(MTB_ML_PRIORITY);
#endif
    /* Initialize the neural network */
    result = mtb_ml_model_init(&model_bin,
                               NULL,
                               &model_obj);
    if (CY_RSLT_SUCCESS != result)
    {
        printf("MTB ML initialization failure: %lu\r\n", (unsigned long) result);
        return result;
    }

    mtb_ml_model_profile_config(model_obj, PROFILE_CONFIGURATION);

    mtb_ml_model_get_output(model_obj, &result_buffer, &model_output_size);

#ifdef ENABLE_LPWWD_LOGS
    /* Print information about the model */
    mtb_ml_utils_print_model_info(model_obj);
#endif

    // Allocate memory for feature buffer
    ml_handle->num_features = config_params->num_features;
    ml_handle->num_feature_frames = num_feature_frames;

    output_info->number_of_feature_frames = ml_handle->num_feature_frames;
    output_info->number_of_tokens = infer_out_size;

    cy_lpwwd_ml_log_info(
            "[LPWWD-ML] Init[Hl:%p FF[%d,%d],modelout:%d",
            ml_handle,
            ml_handle->num_features,
            ml_handle->num_feature_frames,
            model_output_size);

    cy_lpwwd_ml_log_info("[LPWWD-ML] ML Init TFliteU Success");

    return result;
}

cy_rslt_t cy_lpwwd_ml_feed(LPWWD_ML_IP_DATA_TYPE_T *feature_frame)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int num_features = 0, num_feature_frames = 0;

    if(NULL == ml_handle)
    {
        result = CY_RSLT_LPWWD_ML_NOT_INTIAILIZED;
        cy_lpwwd_ml_log_err(result, " not intialized");
        return result;
    }

    if (NULL == feature_frame)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_ml_log_err(result, "Inv Arg: %p ", feature_frame);
        return result;
    }

    num_features = ml_handle->num_features;
    num_feature_frames = ml_handle->num_feature_frames;

    for (int i = 0; i < num_features; i++)
    {
        // stack features into 1D array where i'th feature from all frames is grouped together with others
        uint32_t offset = i * num_feature_frames;

        for (int j = 0; j < num_feature_frames - 1; j++)
        {
            features_buffer_fixed[offset + j] = features_buffer_fixed[offset + j
                    + 1];
        }
        features_buffer_fixed[offset + num_feature_frames - 1] =
                feature_frame[i];
    }

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_FRAME_PRINT)
    printf_feature_buffer_matrix("FeatureBufferComplete:", (unsigned char*) features_buffer_fixed,
        (unsigned int) ml_handle->num_features,
        (unsigned int) ml_handle->num_feature_frames);
#endif

    return result;
}

static cy_rslt_t cy_lpwwd_mtb_ml_utils_convert_int8_to_int8(
        const int8_t* in, int8_t *out, int size, float scale, int zero_point)
{
    int loop_count;
    float val;

    /* Sanity check of input parameters */
    if (in == NULL || out == NULL || size <= 0)
    {
        return MTB_ML_RESULT_BAD_ARG;
    }

    loop_count = size;
    while (loop_count > 0)
    {
        val = ((float)(float)*in++ / scale) + zero_point;
        //val += val > 0.0f ? 0.5f : -0.5f;
        if ((int32_t) val > SCHAR_MAX)
            *out++ = SCHAR_MAX;
        else if ((int32_t) val < SCHAR_MIN)
            *out++ = SCHAR_MIN;
        else
            *out++ = (int8_t) (val);

        loop_count--;
    }
    return CY_RSLT_SUCCESS;
}

static cy_rslt_t cy_lpwwd_mtb_ml_utils_model_quantize_int8(
        const mtb_ml_model_t *obj, const int8_t* input_data, MTB_ML_DATA_T* quantized_values)
{
    if (obj == NULL || input_data == NULL || quantized_values == NULL) {
        return MTB_ML_RESULT_BAD_ARG;
    }

    int32_t size = obj->input_size;
    const int8_t *value = input_data;
    return cy_lpwwd_mtb_ml_utils_convert_int8_to_int8(value, quantized_values, size, obj->input_scale, obj->input_zero_point);
}



cy_rslt_t cy_lpwwd_ml_generate_output_score(
        cy_lpwwd_output_score *poutput_score)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(NULL == ml_handle)
    {
        result = CY_RSLT_LPWWD_ML_NOT_INTIAILIZED;
        cy_lpwwd_ml_log_err(result, " not intialized");
        return result;
    }

    if(NULL == poutput_score)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_ml_log_err(result, "Inv Arg: %p ", poutput_score);
        return result;
    }

    cy_lpwwd_mtb_ml_utils_model_quantize_int8(model_obj,
            (const int8_t*) features_buffer_fixed,
            (MTB_ML_DATA_T*) input_reference);

    mtb_ml_model_run(model_obj, input_reference);

    mtb_ml_utils_model_dequantize(model_obj,output_score);

    for (int j = 0; j < infer_out_size; ++j)
    {
        poutput_score->output_score_buffer[j] = (int16_t) (output_score[j]
                * 32767 + 0.5f);
    }

#if (ENABLE_LPWWD_DEBUG & 0x08)
    cy_lpwwd_ml_log_info("ML-OUT: %e %e %e %e",
            output_score[0],
            output_score[1],
            output_score[2],
            output_score[3] );
#endif

    return result;
}

cy_rslt_t cy_lpwwd_ml_deinit( )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    if(NULL == ml_handle)
    {
        result = CY_RSLT_LPWWD_ML_NOT_INTIAILIZED;
        cy_lpwwd_ml_log_err(result, "ML not initialized");
        return result;
    }

    if (NULL != model_obj)
    {
        mtb_ml_model_deinit(model_obj);
        model_obj = NULL;
    }

    if (NULL != features_buffer_fixed)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_FEATURE_BUFFER,
                    features_buffer_fixed);
        }
        else
        {
            free(features_buffer_fixed);
        }
        features_buffer_fixed = NULL;
    }
    else
    {
        cy_lpwwd_ml_log_info("FFF buffer not created");
    }

    if (NULL != output_score)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY, output_score);
        }
        else
        {
            free(output_score);
        }
        output_score = NULL;
    }
    else
    {
        cy_lpwwd_ml_log_info("OS buffer not created");
    }

    if (NULL != ml_handle)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY, ml_handle);
        }
        else
        {
            free(ml_handle);
        }
        ml_handle = NULL;
        cy_lpwwd_ml_log_info("ML Hdl destroyed");
    }
    else
    {
        cy_lpwwd_ml_log_info("ML Hdl not created");
    }

    return result;
}

int8_t* cy_lpwwd_ml_get_feature_matrix_dbg()
{
    if (NULL == ml_handle)
    {
        return NULL;
    }
    return (int8_t*) features_buffer_fixed;
}
#endif
/* [] END OF FILE */
