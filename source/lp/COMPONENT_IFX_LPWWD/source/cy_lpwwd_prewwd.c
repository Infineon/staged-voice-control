/******************************************************************************
 * File Name: cy_lpwwd_prewwd.c
 *
 * Description: Implements functions for pre wake-word detection.
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
#include "cy_lpwwd_private.h"
#include "cy_lpwwd_prewwd.h"
#include "cy_lpwwd.h"
#include "cy_lpwwd_error.h"
#include "ifx_sp_utils.h"
#include "ifx_pre_post_process.h"
#include <math.h>

#ifdef COMPONENT_MEMORY_CHECK_UTILS
#include "cy_mem_check_utils.h"
#endif
#include "cy_fe.h"

/******************************************************
 *                      Typedefs
 ******************************************************/

cy_fe_handle* fe_handle = NULL;

/******************************************************
 *                      Macros
 ******************************************************/
#if ENABLE_LPWWD_LOGS == 2
#define cy_lpwwd_prewwd_log_info(format,...)  printf ("[PREWWD] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_prewwd_log_err(ret_val,format,...)  printf ("[PREWWD] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_lpwwd_prewwd_log_dbg(format,...)  printf ("[PREWWD] "format" \r\n",##__VA_ARGS__);
#elif ENABLE_LPWWD_LOGS
#define cy_lpwwd_prewwd_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[PREWWD] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_prewwd_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[PREWWD] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_lpwwd_prewwd_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[PREWWD] "format" \r\n",##__VA_ARGS__);
#else
#define cy_lpwwd_prewwd_log_info(format,...)
#define cy_lpwwd_prewwd_log_err(ret_val,format,...)
#define cy_lpwwd_prewwd_log_dbg(format,...)
#endif

/******************************************************
 *                      Variables
 ******************************************************/

/******************************************************
 *                      Function Prototypes
 ******************************************************/

cy_rslt_t cy_lpwwd_prewwd_init(cy_lpwwd_prewwd_config_params_t *config_params,
        uint16_t *num_of_features)
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    cy_fe_config_params_t fe_config = { 0 };
    uint16_t num_features = 0;

    if(NULL != fe_handle)
    {
        status = CY_RSLT_LPWWD_PREWWD_ALREADY_INTIAILIZED;
        cy_lpwwd_prewwd_log_err(status, "Already initialized");
        return status;
    }

    if((NULL == config_params) || (NULL == num_of_features))
    {
        status = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_prewwd_log_err(status, "Inv arg :%p, %p", config_params,
                num_of_features);
        return status;
    }

    fe_config.sampling_rate = config_params->sampling_rate;
    fe_config.frame_size = config_params->frame_size;
    fe_config.component_id = IFX_PRE_PROCESS_IP_COMPONENT_MFCC;
    fe_config.frame_shift = config_params->frame_shift;
    fe_config.number_of_filter_banks = config_params->number_of_filter_banks;
    fe_config.number_of_dct_coefficients =
            config_params->number_of_dct_coefficients;
    fe_config.audio_frame_size = config_params->audio_input_frame_size;

    cy_lpwwd_prewwd_log_info(
            "cy_fe_init:SR:%d,FSz:%d,CId:%d,FSft:%d,FBank:%d,Dct:%d,ASz:%d",
            fe_config.sampling_rate, fe_config.frame_size,
            fe_config.component_id, fe_config.frame_shift,
            fe_config.number_of_filter_banks,
            fe_config.number_of_dct_coefficients, fe_config.audio_frame_size);

    status = cy_fe_init(&fe_config, &fe_handle);
    if (status != CY_RSLT_SUCCESS)
    {
        cy_lpwwd_prewwd_log_err(status, "cy_fe_init fail");
        return status;
    }

    num_features = fe_handle->fe_info.output_size;

    *num_of_features = num_features;

    cy_lpwwd_prewwd_log_info("cy_fe_init success:NumFeature:%d",
            fe_handle->fe_info.output_size);

    return status;
}

cy_rslt_t cy_lpwwd_prewwd_feed(int16_t *input_data)
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    if(fe_handle == NULL)
    {
        status = CY_RSLT_LPWWD_PREWWD_NOT_INTIAILIZED;
        cy_lpwwd_prewwd_log_err(status, "PreWWD not initialized");
        return status;
    }

    if(NULL == input_data)
    {
        status = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_prewwd_log_err(status, "Inv arg: %p", input_data);
        return status;
    }

    status = cy_fe_update_audio_buffer(fe_handle, input_data);

    return status;
}

cy_rslt_t cy_lpwwd_prewwd_generate_feature_frame(
        cy_lpwwd_feature_buffer *feature_buffer)
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    if(fe_handle == NULL)
    {
        status = CY_RSLT_LPWWD_PREWWD_NOT_INTIAILIZED;
        cy_lpwwd_prewwd_log_err(status, "PreWWD not initialized");
        return status;
    }

    if(NULL == feature_buffer)
    {
        status = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_prewwd_log_err(status, "Inv arg: %p", feature_buffer);
        return status;
    }

#ifdef RUN_FLOAT_FE
    status = cy_fe_process(fe_handle, feature_buffer->feature_frame_buffer);
#else
    int32_t out_q;
    status = cy_fe_process(fe_handle, feature_buffer->feature_frame_buffer, &out_q);
#endif
    if (status != CY_RSLT_SUCCESS)
    {
        cy_lpwwd_prewwd_log_err(status, "CY_FE_PROCESS error");
        return status;
    }
    return status;
}

cy_rslt_t cy_lpwwd_reset_audio_buffer()
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    if(fe_handle == NULL)
    {
        status = CY_RSLT_LPWWD_PREWWD_NOT_INTIAILIZED;
        cy_lpwwd_prewwd_log_err(status, "PreWWD not initialized");
        return status;
    }

    status = cy_fe_reset_audio_buffer(fe_handle);
    if (status != CY_RSLT_SUCCESS)
    {
        cy_lpwwd_prewwd_log_err(status, "cy_fe_reset_audio_buffer error");
        return status;
    }
    return status;
}

cy_rslt_t cy_lpwwd_prewwd_deinit()
{
    cy_rslt_t status = CY_RSLT_SUCCESS;
    if(fe_handle == NULL)
    {
        status = CY_RSLT_LPWWD_PREWWD_NOT_INTIAILIZED;
        cy_lpwwd_prewwd_log_err(status, "PreWWD not initialized");
        return status;
    }

    status = cy_fe_deinit(fe_handle);
    if (status != CY_RSLT_SUCCESS)
    {
        cy_lpwwd_prewwd_log_err(status, "cy_fe_deinit error");
        return status;
    }
    else
    {
        cy_lpwwd_prewwd_log_info("cy_fe_deinit success");
    }

    fe_handle = NULL;
    return status;
}

/* [] END OF FILE */
