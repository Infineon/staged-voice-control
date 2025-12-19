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
 * @file cy_staged_voice_control_hp.c
 *
 * @brief Staged voice control API implementation for library
 * running in high performance domain (M55).
 *
 */

#ifdef ENABLE_SVC_HP_MW
#include "cy_staged_voice_control.h"

#include "staged_voice_control_hp_private.h"
#include "staged_voice_control_hp_ipc.h"
#ifdef ENABLE_TASK_FOR_SVC_HP
#include "staged_voice_control_hp_thread.h"
#endif

#ifdef ENABLE_TIMELINE_MARKER
#include "audio_timeline_marker.h"
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

static cy_rslt_t cy_svc_hp_free_resources(svc_hp_instance_t *hp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    /**
     * Intentionally faling back on fail and freeing the allocated resource
     * if any for safer conditions.
     */
#ifdef ENABLE_TASK_FOR_SVC_HP
    ret_val = svc_hp_delete_thread_resouces(hp_instance);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "Create thread resource fail");
    }
#endif

    ret_val = svc_hp_ipc_deinit(hp_instance);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "IPC deinit fail");
    }
    else
    {
        cy_svc_log_info("IPC deinit success");
    }

    if(NULL != hp_instance->cmdProcSyncSVCLPSemaphore)
    {
        cy_rtos_deinit_semaphore(&hp_instance->cmdProcSyncSVCLPSemaphore);
        hp_instance->cmdProcSyncSVCLPSemaphore = NULL;
        cy_svc_log_info("Sem delete success");
    }
    else
    {
        cy_svc_log_info("Sem is not yet created");
    }

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    if(NULL != hp_instance->cmdProcIPCSendSemaphore)
    {
        cy_rtos_deinit_semaphore(&hp_instance->cmdProcIPCSendSemaphore);
        hp_instance->cmdProcIPCSendSemaphore = NULL;
        cy_svc_log_info("Sem delete success");
    }
    else
    {
        cy_svc_log_info("Sem is not yet created");
    }
#endif

    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
}


/**
 * Initializes the Staged Voice Control module on high performance domain.
 * It creates the resources required to communicate with staged voice control
 * running on low power (m33). Resources could be IPC, internal task, etc.,
 *
 * @param[in]  init                 Staged voice control module init
 *                                  configuration parameter in high performance
 *                                  domain.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_hp_init(cy_svc_hp_config_t *init)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_instance_t *hp_instance = NULL;

    hp_instance = svc_hp_get_instance();

    if (NULL == init)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "NULL params %p", init);
        goto CLEAN_RETURN;
    }

    if (true == hp_instance->is_deinit_done)
    {
        ret_val = CY_RSLT_SVC_DEINIT_INIT_NOT_SUPPORTED;
        cy_svc_log_err(ret_val, "cy_svc_hp_init after cy_svc_hp_deinit API call is not supported");
        return ret_val;
    }

    if (true == hp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_ALREADY_INITIALIZED;
        cy_svc_log_err(ret_val, "cy_svc_hp_init already initialized");
        goto CLEAN_RETURN;
    }

    if(NULL == init->data_callback)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Invalid data callback, %p",init->data_callback);
        goto CLEAN_RETURN;
    }

#ifndef ENABLE_LOW_POWER_SVC
#if defined(COMPONENT_CM4) || defined (COMPONENT_CYW20829)
    if (16 <= init->ipc_communication_sync_sempahore)
#else
    if (CY_IPC_SEMA_COUNT <= init->ipc_communication_sync_sempahore)
#endif
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Invalid IPC communication info:%d",
                init->ipc_communication_sync_sempahore);
        goto CLEAN_RETURN;
    }
#endif

    cy_svc_log_info("InitParams:userarg:%p, callback:%p",
            init->callback_user_arg, init->data_callback);


    ret_val = cy_rtos_init_semaphore(&hp_instance->cmdProcSyncSVCLPSemaphore, 1, 0);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "Failed to initialize core sync semaphore ");
        goto CLEAN_RETURN;
    }

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    ret_val = cy_rtos_init_semaphore(&hp_instance->cmdProcIPCSendSemaphore, 1, 0);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "Failed to initialize core sync semaphore ");
        goto CLEAN_RETURN;
    }
#endif

    ret_val = svc_hp_ipc_init(hp_instance, init);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "IPC init fail");
        goto CLEAN_RETURN;
    }

    hp_instance->init_params = *init;

#ifdef ENABLE_TASK_FOR_SVC_HP
    ret_val = svc_hp_create_thread_resouces(hp_instance);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "Create thread resource fail");
        goto CLEAN_RETURN;
    }
#endif

    /* Store the entire create instance params. */
    hp_instance->init_done = true;
    hp_instance->api_set_allowed = true;
    cy_svc_log_info("Init SVC HP success");

    CLEAN_RETURN:
    if (CY_RSLT_SUCCESS != ret_val)
    {
        (void) cy_svc_hp_free_resources(hp_instance);
    }
    return ret_val;
}

