/*
 * (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
 * Technologies AG. All rights reserved.
 * This software, associated documentation and materials ("Software") is
 * owned by Infineon Technologies AG or one of its affiliates ("Infineon")
 * and is protected by and subject to worldwide patent protection, worldwide
 * copyright laws, and international treaty provisions. Therefore, you may use
 * this Software only as provided in the license agreement accompanying the
 * software package from which you obtained this Software. If no license
 * agreement applies, then any use, reproduction, modification, translation, or
 * compilation of this Software is prohibited without the express written
 * permission of Infineon.
 *
 * Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
 * IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
 * THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
 * SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
 * Infineon reserves the right to make changes to the Software without notice.
 * You are responsible for properly designing, programming, and testing the
 * functionality and safety of your intended application of the Software, as
 * well as complying with any legal requirements related to its use. Infineon
 * does not guarantee that the Software will be free from intrusion, data theft
 * or loss, or other breaches ("Security Breaches"), and Infineon shall have
 * no liability arising out of any Security Breaches. Unless otherwise
 * explicitly approved by Infineon, the Software may not be used in any
 * application where a failure of the Product or any consequences of the use
 * thereof can reasonably be expected to result in personal injury.
 */

/**
 * @file staged_voice_control_lp_lpwwd_internal.c
 *
 */
#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_process_data.h"
#include "staged_voice_control_lp_private.h"

#ifdef ENABLE_IFX_LPWWD
#include "staged_voice_control_lp_lpwwd_internal.h"
#endif

/*******************************************************************************
 *                              Macros
 ******************************************************************************/

/*******************************************************************************
 *                              Constants
 ******************************************************************************/

/*******************************************************************************
 *                              Enumerations
 ******************************************************************************/

/*******************************************************************************
 *                              Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

#ifdef ENABLE_IFX_LPWWD
static cy_rslt_t svc_lp_lpwwd_internal_process_detect_result(
        svc_lp_instance_t *lp_instance,
        uint8_t *data,
		cy_lpwwd_wwd_detect_status_t lpwwd_status)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    switch (lpwwd_status)
    {
        case CY_LPWWD_WAKE_WORD_INVALID:
        {
            break;
        }

//        case CY_SVC_LPWWD_STATUS_DETECTION_NOT_STARTED:
//        {
//            break;
//        }

        case CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS:
        {
            break;
        }

        case CY_LPWWD_FAIL_REASON_NOT_KNOWN:
        case CY_LPWWD_WAKE_WORD_NOT_DETECTED:
        case CY_LPWWD_WAKE_WORD_REJECTED:
        {
            (void) svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_NOT_DETECTED);

            lp_instance->lpwwd_detected = false;
            lp_instance->lpwwd_feed_past_2_buffers_on_start_completed = false;

            cy_svc_log_dbg("LPWWD detection fail");
            break;
        }

        case CY_LPWWD_TIMEOUT:
        {
            (void) svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_NOT_DETECTED);

            lp_instance->lpwwd_detected = false;
            lp_instance->lpwwd_feed_past_2_buffers_on_start_completed = false;
//            cy_svc_log_dbg("LPWWD detection timeout");
            break;
        }

        case CY_LPWWD_WAKE_WORD_DETECTED:
        {
            (void) svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED);

            lp_instance->lpwwd_detected = true;

            svc_lp_start_circular_buf_update_on_transition_to_hp(lp_instance,
                    data, SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED);

            cy_svc_log_dbg("LPWWD DETD [Addr:%p,DET:%d:NDET:%d:Frame:%d]", data,
                    lp_instance->stats.lpwwd_detect_counter_dbg,
                    lp_instance->stats.lpwwd_not_detect_counter_dbg,
                    lp_instance->stats.frame_counter_received_after_last_aad_dbg);
            break;
        }

//        case CY_SVC_LPWWD_STATUS_DETECTION_MAX:
//        {
//            break;
//        }

        default:
        {
            break;
        }
    }

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}
#endif

cy_rslt_t svc_lp_lpwwd_internal_init(
        svc_lp_instance_t *lp_instance,
        cy_svc_lp_config_t *init)
{
#ifdef ENABLE_IFX_LPWWD
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_lpwwd_config_params_t config_params = {0};

#ifdef ENABLE_SVC_ML_MW_SUPPORT
    config_params.dual_wake_word_detection = SVC_MODEL_DUAL_WAKEWORD_ENABLED;
    config_params.ml_model_meta_buf = NULL;
    config_params.ml_model_binary_buf = NULL;
#else
    config_params.dual_wake_word_detection = SVC_MODEL_DUAL_WAKEWORD_ENABLED;
    config_params.ml_model_meta_buf = SVC_MODEL_PARAMS_BUFFER;
    config_params.ml_model_binary_buf = SVC_MODEL_BINARY_BUFFER;
#endif

#ifdef ENABLE_IFX_LPWWD_HMMS
    config_params.hmm_model1_keyword_binary_buf = SVC_MODEL_HMM_KEYWORD_1;
    config_params.hmm_model1_garbage_binary_buf =  SVC_MODEL_HMM_GARBAGE_1;
    config_params.hmm_model1_noise_binary_buf = SVC_MODEL_HMM_NOISE_1;
    config_params.hmm_model2_keyword_binary_buf = SVC_MODEL_HMM_KEYWORD_2;
    config_params.hmm_model2_garbage_binary_buf =  SVC_MODEL_HMM_GARBAGE_2;
    config_params.hmm_model2_noise_binary_buf = SVC_MODEL_HMM_NOISE_2;
#endif /* ENABLE_IFX_LPWWD_HMMS */
    /* Read feature scale and feature offset from model itself, So assigning zeros*/
    config_params.feature_scale = 0;
    config_params.feature_offset = 0;

    config_params.lookback_buffer_length = init->sod_onset_detect_max_late_hit_delay_ms * 16000 / 1000;
    config_params.mel_low_freq = init->mel_low_freq;
    config_params.mel_high_freq = init->mel_high_freq;

    cy_svc_log_info("InitLPWWD[%d,%p,%p,%p,%p,%p,%p,%p,%p, %f,%d,%d]",
    		config_params.dual_wake_word_detection,
			config_params.ml_model_meta_buf,
			config_params.ml_model_binary_buf,
			config_params.hmm_model1_keyword_binary_buf,
			config_params.hmm_model1_garbage_binary_buf ,
			config_params.hmm_model1_noise_binary_buf,
			config_params.hmm_model2_keyword_binary_buf,
			config_params.hmm_model2_garbage_binary_buf ,
			config_params.hmm_model2_noise_binary_buf,
			config_params.feature_scale,
			config_params.feature_offset,
			config_params.lookback_buffer_length);

    ret_val = cy_lpwwd_init(&config_params, &lp_instance->lpwwd_handle);
    if(ret_val != CY_RSLT_SUCCESS)
    {
    	cy_svc_log_err(ret_val, "Failed to initialize LPWWD component");
    	return ret_val;
    }

    return ret_val;
