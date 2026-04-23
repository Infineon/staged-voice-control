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


cy_rslt_t svc_lp_hpwwd_reset(
        svc_lp_instance_t *lp_instance)
{
    if (!(lp_instance->init_params.stage_config_list
            & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS))
    {
        return CY_RSLT_SUCCESS;
    }

    lp_instance->hpwwd_trigger_data_final_address = NULL;
    lp_instance->post_hpwwd_pending_frame_counter_to_hp = 0;
    lp_instance->post_wwd_frame_count_req_by_hp = 0;

    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_asr_reset(
        svc_lp_instance_t *lp_instance)
{
    if (!(lp_instance->init_params.stage_config_list
            & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS))
    {
        return CY_RSLT_SUCCESS;
    }

    lp_instance->stream_requested_on_asr_processing_query_state = false;

    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_hpwwd_reset_and_high_components(
        svc_lp_instance_t *lp_instance)
{
    (void) svc_lp_hpwwd_reset(lp_instance);
    (void) svc_lp_asr_reset(lp_instance);

    return CY_RSLT_SUCCESS;
}

cy_rslt_t svc_lp_asr_reset_and_high_components(
        svc_lp_instance_t *lp_instance)
{
    (void) svc_lp_asr_reset(lp_instance);

    return CY_RSLT_SUCCESS;
}

#endif
