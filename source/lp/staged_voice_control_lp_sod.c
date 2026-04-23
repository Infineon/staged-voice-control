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
 * @file staged_voice_control_lp_sod.c
 *
 */

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_sod.h"
#include "staged_voice_control_lp_lpwwd.h"
#include "staged_voice_control_lp_hpwwd.h"
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_low_noise.h"
#include "staged_voice_control_lp_gain.h"
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

cy_rslt_t svc_lp_sod_init(svc_lp_instance_t *lp_instance,
        cy_svc_lp_config_t *init)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_sod_config_params config_params = {0};

    config_params.sensitivity = init->sod_sensitivity;
    config_params.onset_gap_setting_ms = init->sod_onset_gap_setting_ms;

    ret_val = cy_sod_init(&config_params, &lp_instance->sod_handle);

    return ret_val;
}

cy_rslt_t svc_lp_sod_deinit(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    ret_val = cy_sod_deinit(&lp_instance->sod_handle);
    lp_instance->sod_handle = NULL;
    return ret_val;
}

cy_rslt_t svc_lp_sod_check_if_any_data_to_be_sent_to_hp(
        svc_lp_instance_t *lp_instance,
        cy_sod_status_t sod_status,
        uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (!(lp_instance->init_params.stage_config_list
            & CY_SVC_ENABLE_LPWWD)
            && ((lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS)
                    || (lp_instance->init_params.stage_config_list
                            & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS)))
    {
        /**
         * SOD is detected, LPWWD is not enabled by the user, Hence
         * directly sending the buffer information to SVC HP after
         * SVC has detected SOD.
         */
        cy_svc_log_info(
                "LPWWD disabled, HPWWD/ASR enabled. Direct fwd data to HP");
        svc_lp_start_circular_buf_update_on_transition_to_hp(
                lp_instance, data, SVC_TRIGGER_SPEECH_ONSET_DETECTED);
    }

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}

/**
 * Create the required resource for the instance.
 *
 * @param[in]  lp_instance             Staged voice control module instance
 * @param[in]  create_domain        Create low power domain configuration
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_lp_sod_process(svc_lp_instance_t *lp_instance, uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    bool b_speech_detect_required = false;
    cy_sod_status_t sod_status = CY_SOD_STATUS_INVALID;
    int16_t audio_frame_temp[160]; // temporary audio frame for applying additional SOD mic gain
    int16_t* sod_processing_gain_ptr = NULL;

    if ((CY_SVC_STAGE_WAITING_FOR_SPEECH_ONSET_DETECTION
            == lp_instance->current_stage)
            || (CY_SVC_STAGE_WAITING_FOR_LOW_POWER_WAKEUP_WORD_DETECTION
                    == lp_instance->current_stage))
    {
        b_speech_detect_required = true;
    }

    if(true == b_speech_detect_required)
    {
        if (true == svc_lp_low_noise_detected((int16_t*) data))
        {
            /* Send event to app that low noise is detected -no state change */
            svc_lp_send_event_to_app_on_stage_change(
                    lp_instance,
                    SVC_TRIGGER_LOW_NOISE_DETECTED);
        }
    }

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    if(lp_instance->gain_config.sod_processing_gain > 1)
    {
        /* Apply post HPF gain with clipping */
        ret_val = svc_lp_apply_gain_with_clip(lp_instance->gain_config.sod_processing_gain, (int16_t*)data, audio_frame_temp, MONO_FRAME_SIZE);
        if(ret_val != CY_RSLT_SUCCESS)
        {
            cy_svc_log_err(ret_val,"Post HPF gain application failed");
            // not returning here, continue with original data
            sod_processing_gain_ptr = (int16_t*)data;
        }
        else
        {
            sod_processing_gain_ptr = (int16_t*)audio_frame_temp;
        }
    }
    else
    {
        sod_processing_gain_ptr = (int16_t*)data;
    }

    ret_val = cy_sod_process(lp_instance->sod_handle,
            b_speech_detect_required,
            sod_processing_gain_ptr,
            &sod_status);