#ifdef ENABLE_TIMELINE_MARKER
void svc_hp_update_timeline_marker(cy_svc_set_state_t state)
{
#if 0
    switch(state)
    {
        case CY_SVC_SET_STATE_INVALID:
        {
            break;
        }
        case CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED:
        {
            cy_audio_timeline_marker_update_status(
                    CY_AUDIO_TIMELINE_HPWWD_DETECTED, NULL);
            break;
        }
        case CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED:
        {
            cy_audio_timeline_marker_update_status(
                    CY_AUDIO_TIMELINE_HPWWD_NOT_DETECTED, NULL);
            break;
        }
        case CY_SVC_SET_STATE_ASR_DETECTED:
        {
            cy_audio_timeline_marker_update_status(
                    CY_AUDIO_TIMELINE_ASR_DETECTED, NULL);
            break;
        }
        case CY_SVC_SET_STATE_ASR_NOT_DETECTED:
        {
            cy_audio_timeline_marker_update_status(
                    CY_AUDIO_TIMELINE_ASR_NOT_DETECTED, NULL);
            break;
        }
        case CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED:
        {
            cy_audio_timeline_marker_update_status(
                    CY_AUDIO_TIMELINE_ASR_PROCESS_COMPLETED, NULL);
            break;
        }
        case CY_SVC_SET_STATE_MAX:
        {
            break;
        }
        default:
        {
            break;
        }
    }
    return;
#endif
}
#endif

/**
 * Once SVC-HP application process the data and get the response of data
 * processing, the response of data processing needs to be sent to SVC through
 * this API. This API would take care of sending the message internally through
 * IPC to the SVC-LP module.
 *
 * @param[in] state                 State of the processed data
 *
 * @param[in] state_info            Information requested by the SVC HP application
 *
 *                                  example:
 *                                  For state - CY_SVC_SET_STATE_HIGH_PERFORMANCE_DETECT_IN_PROGRESS:
 *                                  state-info cy_svc_hp_set_state_hpwwd_detection_in_prog_info_t
 *
 *                                  For state - CY_SVC_SET_STATE_ASR_DETECTED
 *                                  state-info - cy_svc_hp_set_state_asr_detected_info_t
 *
 *                                  For other states: state_info must be NULL.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_hp_set_state(cy_svc_set_state_t state, void *state_info)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_instance_t *hp_instance = NULL;
    bool valid_state = false;

#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
    SVC_HP_CHECK_POINT_APP_API()
#endif

    hp_instance = svc_hp_get_instance();

    if (false == hp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "SVC HP is not initialized");
        goto CLEAN_RETURN;
    }

    hp_instance->set_state_dbg = state;

    if(false == hp_instance->api_set_allowed)
    {
        ret_val = CY_RSLT_SVC_INVALID_STATE;
        cy_svc_log_err(ret_val, "API not allowed");
        goto CLEAN_RETURN;
    }


    if ((CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED == state
            || CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED
                    == state))
    {
        if (hp_instance->stage_config_list_from_lp
                & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS)
        {
            valid_state = true;
        }
    }

    if ((CY_SVC_SET_STATE_ASR_DETECTED == state
            || CY_SVC_SET_STATE_ASR_NOT_DETECTED == state)
            || CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED == state)
    {
        if (hp_instance->stage_config_list_from_lp
                & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS)
        {
            valid_state = true;
        }
    }

    if ((CY_SVC_SET_STATE_ASR_DETECTED == state
            || CY_SVC_SET_STATE_HIGH_PERFORMANCE_DETECT_IN_PROGRESS
                    == state))
    {
        if (NULL != state_info)
        {
            valid_state = true;
        }
    }
    else
    {
        if (NULL != state_info)
        {
            valid_state = false;
        }
    }

    if (false == valid_state)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Invalid state[%d], cfg-lp[0x%x], %p", state,
                hp_instance->stage_config_list_from_lp,
                state_info);
        goto CLEAN_RETURN;
    }

    if(false == is_in_isr())
    {
        cy_svc_log_info("set_state req: 0x%x", state);
    }

    ret_val = svc_hp_ipc_send_command_set_state(hp_instance, state, state_info);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "set_state fail");
#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
        SVC_HP_CHECK_POINT_APP_API()
#endif
        goto CLEAN_RETURN;
    }

#ifdef ENABLE_TIMELINE_MARKER
    svc_hp_update_timeline_marker(state);
#endif

    CLEAN_RETURN:
#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
    SVC_HP_CHECK_POINT_APP_API()
#endif

    return ret_val;
}

/**
 * Get the current stage of staged voice control module.
 *
 * @param[out]  stage                staged voice control module's current stage
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_hp_get_current_stage(
        cy_svc_stage_t *stage)
{

    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_instance_t *hp_instance = NULL;

#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
    SVC_HP_CHECK_POINT_APP_API()
#endif

    hp_instance = svc_hp_get_instance();

    if (false == hp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "SVC HP is not initialized");
        goto CLEAN_RETURN;
    }

    if(false == hp_instance->api_set_allowed)
    {
        ret_val = CY_RSLT_SVC_INVALID_STATE;
        cy_svc_log_err(ret_val, "API not allowed");
        goto CLEAN_RETURN;
    }

    if (NULL == stage)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "NULL params %p", stage);
        goto CLEAN_RETURN;
    }

    *stage = hp_instance->current_stage;

    ret_val = CY_RSLT_SUCCESS;
    CLEAN_RETURN:

#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
    SVC_HP_CHECK_POINT_APP_API()
#endif

    return ret_val;
}

/**
 * De-initialize the Staged Voice Control module in high performance domain and
 * deletes the resources created during \ref cy_svc_hp_init
 *
 * @param None
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_hp_deinit(void)
{
    svc_hp_instance_t *hp_instance = NULL;
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    hp_instance = svc_hp_get_instance();
    hp_instance->api_set_allowed = false;

    if (false == hp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "cy_svc_hp_init is not initialized");
        goto CLEAN_RETURN;
    }

    (void) cy_svc_hp_free_resources(hp_instance);

    memset(hp_instance, 0, sizeof(*hp_instance));
    hp_instance->is_deinit_done = true;
    cy_svc_log_info("cy_svc_hp_deinit success");

    ret_val = CY_RSLT_SUCCESS;

    CLEAN_RETURN:
    return ret_val;
}

#endif
