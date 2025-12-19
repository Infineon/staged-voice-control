/******************************************************************************
 * File Name: cy_lpwwd_post_wwd.c
 *
 * Description: Implements functions for post wake-word detection.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>


#include "cy_lpwwd_private.h"
#include "cy_lpwwd_postwwd.h"
#include "cy_lpwwd.h"

#ifdef COMPONENT_MEMORY_CHECK_UTILS
#include "cy_mem_check_utils.h"
#endif

#include "math.h"
#include "cy_log.h"
#include "cy_result.h"
#include "ifx_pre_post_process.h"
#include "cy_lpwwd_error.h"
#include "cy_lpwwd_common.h"
#include "cy_post_process.h"
#include "cy_lpwwd_defines.h"
#include "cy_lpwwd_debug_utils.h"

/******************************************************
 *                      Typedefs
 ******************************************************/

/******************************************************
 *                      Macros
 ******************************************************/
#if ENABLE_LPWWD_LOGS == 2
#define cy_lpwwd_postwwd_log_info(format,...)  printf ("[postwwd] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_postwwd_log_err(ret_val,format,...)  printf ("[postwwd] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_lpwwd_postwwd_log_dbg(format,...)  printf ("[postwwd] "format" \r\n",##__VA_ARGS__);
#elif ENABLE_LPWWD_LOGS
#define cy_lpwwd_postwwd_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[postwwd] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_postwwd_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[postwwd] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_lpwwd_postwwd_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[postwwd] "format" \r\n",##__VA_ARGS__);
#else
#define cy_lpwwd_postwwd_log_info(format,...)
#define cy_lpwwd_postwwd_log_err(ret_val,format,...)
#define cy_lpwwd_postwwd_log_dbg(format,...)
#endif

/******************************************************
 *                      Variables
 ******************************************************/


cy_inference_post_process_handle* pp_handle = NULL;
//float hmm_pp_thd = 0;
float naive_pp_thd = NAIVE_PP_THD;
float wwd_trigger_threshold = 0.5f;
static uint16_t infer_out_size = 4;

int hmm_pp_detections;
int hmm_pp_kwrejections;
int hmm_pp_rejections;
int hmm_pp_sptimeouts;
int hmm_pp_ntimeouts;
int hmm_pp_other_cases;
int hmm_pp_no_decisions;

/* Buffer for inference output ids */
int* output_id_array = NULL;    /* This can be NULL for one keyword model by default. Used for multi-keywords models. */
void *reduced_output_score = NULL;
#define CLASS_ID_SINGLE_WWD "WWDtoken0, WWDtoken1, garbage, noise"

/******************************************************
 *                      Function Prototypes
 ******************************************************/
void init_hmm_pp_stats()
{
    hmm_pp_detections = 0;
    hmm_pp_kwrejections = 0;
    hmm_pp_rejections = 0;
    hmm_pp_sptimeouts = 0;
    hmm_pp_ntimeouts = 0;
    hmm_pp_other_cases = 0;
    hmm_pp_no_decisions = 0;
}


cy_rslt_t cy_lpwwd_postwwd_classid_init(char *class_id_buffer)
{
    cy_rslt_t status = CY_RSLT_SUCCESS;

    if (class_id_buffer != NULL)
    {/* Prepare for combining output scores */

        if (svc_alloc_memory)
        {
            svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                    (uint32_t)(infer_out_size * sizeof(int)),
                    (void**) &output_id_array);
            if (NULL != output_id_array)
            {
                memset(output_id_array, 0, infer_out_size * sizeof(int));
            }
        }
        else
        {
            output_id_array = (int*) calloc(infer_out_size, sizeof(int));
        }
        /* HMMS PP always requires 16-bit input but Naive PP always requires float input, hence creat a bigger buffer */

        if (svc_alloc_memory)
        {
            svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                    (uint32_t)(NUM_PP_CLASS * sizeof(float)),
                    (void**) &reduced_output_score);
            if (NULL != reduced_output_score)
            {
                memset(reduced_output_score, 0, NUM_PP_CLASS * sizeof(float));
            }
        }
        else
        {
            reduced_output_score = calloc(NUM_PP_CLASS, sizeof(float));
        }
        if (output_id_array == NULL || reduced_output_score == NULL)
        {
        	cy_lpwwd_postwwd_log_info("IFX Class Convertion memory allocation error, exit!");
            return CY_RSLT_LPWWD_OUT_OF_MEMORY;
        }

        status = ifx_class_convertion_init(class_id_buffer, output_id_array, infer_out_size);
        if (status != CY_RSLT_SUCCESS) {
        	cy_lpwwd_postwwd_log_info("IFX Class Convertion init error status:, exit!");
            return status;
        }

        cy_lpwwd_postwwd_log_info("ifx class init success");
    }
    else
    {
    	 cy_lpwwd_postwwd_log_info("ifx class init skipped");
    }
    return status;
}



