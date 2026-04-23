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
 * @file staged_voice_control_lp_stages.h
 *
 */

#ifndef CY_SVC_LP_STAGES_H__
#define CY_SVC_LP_STAGES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_private.h"

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
 *                              Type Definitions
 ******************************************************************************/

/*******************************************************************************
 *                              Structures
 ******************************************************************************/

/*******************************************************************************
 *                              Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

cy_rslt_t svc_lp_trigger_state(
        svc_lp_instance_t *lp_instance,
        svc_stage_trigger_t stage_trigger);

cy_rslt_t svc_lp_send_event_to_app_on_stage_change(
        svc_lp_instance_t *lp_instance,
        svc_stage_trigger_t state_trigger);

cy_rslt_t svc_lp_trigger_state_from_hp_set_state(
        svc_lp_instance_t *lp_instance,
        cy_svc_set_state_t  set_state,
        void *set_state_info);

cy_rslt_t svc_lp_trigger_stage_from_lp_app_set_stage (
        svc_lp_instance_t *lp_instance,
        cy_svc_stage_t   set_stage);

#ifdef ENABLE_TIMELINE_MARKER
cy_rslt_t svc_lp_trigger_audio_timeline_marker_update (
        svc_stage_trigger_t stage_trigger);
#endif

#endif
#endif /* CY_SVC_LP_STAGES_H__ */
