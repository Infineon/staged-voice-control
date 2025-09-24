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

#ifndef ENABLE_SVC_ML_MW_SUPPORT
#ifndef ENABLE_ML_STUB
#ifndef ENABLE_IFX_INFERENCE
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

#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/memory_helpers.h"
#include "tensorflow/lite/micro/tflite_bridge/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_utils.h"
#include "tensorflow/lite/schema/schema_generated.h"

 /* TODO: This param needs to be tweaked based on model, Right now fixing
  * the working number based on experiementation for all models */
//#define SVC_DEF_SUFFICIENT_ARENA_SIZE   (2*1024*1024)
//#define SVC_DEF_SUFFICIENT_ARENA_SIZE   (20*1024)
//#ifndef SVC_DEF_SUFFICIENT_ARENA_SIZE
//#define SVC_DEF_SUFFICIENT_ARENA_SIZE   ((6*1024)+1024)
//#endif

extern const unsigned int SVC_MODEL_ARENA_BUFFER_SIZE;

#include "cy_staged_voice_control_lp.h"
extern cy_svc_lp_alloc_memory_callback_t svc_alloc_memory;
extern cy_svc_lp_free_memory_callback_t  svc_free_memory;

#define ARENA_SIZE_QUANTIZATION (1024)
#define ARENA_SIZE_MIN (16 * ARENA_SIZE_QUANTIZATION)

//uint8_t tensor_arena[SVC_DEF_SUFFICIENT_ARENA_SIZE] = {0};
uint8_t *tensor_arena = NULL;
IFX_FE_DATA_TYPE_T* features_buffer_fixed = NULL;  /* Only used for fixed-point case */

uint16_t infer_out_size = 4;
float feature_scale = 0;
int16_t feature_offset = 0;

/******************************************************
 *                      Typedefs
 ******************************************************/

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

//#define PRINT_ML_FEED_ARRAY
//#define SIMULATE_ML_FEED_ARRAY
#ifdef PRINT_ML_FEED_ARRAY
#define ML_FEED_COUNTER_MATCH (1)
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
    /* Buffer for holding features */
    void *tflm_obj; /**< pointer of Tflite-micro runtime object */
    LPWWD_ML_DATA_TYPE_T *feature_buffer;
} cy_lpwwd_ml_handle_t;

cy_lpwwd_ml_handle_t *ml_handle = NULL;


tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;

static float factor;
float* output_score = NULL;

#define FETCH_ELEMENT(my_type)                                \
    do {                                                      \
      my_type* data = tflite::GetTensorData<my_type>(output); \
      *val = data[idx];                                       \
      if (data[idx] > data[*best_idx])                        \
        *best_idx = idx;                                      \
      return true;                                            \
    } while (0);


static bool get_element(tflite::MicroErrorReporter* micro_error_reporter, TfLiteTensor* output, int idx, int* best_idx, double* val)
{
    switch (output->type)
    {
    case kTfLiteFloat32:
        FETCH_ELEMENT(float);
        break;
    case kTfLiteInt8:
        FETCH_ELEMENT(int8_t);
        break;
    case kTfLiteInt16:
        FETCH_ELEMENT(int16_t);
        break;
    case kTfLiteInt32:
        FETCH_ELEMENT(int32_t);
        break;
    case kTfLiteUInt8:
        FETCH_ELEMENT(uint8_t);
        break;
        // Note: There is no UInt16
    case kTfLiteUInt32:
        FETCH_ELEMENT(uint32_t);
        break;
        // There are a number of other types that we don't expect to ever use. If one happens, report an error.
    default:
        TF_LITE_REPORT_ERROR(micro_error_reporter, "Unhandled Tensor output type: %d\n", output->type);
        return false;
        break;
    }
}

static float dequantize(float value, TfLiteAffineQuantization* aq)
{
    float result;
    tflite::Dequantize(&value, 1, aq->scale->data[aq->quantized_dimension], aq->zero_point->data[aq->quantized_dimension], &result);
    return result;
}