cy_rslt_t cy_lpwwd_postwwd_init(cy_lpwwd_postwwd_config_params_t *config_params)
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    cy_inference_post_process_config_params_t hmm_pp_config = { 0 };

    if(NULL != pp_handle)
    {
        status = CY_RSLT_LPWWD_POSTWWD_ALREADY_INTIAILIZED;
        cy_lpwwd_postwwd_log_err(status, "Already initialized");
        return status;
    }

    if(NULL == config_params)
    {
        status = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_postwwd_log_err(status, "Inv arg :%p", config_params);
        return status;
    }

    hmm_pp_config.sampling_rate = config_params->sampling_rate;
    hmm_pp_config.number_of_classes = config_params->number_of_tokens;
    hmm_pp_config.frame_rate = config_params->frame_rate;
    hmm_pp_config.lookback_buffer_length =  config_params->lookback_buffer_length;

    hmm_pp_config.stacked_frame_delay = config_params->stacked_frame_delay;
    hmm_pp_config.detection_threshold = config_params->detection_threshold;

    init_hmm_pp_stats();

    cy_lpwwd_postwwd_log_info(
            "cy_inference_post_process_init:SR:%d,Cls:%d,Fr:%d,LookBk:%d,StDelay:%d,Th:%d",
            hmm_pp_config.sampling_rate, hmm_pp_config.number_of_classes,
            hmm_pp_config.frame_rate, hmm_pp_config.lookback_buffer_length,
            hmm_pp_config.stacked_frame_delay,
            hmm_pp_config.detection_threshold);

    status = cy_inference_post_process_init(&hmm_pp_config, &pp_handle);
    if (status != CY_RSLT_SUCCESS)
    {
        cy_lpwwd_postwwd_log_err(status, "cy_inference_post_process_init fail");
        return status;
    }

    //speech_utils_post_process_get_threshold(pp_handle->pp_handle, &hmm_pp_thd);

    if (naive_pp_thd < 0)
    {
        // retrieve default theshold
        naive_pp_thd = wwd_trigger_threshold;
    }
    else
    {
        // set threhold
        wwd_trigger_threshold = naive_pp_thd;
    }


    char *class_id_buffer = CLASS_ID_SINGLE_WWD;

    status = cy_lpwwd_postwwd_classid_init(class_id_buffer);

    return status;
}

