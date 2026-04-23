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
 * @file staged_voice_control_lp_lpwwd_external.c
 *
 */

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_process_data.h"

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
static cy_rslt_t svc_lp_handle_lpwwd_external_detected_state(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    (void) svc_lp_trigger_state(lp_instance,
            SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED);

    lp_instance->lpwwd_detected = true;
    cy_svc_log_dbg("LPWWD is detected");

    svc_lp_start_circular_buf_update_on_transition_to_hp(lp_instance, data,
            SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED);

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}


cy_rslt_t svc_lp_process_data_lpwwd_external(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
    cy_svc_lp_external_lpwwd_state_t lpwwd_detected =
            CY_SVC_EXTERNAL_LPWWD_WWD_STATE_INVALID;
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (CY_SVC_STAGE_WAITING_FOR_LOW_POWER_WAKEUP_WORD_DETECTION
            == lp_instance->current_stage)
    {
        if (false == lp_instance->lpwwd_detected)
        {
            if (NULL != lp_instance->init_params.lpwwd_external_data_callback)
            {
                lp_instance->stats.lpwwd_feed_counter_dbg++;

#ifdef ENABLE_SVC_LP_CHECK_POINT
                SVC_LP_CHECK_POINT();
#endif
                ret_val = lp_instance->init_params.lpwwd_external_data_callback(
                        (CY_SVC_DATA_T*) data, 1,
                        lp_instance->init_params.callback_user_arg,
                        &lpwwd_detected);
                if (CY_RSLT_SUCCESS != ret_val)
                {
                    cy_svc_log_err(ret_val, "data_callback failed");
                }

#ifdef ENABLE_SVC_LP_CHECK_POINT
                SVC_LP_CHECK_POINT();
#endif

                switch(lpwwd_detected)
                {
                    case CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_IN_PROGRESS:
                        break;

                    case CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_FAILED:
                    {
                        (void) svc_lp_trigger_state(lp_instance,
                                SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_NOT_DETECTED);

                        cy_svc_log_dbg("ELPWWD NDETD [Addr:%p,DET:%d:NDET:%d]", data,
                                lp_instance->stats.lpwwd_detect_counter_dbg,
                                lp_instance->stats.lpwwd_not_detect_counter_dbg);
                        break;
                    }
                    case CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_SUCCESS:
                    {
                        svc_lp_handle_lpwwd_external_detected_state(lp_instance,
                                data);

                        cy_svc_log_dbg("ELPWWD DETD [Addr:%p,DET:%d:NDET:%d]", data,
                                lp_instance->stats.lpwwd_detect_counter_dbg,
                                lp_instance->stats.lpwwd_not_detect_counter_dbg);
                        break;
                    }
                    case CY_SVC_EXTERNAL_LPWWD_WWD_STATE_INVALID:
                    case CY_SVC_EXTERNAL_LPWWD_WWD_STATE_MAX:
                    {
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}

#endif
