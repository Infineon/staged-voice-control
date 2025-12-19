/*
 * (c) 2025, Infineon Technologies AG, or an affiliate of Infineon
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
 * @file staged_voice_control_lp_aad.c
 *
 */

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_aad.h"
#include "staged_voice_control_lp_sod.h"
#include "staged_voice_control_lp_lpwwd.h"
#include "staged_voice_control_lp_hpwwd.h"

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


cy_rslt_t svc_lp_aad_reset(svc_lp_instance_t *lp_instance)
{
    lp_instance->stats.frame_counter_received_after_last_aad_dbg = 0;

    /**
     * Resetting the last frame start address, as there is a gap
     * in the feed, old frames address stored is of no use.
     */
    lp_instance->last_processed_frame_start_address = NULL;

    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_aad_reset_and_high_components(svc_lp_instance_t *lp_instance)
{
    svc_lp_aad_reset(lp_instance);

    /**
     * Reset the higher stage states if any exists.
     */
    (void) svc_lp_sod_reset(lp_instance);
    (void) svc_lp_lpwwd_reset(lp_instance);
    (void) svc_lp_hpwwd_reset(lp_instance);
    (void) svc_lp_asr_reset(lp_instance);

    if (CY_SVC_ENABLE_SOD & lp_instance->init_params.stage_config_list)
    {
        (void) cy_sod_reset(lp_instance->sod_handle);
    }
    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_auto_aad_detection_check(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (true == lp_instance->enable_auto_detect_frame_feed_discontinuity)
    {
        cy_time_t cur_time = 0;
        (void) cy_rtos_get_time(&cur_time);

        if (0 == lp_instance->stats.last_frame_feed_time_ms)
        {
            cy_svc_log_info("First Feed after boot");
            svc_lp_trigger_state(lp_instance,
                    SVC_TRIGGER_ACOUSTIC_ACTIVITY_FORCEFULLY);
        }
        else if (cur_time > lp_instance->stats.last_frame_feed_time_ms)
        {
            if ((cur_time - lp_instance->stats.last_frame_feed_time_ms)
                    > lp_instance->auto_frame_feed_discontinuity_timeout_ms)
            {
                svc_lp_trigger_state(lp_instance,
                        SVC_TRIGGER_ACOUSTIC_ACTIVITY_FORCEFULLY);
            }
        }
        else
        {
            /**
             * Normal execution time. During continuous feed time.
             */
            ;
        }

        lp_instance->stats.last_frame_feed_time_ms = cur_time;
    }

    if (CY_SVC_STAGE_WAITING_FOR_ACOUSTIC_ACTIVITY
            == lp_instance->current_stage)
    {
        svc_lp_trigger_state(lp_instance,
                SVC_TRIGGER_ACOUSTIC_ACTIVITY_DETECTED);
    }

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}

#endif