cy_rslt_t cy_lpwwd_postwwd_process(int16_t *output_score,
        cy_lpwwd_wwd_detect_status_t *wwd_status)
{
    int32_t decision = 0;
    cy_rslt_t status = CY_RSLT_SUCCESS;

    if(pp_handle == NULL)
    {
        status = CY_RSLT_LPWWD_POSTWWD_NOT_INTIAILIZED;
        cy_lpwwd_postwwd_log_err(status, "PostWWD not initialized");
        return status;
    }

    if ((NULL == output_score) || (NULL == wwd_status))
    {
        status = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_postwwd_log_err(status, "Inv arg :%p, %p", output_score,
                wwd_status);
        return status;
    }

    if (output_id_array != NULL && reduced_output_score != NULL)
    { /* Combine negative output scores */
        status = ifx_class_convertion_for_pp(output_score, IFX_ML_DATA_INT16, reduced_output_score, output_id_array, infer_out_size);
        if (status != IFX_SP_ENH_SUCCESS)
        {
        	status = CY_RSLT_LPWWD_ERROR_POST_PROCESS;
        	cy_lpwwd_postwwd_log_err( status, "ML Class Convertion error status:, exit!");
            return status;
        }
        //int byte_size = NUM_PP_CLASS * sizeof(int16_t);
        //memcpy(output_score_fixed, reduced_output_score, byte_size);


//        if (control_bit_flag & PP_CLASS_CNV_CAPTURE_ENABLE_BIT) {
//            out_cdv << " Converted Class for PP:";
//            for (int j = 0; j < NUM_PP_CLASS; ++j) {
//                out_cdv << output_score_fixed[j] << " ";
//            }
//        }

#if 0
        /* Naive post process needs float output */
        status = ifx_class_convertion_for_pp(output_score, IFX_ML_DATA_FLOAT, reduced_output_score, output_id_array, infer_out_size);
        if (status != IFX_SP_ENH_SUCCESS)
        {
            std::cout << "ML Naive Float Class Convertion error status:, exit!" << status << std::endl;
            return status;
        }
        byte_size = NUM_PP_CLASS * sizeof(float);
        memcpy(output_score, reduced_output_score, byte_size);
#endif
    }


    *wwd_status = CY_LPWWD_WAKE_WORD_INVALID;

    if(NULL != reduced_output_score)
    {

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
    	print_output_score_reduced(reduced_output_score);
#endif
        status = cy_inference_post_process(pp_handle, reduced_output_score,
                &decision);
    }
    else
    {
        status = cy_inference_post_process(pp_handle, output_score,
                &decision);
    }

    if (decision == 1)
    {
        hmm_pp_detections++;
        *wwd_status = CY_LPWWD_WAKE_WORD_DETECTED;
    }
    else if (decision == 0)
    {
        hmm_pp_no_decisions++;
        *wwd_status = CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS;
    }
    else if (decision == -1)
    {
        hmm_pp_kwrejections++;
        *wwd_status = CY_LPWWD_WAKE_WORD_NOT_DETECTED;
    }
    else if (decision == -2)
    {
        hmm_pp_rejections++;
        *wwd_status = CY_LPWWD_WAKE_WORD_REJECTED;
    }
    else if (decision == -3)
    {
        hmm_pp_sptimeouts++;
        *wwd_status = CY_LPWWD_TIMEOUT;
    }
    else if (decision == -4)
    {
        hmm_pp_ntimeouts++;
        *wwd_status = CY_LPWWD_TIMEOUT;
    }
    else if (decision < 0)
    {
        hmm_pp_other_cases++;
        *wwd_status = CY_LPWWD_FAIL_REASON_NOT_KNOWN;
    }

    return status;
}

cy_rslt_t cy_lpwwd_postwwd_reset()
{
    cy_rslt_t status = CY_RSLT_SUCCESS;

    if(pp_handle == NULL)
    {
        status = CY_RSLT_LPWWD_POSTWWD_NOT_INTIAILIZED;
        cy_lpwwd_postwwd_log_err(status, "PostWWD not initialized");
        return status;
    }

    status = cy_inference_post_process_reset(pp_handle);
    if(CY_RSLT_SUCCESS != status)
    {
        cy_lpwwd_postwwd_log_err(status, "PostWWD reset fail");
    }
    return status;
}

cy_rslt_t cy_lpwwd_postwwd_deinit()
{
    cy_rslt_t status = CY_RSLT_SUCCESS;

    if(pp_handle == NULL)
    {
        status = CY_RSLT_LPWWD_POSTWWD_NOT_INTIAILIZED;
        cy_lpwwd_postwwd_log_err(status, "PostWWD not initialized");
        return status;
    }

    status = cy_inference_post_process_deinit(pp_handle);
    if(CY_RSLT_SUCCESS != status)
    {
        cy_lpwwd_postwwd_log_err(status, "PostWWD deinit fail");
    }
    else
    {
        cy_lpwwd_postwwd_log_info("PostWWD deinit success");
    }

    if (NULL != output_id_array)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY, output_id_array);
        }
        else
        {
            free(output_id_array);
        }
		output_id_array = NULL;
	}

    if (NULL != reduced_output_score)
    {
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY, reduced_output_score);
        }
        else
        {
            free(reduced_output_score);
        }
		reduced_output_score = NULL;
	}

    pp_handle = NULL;

    return status;
}


/* [] END OF FILE */