static float dequantize(tflite::MicroErrorReporter micro_error_reporter, float value, TfLiteQuantization* quantization)
{
    switch (quantization->type)
    {
    case kTfLiteNoQuantization:
    {
        return value;
    }
    case kTfLitePackedAffineQuantization:
    {
        TfLitePackedAffineQuantization* paq = (TfLitePackedAffineQuantization*)(quantization->params);
        return dequantize(value, &(paq->affine));
    }
    case kTfLiteAffineQuantization:
    {
        TfLiteAffineQuantization* aq = (TfLiteAffineQuantization*)(quantization->params);
        return dequantize(value, aq);
    }
    default:
    {
        TF_LITE_REPORT_ERROR(&micro_error_reporter, "Unhandled quantization type.\n");
        return value;
    }
    }
}

static int get_zp_and_scale_for_input(tflite::MicroInterpreter *pinterpreter,float *scale, int16_t *zero_point)
{
    TfLiteTensor *tfinput = pinterpreter->input(0);
    TfLiteQuantization *tfquantization = &tfinput->quantization;

    *scale = 0;
    *zero_point = 0;

    if(tfquantization->type == kTfLiteAffineQuantization)
    {
    	TfLiteAffineQuantization *taffinequant =
    			(TfLiteAffineQuantization *)tfquantization->params;

		*scale = taffinequant->scale->data[taffinequant->quantized_dimension];
		*zero_point = (int16_t)taffinequant->zero_point->data[taffinequant->quantized_dimension];
    }
    return 0;
}