#else
    return CY_RSLT_SUCCESS;
#endif
}

extern cy_rslt_t cy_lpwwd_prewwd_feed(int16_t *input_data);
cy_rslt_t svc_lp_process_data_lpwwd_internal(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
#ifdef ENABLE_IFX_LPWWD
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_lpwwd_wwd_detect_status_t wwd_status = CY_LPWWD_WAKE_WORD_INVALID;

    if ((CY_SVC_STAGE_WAITING_FOR_LOW_POWER_WAKEUP_WORD_DETECTION
            == lp_instance->current_stage)
            && (false == lp_instance->lpwwd_detected))
    {
        lp_instance->stats.lpwwd_feed_counter_dbg++;

#ifdef ENABLE_USB_DBG_OUTPUT
        usb_send_out_dbg_put(3, (short*) data);
#endif

        if(false == lp_instance->lpwwd_feed_past_2_buffers_on_start_completed)
        {
        	uint8_t *pre_buffer = NULL;

        	svc_lp_get_circular_buf_pre_shift_buffer_from_any_address(
        	        lp_instance, data, 2 , &pre_buffer);

        	cy_lpwwd_prewwd_feed((int16_t*) pre_buffer);

        	svc_lp_get_circular_buf_pre_shift_buffer_from_any_address(
        	        lp_instance, data, 1 , &pre_buffer);

        	cy_lpwwd_prewwd_feed((int16_t*) pre_buffer);

        	lp_instance->lpwwd_feed_past_2_buffers_on_start_completed = true;
        }

#ifdef ENABLE_SVC_LP_CHECK_POINT
        SVC_LP_CHECK_POINT();
#endif
        ret_val = cy_lpwwd_feed(lp_instance->lpwwd_handle, (char *)data, &wwd_status);
        if(CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err(ret_val, "Failed to process data in LPWWD");
            goto CLEAN_RETURN;
        }
#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

        ret_val = svc_lp_lpwwd_internal_process_detect_result(lp_instance, data,
    		    wwd_status);
        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err(ret_val, "postwwd process fail");
            goto CLEAN_RETURN;
        }
    }
    else
    {
    	ret_val = CY_RSLT_SUCCESS;
    }


    CLEAN_RETURN: return ret_val;
#else
    return CY_RSLT_SUCCESS;
#endif
}

cy_rslt_t svc_lp_lpwwd_internal_reset(svc_lp_instance_t *lp_instance)
{
#ifdef ENABLE_IFX_LPWWD
    cy_lpwwd_reset(lp_instance->lpwwd_handle);
    return CY_RSLT_SUCCESS;
#else
    return CY_RSLT_SUCCESS;
#endif
}

cy_rslt_t svc_lp_lpwwd_internal_deinit(
        svc_lp_instance_t *lp_instance)
{
#ifdef ENABLE_IFX_LPWWD
	cy_rslt_t result = CY_RSLT_SUCCESS;
    result = cy_lpwwd_deinit(&lp_instance->lpwwd_handle);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_svc_log_err(result, "Failed to de-initialize LPWWD");
        return result;
    }
    lp_instance->lpwwd_handle = NULL;
    return result;
#else
    return CY_RSLT_SUCCESS;
#endif
}
#endif
