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
 * @file staged_voice_control_lp_lpwwd.c
 *
 */
#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_process_data.h"
#include "staged_voice_control_lp_lpwwd_external.h"
#include "staged_voice_control_lp_lpwwd_internal.h"
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_aad.h"
#include "staged_voice_control_lp_hpwwd.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/
#define DEFAULT_SOD_LOOK_BACK_SIZE_FRAME_COUNT_FOR_LPWWD (10) //100ms.

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

unsigned int sod_detect_delay_look_back_frame_counter =
		DEFAULT_SOD_LOOK_BACK_SIZE_FRAME_COUNT_FOR_LPWWD;

static cy_rslt_t svc_lp_lpwwd_process_data_get_preshift_buffer(
        svc_lp_instance_t *lp_instance,
        uint8_t *data,
        uint8_t **pp_pre_shift_buffer)
{
    uint8_t *pre_shift_buffer = NULL;
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (NULL == pp_pre_shift_buffer)
    {
        goto CLEAN_RETURN;
    }
    *pp_pre_shift_buffer = NULL;

    if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_SOD)
    {
        if (0 < lp_instance->init_params.sod_onset_detect_max_late_hit_delay_ms)
        {
            sod_detect_delay_look_back_frame_counter =
                    lp_instance->init_params.sod_onset_detect_max_late_hit_delay_ms
                            / 10;
        }

        /**
         * This check ensures the pre-shift buffer needed for LPWWD is valid
         * and available. This check will avoid providing garbage when we dont
         * have enough data on start from AAD.
         */
        if (lp_instance->stats.frame_counter_received_after_last_aad_dbg
                < sod_detect_delay_look_back_frame_counter)
        {
            //cy_svc_log_dbg("Feed is not sufficient to start lpwwd");
            ret_val = CY_RSLT_SUCCESS;
            goto CLEAN_RETURN;
        }

        svc_lp_get_circular_buf_pre_shift_buffer_from_any_address(lp_instance,
                data, sod_detect_delay_look_back_frame_counter,
                &pre_shift_buffer);

        *pp_pre_shift_buffer = pre_shift_buffer;
    }
    else
    {
        *pp_pre_shift_buffer = data;
    }
    ret_val = CY_RSLT_SUCCESS;

    CLEAN_RETURN: return ret_val;
}

cy_rslt_t svc_lp_process_data_lpwwd(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t *pre_shift_buffer = NULL;

    (void) svc_lp_lpwwd_process_data_get_preshift_buffer(lp_instance, data,
            &pre_shift_buffer);
    if(NULL == pre_shift_buffer)
    {
        ret_val = CY_RSLT_SUCCESS;
    }
    else
    {
        if (true == lp_instance->init_params.is_lpwwd_external)
        {
            ret_val = svc_lp_process_data_lpwwd_external(lp_instance,
                    pre_shift_buffer);
        }
        else
        {
#ifdef ENABLE_IFX_LPWWD
            ret_val = svc_lp_process_data_lpwwd_internal(lp_instance,
                    pre_shift_buffer);
#else
            ret_val = CY_RSLT_SUCCESS;
#endif
        }
    }
    return ret_val;
}

cy_rslt_t svc_lp_lpwwd_init(
        svc_lp_instance_t *lp_instance,
        cy_svc_lp_config_t *init)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if(true == init->is_lpwwd_external)
    {
        ret_val = CY_RSLT_SUCCESS;
        goto CLEAN_RETURN;
    }
    else
    {
#ifdef ENABLE_IFX_LPWWD
        ret_val = svc_lp_lpwwd_internal_init(lp_instance, init);
        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err(ret_val, "svc_lp_lpwwd_internal_init fail");
            goto CLEAN_RETURN;
        }
#else
        ret_val = CY_RSLT_SUCCESS;
#endif
    }
    CLEAN_RETURN: return ret_val;
}


cy_rslt_t svc_lp_lpwwd_reset(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (!(lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_LPWWD))
    {
        return CY_RSLT_SUCCESS;
    }

    if (true == lp_instance->init_params.is_lpwwd_external)
    {
        ret_val = CY_RSLT_SUCCESS;
    }
    else
    {
#ifdef ENABLE_IFX_LPWWD
        ret_val = svc_lp_lpwwd_internal_reset(lp_instance);
#else
        ret_val = CY_RSLT_SUCCESS;
#endif
    }

    lp_instance->lpwwd_detected = false;
    lp_instance->lpwwd_feed_past_2_buffers_on_start_completed = false;
    lp_instance->post_wwd_frame_count_req_by_hp = 0;
    lp_instance->post_lpwwd_received_frame_count = 0;

    return ret_val;
}

cy_rslt_t svc_lp_lpwwd_reset_and_high_components(
        svc_lp_instance_t *lp_instance)
{
    (void) svc_lp_lpwwd_reset(lp_instance);
    /**
     * Reset the higher stage states if any exists.
     */
    (void) svc_lp_hpwwd_reset(lp_instance);
    (void) svc_lp_asr_reset(lp_instance);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_lpwwd_deinit(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (true == lp_instance->init_params.is_lpwwd_external)
    {
        ret_val = CY_RSLT_SUCCESS;
    }
    else
    {
#ifdef ENABLE_IFX_LPWWD
        ret_val = svc_lp_lpwwd_internal_deinit(lp_instance);
#else
        ret_val = CY_RSLT_SUCCESS;
#endif
    }

    return ret_val;
}

cy_rslt_t svc_lp_lpwwd_get_wwd_identified(
        svc_lp_instance_t *lp_instance,
        cy_svc_buffer_info_t *buff_info)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if(NULL == buff_info)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        goto CLEAN_RETURN;
    }
    *buff_info = 0;

    /**
     * TODO: If model is not able to identify, we have to say WWD identified.
     */
    *buff_info = CY_SVC_BUF_INFO_WWD_IDENTIFIED;

    ret_val = CY_RSLT_SUCCESS;
    CLEAN_RETURN:
    return ret_val;
}


#endif