cy_rslt_t cy_lpwwd_ml_init(cy_lpwwd_ml_ctrl_params_t *config_params,
        cy_lpwwd_ml_output_info_t *output_info)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    const void *model_data = NULL;
    tflite::MicroInterpreter *pinterpreter = NULL;
    int num_feature_frames = 0;

    if ((NULL == config_params) || (NULL == output_info))
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Invalid Args: %p %p",
                config_params, output_info);
        return result;
    }

    if (NULL == config_params->ml_model_binary_buf)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Invalid Mdl: %p",
                config_params->ml_model_binary_buf);
        return result;
    }

    model_data = (const void*) config_params->ml_model_binary_buf;

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

    const tflite::Model *model = ::tflite::GetModel(model_data);

    if (model->version() != TFLITE_SCHEMA_VERSION)
    {
        result = CY_RSLT_LPWWD_ML_TFLITE_VER_MISMATCH;

        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Model provided is schema version %d not equal "
                        "to supported version %d.\n", model->version(),
                TFLITE_SCHEMA_VERSION);

        cy_lpwwd_ml_log_err(result,
                "[LPWWD-ML] TFliteU version mismatch, expected:%d, Cur:%ld",
                TFLITE_SCHEMA_VERSION, model->version());
        cy_lpwwd_ml_deinit();
        return result;
    }

    cy_lpwwd_ml_log_info("TFLite Vesrion:%ld", model->version());

    cy_lpwwd_ml_log_info("Arena Buffer Size: %d",SVC_MODEL_ARENA_BUFFER_SIZE);

    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_ML_ARENA_BUFFER,
                (uint32_t) SVC_MODEL_ARENA_BUFFER_SIZE, (void**) &tensor_arena);
        if (NULL != tensor_arena)
        {
            memset(tensor_arena, 0, SVC_MODEL_ARENA_BUFFER_SIZE);
        }
    }
    else
    {
        tensor_arena = (uint8_t*) calloc(1, SVC_MODEL_ARENA_BUFFER_SIZE);
    }
    if(NULL == tensor_arena)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Tensor buff allocation failed\n");
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Tensor buf alloc fail");
        cy_lpwwd_ml_deinit();
        return result;
    }

    pinterpreter = new tflite::MicroInterpreter(model, resolver, tensor_arena,
    		SVC_MODEL_ARENA_BUFFER_SIZE);

    ml_handle->tflm_obj = reinterpret_cast<void*>(pinterpreter);

    TfLiteStatus allocate_status = pinterpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Tensor allocation failed\n");
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] Tensor alloc fail");
        cy_lpwwd_ml_deinit();
        return result;
    }

    /* check model correctness */
    if (pinterpreter->inputs_size() != 1 || pinterpreter->outputs_size() != 1)
    {/* LPWWD only use sequential NN model */
        result = CY_RSLT_LPWWD_ML_INVALID_MODEL;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Only sequential NN model should be used in LPWWD\n");
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] InvModel: IPSz:%d OPSz:%d",
                pinterpreter->inputs_size(), pinterpreter->outputs_size());
        cy_lpwwd_ml_deinit();
        return result;
    }

    if (pinterpreter->input(0)->type != kTfLiteFloat32
            && pinterpreter->input(0)->type != kTfLiteInt8)
    {/* only use either float or 8-bit integer */
        result = CY_RSLT_LPWWD_ML_INVALID_MODEL;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Input data type is neither float32 nor int8 in LPWWD\n");
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] IPType:%d",
                pinterpreter->input(0)->type);
        cy_lpwwd_ml_deinit();
        return result;
    }

    if (pinterpreter->output(0)->type != kTfLiteFloat32
            && pinterpreter->output(0)->type != kTfLiteInt8)
    {/* only use either float or 8-bit integer */
        result = CY_RSLT_LPWWD_ML_INVALID_MODEL;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Output data type is neither float32 nor int8 in LPWWD\n");
        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] OPType:%d",
                pinterpreter->output(0)->type);
        cy_lpwwd_ml_deinit();
        return result;
    }

    int infer_input_byte_size = pinterpreter->input(0)->bytes;

    if (infer_out_size
            != tflite::ElementCount(*(pinterpreter->output(0)->dims)))
    {
        result = CY_RSLT_LPWWD_ML_INVALID_MODEL;
        TF_LITE_REPORT_ERROR(&micro_error_reporter,
                "Output data elemenets should be equal to 4 in LPWWD\n");

        cy_lpwwd_ml_log_err(result, "[LPWWD-ML] InferSize mismatch:%d",
                tflite::ElementCount(*(pinterpreter->output(0)->dims)));
        cy_lpwwd_ml_deinit();
        return result;
    }
    num_feature_frames = infer_input_byte_size / config_params->num_features;
    if (pinterpreter->input(0)->type == kTfLiteFloat32)
        num_feature_frames = num_feature_frames / sizeof(float);

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

    // Allocate memory for feature buffer
    ml_handle->num_features = config_params->num_features;
    ml_handle->num_feature_frames = num_feature_frames;

    output_info->number_of_feature_frames = ml_handle->num_feature_frames;
    output_info->number_of_tokens = infer_out_size;

	(void)get_zp_and_scale_for_input(pinterpreter,&feature_scale,&feature_offset);
	factor = 1.0f / feature_scale;
	cy_lpwwd_ml_log_info("From_Model: Factor:%e, FeatureScale:%e FeatureOffset:%d",factor,feature_scale,feature_offset);

    if((0 != config_params->feature_scale) &&
       (0 != config_params->feature_offset))
    {
		feature_scale = config_params->feature_scale;
		feature_offset = config_params->feature_offset;
		factor = 1.0f / feature_scale;

    	cy_lpwwd_ml_log_info("From_App(overwritten): Factor:%e, FeatureScale:%e FeatureOffset:%d",factor,feature_scale,feature_offset);
    }

    cy_lpwwd_ml_log_info(
            "[LPWWD-ML] Init[Hl:%p ItrPr[V:%ld,Sq[%d,%d],Typ[%d,%d],Inf[%d,%d],Are:%d,FF[%d,%d]",
            ml_handle,
            model->version(),
            pinterpreter->inputs_size(), pinterpreter->outputs_size(),
            pinterpreter->input(0)->type,pinterpreter->output(0)->type,
            pinterpreter->input(0)->bytes,
            tflite::ElementCount(*(pinterpreter->output(0)->dims)),
            pinterpreter->arena_used_bytes(),
            ml_handle->num_features,
            ml_handle->num_feature_frames);

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

cy_rslt_t cy_lpwwd_ml_generate_output_score(
        cy_lpwwd_output_score *poutput_score)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    tflite::MicroInterpreter *pinterpreter = NULL;

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

    pinterpreter =
            reinterpret_cast<tflite::MicroInterpreter*>(ml_handle->tflm_obj);