#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    if(false == lp_instance->sod_detected)
    {
        if (CY_SOD_STATUS_DETECTED == sod_status)
        {
            /**
             * Fresh Speech detected, set the SOD detected flag to true and reset the SOD re-detect count.
             */
            lp_instance->sod_detected = true;
            lp_instance->sod_redetect_count = 0;

            (void) svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_SPEECH_ONSET_DETECTED);

#ifdef ENABLE_TIMELINE_MARKER
            (void) svc_lp_trigger_audio_timeline_marker_update(
                    SVC_TRIGGER_SPEECH_ONSET_DETECTED);
#endif

            cy_svc_log_dbg("SOD DETD [%p,Cnt:%d, Frame:%d]", data,
                    lp_instance->stats.sod_detect_counter_dbg, lp_instance->stats.frame_counter_received_after_last_aad_dbg);

#ifdef SIMULATE_SOD_IPC_TRIGGER_TEST
            svc_lp_app_ipc_send_command_to_trigger_event_from_hp(
                    CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED);
#endif

            if (!(lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_LPWWD))
            {
                if ((lp_instance->init_params.stage_config_list
                        & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS)
                        || (lp_instance->init_params.stage_config_list
                                & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS))
                {
                    (void) svc_lp_start_circular_buf_update_on_transition_to_hp(
                            lp_instance, data,
                            SVC_TRIGGER_SPEECH_ONSET_DETECTED);
                }
            }
        }
        else if (CY_SOD_STATUS_INPUT_DATA_PROCESSED == sod_status)
        {
            ;
        }
    }
    else
    {
        if(CY_SOD_STATUS_DETECTED == sod_status )
        {
            lp_instance->sod_redetect_count++;
#if 0
            (void) svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_SPEECH_ONSET_DETECTED);
#endif
#ifdef ENABLE_TIMELINE_MARKER
            (void) svc_lp_trigger_audio_timeline_marker_update(
                    SVC_TRIGGER_SPEECH_ONSET_DETECTED);
#endif

            cy_svc_log_dbg("SOD REDETD, Addr:%p,Cnt:%d, Frame:%d", data,
                    lp_instance->stats.sod_detect_counter_dbg, lp_instance->stats.frame_counter_received_after_last_aad_dbg);

            if (!(lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_LPWWD))
            {
                if ((lp_instance->init_params.stage_config_list
                        & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS)
                        || (lp_instance->init_params.stage_config_list
                                & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS))
                {
                    svc_lp_send_ipc_event_circular_buffer_update(lp_instance, data,
                            lp_instance->circular_shared_buffer->frame_size_in_bytes,
                            0, 0);
                }
            }
        }
    }

    return ret_val;
}


/**
 * Create the required resource for the instance.
 *
 * @param[in]  lp_instance             Staged voice control module instance
 * @param[in]  create_domain        Create low power domain configuration
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_lp_sod_reset (svc_lp_instance_t *lp_instance)
{
    lp_instance->sod_detected = false;
    lp_instance->sod_redetect_count = 0;

    cy_svc_log_dbg("Reset: SOD [Frame:%d]",  lp_instance->stats.frame_counter_received_after_last_aad_dbg);

    return CY_RSLT_SUCCESS;
}

/**
 * Create the required resource for the instance.
 *
 * @param[in]  lp_instance             Staged voice control module instance
 * @param[in]  create_domain        Create low power domain configuration
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_lp_sod_reset_and_high_components(svc_lp_instance_t *lp_instance)
{
    (void) svc_lp_sod_reset(lp_instance);
    /**
     * Reset the higher stage states if any exists.
     */
    (void) svc_lp_lpwwd_reset(lp_instance);
    (void) svc_lp_hpwwd_reset(lp_instance);
    (void) svc_lp_asr_reset(lp_instance);

    cy_svc_log_dbg("Reset: SOD, LPWWD, HPWWD, ASR (if applicable done)");

    return CY_RSLT_SUCCESS;
}

/**
 * @brief Configure low noise settings for the staged voice control low power module.
 *
 * This function applies the specified low noise configuration parameters to optimize
 * the voice control system for low noise environments. The configuration affects
 * noise reduction algorithms and sensitivity thresholds.
 *
 * @param[in] low_noise_config Pointer to the low noise configuration structure
 *                            containing the desired noise reduction parameters.
 *                            Must not be NULL.
 *
 * @return cy_rslt_t Result code indicating the success or failure of the operation.
 *
 */
cy_rslt_t cy_svc_lp_low_noise_config(cy_svc_lp_low_noise_config_t *low_noise_config)
{
    if(NULL == low_noise_config)
    {
        cy_svc_log_err(CY_RSLT_SVC_BAD_ARG, "NULL params %p", low_noise_config);
        return CY_RSLT_SVC_BAD_ARG;
    }

    uint32_t timeout_ms = low_noise_config->timeout_ms;
    uint32_t aad_threshold = low_noise_config->low_noise_threshold;
    bool enable_feature = low_noise_config->enable_feature;

    return svc_lp_low_noise_config(timeout_ms, aad_threshold, enable_feature);
}

uint32_t svc_lp_get_sod_redetection_count(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();
    cy_svc_log_dbg("Get sod_redetect_count: %"PRIu32, lp_instance->sod_redetect_count);
    return lp_instance->sod_redetect_count;
}

void svc_lp_decrement_sod_redetection_count(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    if(lp_instance->sod_redetect_count>0)
    {
        lp_instance->sod_redetect_count--;
        cy_svc_log_dbg("Decremented sod_redetect_count After value: %"PRIu32, lp_instance->sod_redetect_count);
    }

}


#endif