#ifdef PRINT_ML_FEED_ARRAY
    static int ml_feed_counter = 0;
    ml_feed_counter++;

    if(ML_FEED_COUNTER_MATCH)
    {
        printf("\r\nMLInput[%d]:",ml_feed_counter);
	}

#endif

    for (size_t i = 0; i <  pinterpreter->inputs_size(); ++i)
    {
        TfLiteTensor *input = pinterpreter->input(i);
        int8_t *ptr = (int8_t*) (input->data.data);
#ifdef PRINT_ML_FEED_ARRAY
        int8_t temp_val = 0;
#endif

#ifdef SIMULATE_ML_FEED_ARRAY
        int8_t *feed_simulate = NULL;
        extern char *get_simulate_ml_feed_array();
        feed_simulate = (int8_t *)get_simulate_ml_feed_array();
        if(NULL == feed_simulate)
        {
        	printf("\r\nSimulate  completed\r\n");
        	while(1);
        }
#endif

        for (size_t j = 0; j < input->bytes; j++)
        {
#ifdef SIMULATE_ML_FEED_ARRAY
        	 *ptr++ = *feed_simulate++;
#else
            float temp = (float) features_buffer_fixed[j] * factor
                    + (float) feature_offset;

            if (temp > 127.0f)
            {
                *ptr++ = 127;
#ifdef PRINT_ML_FEED_ARRAY
                temp_val = 127;
#endif
            }
            else if (temp < -128.0f)
            {
                *ptr++ = -128;
#ifdef PRINT_ML_FEED_ARRAY
                temp_val = -128;
#endif
            }
            else
            {
                *ptr++ = (int8_t) temp;
#ifdef PRINT_ML_FEED_ARRAY
                temp_val = (int8_t) temp;
#endif
            }

#ifdef PRINT_ML_FEED_ARRAY
            (void)temp_val;

            if(ML_FEED_COUNTER_MATCH)
            {
                printf("0X%02X,",(unsigned char)temp_val);
        	}
#endif
#endif
        }
    }

#ifdef PRINT_ML_FEED_ARRAY
    if(ML_FEED_COUNTER_MATCH)
    {
        printf("\r\nMLInputEnd\r\n");
	}
#endif

    TfLiteStatus tf_status = pinterpreter->Invoke();
    if (tf_status != kTfLiteOk)
    {
        TF_LITE_REPORT_ERROR(&micro_error_reporter, "Invoke failed\n");
        cy_lpwwd_ml_log_err(tf_status,"TFLite Invoke error status:, exit");
        return tf_status;
    }

    for (size_t i = 0; i < pinterpreter->outputs_size(); i++)
    {
        TfLiteTensor *output = pinterpreter->output(i);
        TfLiteQuantization quantization = output->quantization;
        double val = 0;
        int best = 0;

        for (int j = 0; j < infer_out_size; ++j)
        {
            get_element(&micro_error_reporter, output, j, &best, &val);

            output_score[j] = dequantize(micro_error_reporter, val,
                    &quantization);

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

    }

    return result;
}

cy_rslt_t cy_lpwwd_ml_deinit( )
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    tflite::MicroInterpreter *pinterpreter = NULL;

    if(NULL == ml_handle)
    {
        result = CY_RSLT_LPWWD_ML_NOT_INTIAILIZED;
        cy_lpwwd_ml_log_err(result, "ML not initialized");
        return result;
    }

    pinterpreter =
            reinterpret_cast<tflite::MicroInterpreter*>(ml_handle->tflm_obj);

    if (NULL != pinterpreter)
    {
        delete pinterpreter;
        pinterpreter = NULL;
        cy_lpwwd_ml_log_info("Intreprt delete done");
    }
    else
    {
        cy_lpwwd_ml_log_info("Intreprt not created");
    }

    if (NULL != tensor_arena)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_ML_ARENA_BUFFER, tensor_arena);
        }
        else
        {
            free(tensor_arena);
        }
        tensor_arena = NULL;
    }
    else
    {
        cy_lpwwd_ml_log_info("tensor_arena buffer not created");
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
#endif
#endif
/* [] END OF FILE */
