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
 * @file cy_staged_voice_control_lp.c
 *
 * @brief Staged voice control API implementation for library
 * running in low power domain (M33).
 *
 */

#include "cy_staged_voice_control.h"
#include "staged_voice_control_lp_private.h"

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_resource.h"
#include "staged_voice_control_lp_process_data.h"
#include "staged_voice_control_lp_resource.h"
#ifndef ENABLE_MIC_INPUT_FEED
#ifdef ENABLE_USB_DBG_OUTPUT
#include "audio_usb_send_utils.h"
#endif
#endif
#include "cy_audio_license.h"

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

/**
 * Initializes the staged voice control module in low power device, creates the
 * required resources based on the input configuration, the resource could be
 * buffer, internal task, IPC communication, etc.,
 *
 * @param[in]  init                 Staged voice control module init
 *                                  configuration parameter in low power domain.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_init(cy_svc_lp_config_t *init)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;

    if (NULL == init)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "NULL params %p", init);
        goto CLEAN_RETURN;
    }

    lp_instance = svc_lp_get_instance();

    svc_alloc_memory = init->alloc_memory;
    svc_free_memory = init->free_memory;

    if((NULL == svc_alloc_memory) || (NULL == svc_free_memory))
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Mem alloc/free callback is null");
        goto CLEAN_RETURN;
    }

    /*
     * SVC doesnt support cy_svc_lp_init API call once cy_svc_lp_deinit is called. hence the below check is
     * to return error when init is called after deinit of SVC
     */
    if(true == lp_instance->is_deinit_done)
    {
        ret_val = CY_RSLT_SVC_DEINIT_INIT_NOT_SUPPORTED;
        cy_svc_log_err(ret_val, "cy_svc_lp_init is not allowed after cy_svc_deinit API is invoked");
        goto CLEAN_RETURN;
    }

    if (true == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_ALREADY_INITIALIZED;
        cy_svc_log_err(ret_val, "cy_svc_lp_init already initialized");
        goto CLEAN_RETURN;
    }

#if COMPONENT_MEMORY_CHECK_UTILS
#if (ENABLE_SVC_DEBUG & ENABLE_SVC_DBG_MALLOC_INFO)
    cy_mem_get_allocated_memory("Start => cy_svc_lp_init");
#endif
#endif


    ret_val = svc_lp_create_resource_for_instance(lp_instance, init);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        cy_svc_log_info("cy_svc_lp_init success, SVC ret base: 0x%x",
                (unsigned int) CY_RSLT_SVC_ERR_BASE);
        lp_instance->init_done = true;
        lp_instance->api_set_allowed = true;
    }
    else
    {
        memset(lp_instance, 0, sizeof(*lp_instance));
        cy_svc_log_err(ret_val, "cy_svc_lp_init fail");
    }

#if COMPONENT_MEMORY_CHECK_UTILS
#if (ENABLE_SVC_DEBUG & ENABLE_SVC_DBG_MALLOC_INFO)
    cy_mem_get_allocated_memory("End => cy_svc_lp_init");
#endif
#endif

    CLEAN_RETURN: return ret_val;
}

/**
 * Get the current stage of staged voice control module.
 *
 * @param[out]  stage                staged voice control module's current stage
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_get_current_stage(cy_svc_stage_t *stage)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif

    lp_instance = svc_lp_get_instance();

    if (false == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "SVC LP is not yet initialized");
        goto CLEAN_RETURN;
    }

    if(false == lp_instance->api_set_allowed)
    {
        ret_val = CY_RSLT_SVC_INVALID_STATE;
        cy_svc_log_err_on_no_isr(ret_val, "get stage not allowed");
        goto CLEAN_RETURN;
    }

    if (NULL == stage)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err_on_no_isr(ret_val, "Null argument %p", stage);
        goto CLEAN_RETURN;
    }

    /**
     * Mutex protection is not added intentionally, as all the cy_svc_lp_set_stage
     * set stage apis are synchronized with process. Hence it may not be required
     */
    *stage = lp_instance->current_stage;
    ret_val = CY_RSLT_SUCCESS;

    CLEAN_RETURN:
#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    return ret_val;
}

/**
 * SVC-LP application can force the stage in SVC module.
 *
 * @param[in] stage                 Required stage to set.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_set_stage(cy_svc_stage_t stage)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;
    bool valid_stage = false;

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif

    lp_instance = svc_lp_get_instance();

    cy_svc_log_info("SetStage:%d", stage);

    if (false == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "SVC LP is not yet initialized");
        goto CLEAN_RETURN;
    }

    if(false == lp_instance->api_set_allowed)
    {
        ret_val = CY_RSLT_SVC_INVALID_STATE;
        cy_svc_log_err_on_no_isr(ret_val, "Set stage not allowed");
        goto CLEAN_RETURN;
    }

    switch (stage)
    {
        case CY_SVC_STAGE_WAITING_FOR_ACOUSTIC_ACTIVITY:
        {
            cy_svc_log_err(ret_val,
                    "TODO: Post AAD wait data id to data Q. Empty the data Q");
            /**
             * TODO: Post a AAD wait data id to data Q. Empty the data Q till
             * and then fallback to AAD stage.
             *
             * TODO: To be discussed, this API for this requirement
             * to be synchronous or async and it depends on the application
             * requirement on timings to put LP core to deep sleep.
             */
            valid_stage = false;
            break;
        }
        case CY_SVC_STAGE_WAITING_FOR_SPEECH_ONSET_DETECTION:
        {
            if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_SOD)
            {
                valid_stage = false;
            }
            break;
        }
        case CY_SVC_STAGE_WAITING_FOR_LOW_POWER_WAKEUP_WORD_DETECTION:
        {
            if (lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_LPWWD)
            {
                valid_stage = false;
            }
            break;
        }

        case CY_SVC_STAGE_WAITING_FOR_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTION:
        {
            if (lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS)
            {
                valid_stage = false;
            }
            break;
        }
        case CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT:
        {
            if (lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS)
            {
                valid_stage = true;
            }
            break;
        }
        case CY_SVC_STAGE_ASR_PROCESSING_QUERY_DETECTED:
        {
            if (lp_instance->init_params.stage_config_list
                    & CY_SVC_ENABLE_ASR_STATE_TRANSITIONS)
            {
                valid_stage = false;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (false == valid_stage)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Invalid stage[%d], cfg-lp[0x%x]", stage,
                lp_instance->init_params.stage_config_list);
        goto CLEAN_RETURN;
    }

    ret_val = svc_lp_post_to_cmd_q_to_set_stage_from_lp_app(lp_instance, stage);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        (void) cy_rtos_get_semaphore(&lp_instance->cmdProcSyncSemaphore,
        CY_RTOS_NEVER_TIMEOUT, is_in_isr());
        /**
         * The result is a volatile variable and it will be updated by the SVC
         * task after processing the set stage request.
         */
        if (CY_RSLT_SUCCESS != lp_instance->lp_app_set_stage_process_result)
        {
            cy_svc_log_err(lp_instance->lp_app_set_stage_process_result,
                    "set stage fail");
        }
        else
        {
            cy_svc_log_info("set stage success");
        }

        ret_val = lp_instance->lp_app_set_stage_process_result;
    }

    CLEAN_RETURN:
#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    return ret_val;
}

/**
 * Feed input audio data to staged voice control module in low power domain.
 *
 * @param[in]  data                 Pointer to the input audio data.
 *
 *                                  Application can free/reuse the buffer pointer
 *                                  passed in this API once the API is returned.
 *                                  (i.e., Staged voice module will make a copy
 *                                  of the input data internally in its buffer).
 *                                  Length of the input audio data passed in the
 *                                  buffer is constant should not vary for every
 *                                  feed.(i.e, Application should feed always the
 *                                  complete buffer).
 *
 *                                  If Application needs to feed stereo data,
 *                                  then the data format must be non-interleaved
 *                                  format. example: For stereo data of 10ms
 *                                  worth, first 320bytes must be of channel-1
 *                                  and then next 320bytes must be channel-2.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_feed(
CY_SVC_DATA_T *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    lp_instance = svc_lp_get_instance();

    if (false == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err_on_no_isr(ret_val, "SVC LP is not yet initialized");
        goto CLEAN_RETURN;
    }

    if(false == lp_instance->api_set_allowed)
    {
        ret_val = CY_RSLT_SVC_INVALID_STATE;
        cy_svc_log_err_on_no_isr(ret_val, "feed not allowed");
        goto CLEAN_RETURN;
    }

    if (NULL == data)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err_on_no_isr(ret_val, "Null argument");
        goto CLEAN_RETURN;
    }

    // Check if license is expired
    if( true == cy_avc_lib_is_license_expired())
    {
        ret_val = CY_RSLT_SVC_LICENSE_EXPIRED;
        cy_svc_log_err_on_no_isr(ret_val, "License expired");
        goto CLEAN_RETURN;
    }

    ret_val = svc_lp_feed_data_to_cbuf_and_notify_task(lp_instance, data);
    if (CY_RSLT_SUCCESS != ret_val)
    {
//        cy_svc_log_err_on_no_isr(ret_val,
//                "svc_lp_feed_data_to_cbuf_and_notify_task fail");
    }

    CLEAN_RETURN:
#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    return ret_val;
}

/**
 * Deinit the staged voice control module in low power device, deletes the
 * resources created during \ref cy_svc_lp_init.
 *
 * @param none
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_deinit(void)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;

#if COMPONENT_MEMORY_CHECK_UTILS
#if (ENABLE_SVC_DEBUG & ENABLE_SVC_DBG_MALLOC_INFO)
    cy_mem_get_allocated_memory("Start => cy_svc_lp_deinit");
#endif
#endif

    lp_instance = svc_lp_get_instance();

    lp_instance->api_set_allowed = false;

    if (false == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "cy_svc_lp_init is not initialized");
        goto CLEAN_RETURN;
    }

    ret_val = svc_lp_delete_resource_for_instance(lp_instance);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        memset(lp_instance, 0, sizeof(*lp_instance));
        lp_instance->is_deinit_done = true;
        cy_svc_log_info("cy_svc_lp_deinit success");
    }
    else
    {
        cy_svc_log_err(ret_val, "cy_svc_lp_deinit fail");
    }

#if COMPONENT_MEMORY_CHECK_UTILS
#if (ENABLE_SVC_DEBUG & ENABLE_SVC_DBG_MALLOC_INFO)
    cy_mem_get_allocated_memory("End => cy_svc_lp_deinit");
#endif
#endif

    svc_alloc_memory = NULL;
    svc_free_memory = NULL;

    CLEAN_RETURN: return ret_val;
}

/**
 * Sets the info of the HP core. This is needed to synchronize the
 * IPC communication between SVC LP and SVC HP core.
 *
 * @param[in]  core_info                HP core information
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_set_hp_core_info(cy_svc_lp_set_hp_core_info_t core_info)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_instance_t *lp_instance = NULL;

    lp_instance = svc_lp_get_instance();

    if (false == lp_instance->init_done)
    {
        ret_val = CY_RSLT_SVC_NOT_INITIALIZED;
        cy_svc_log_err(ret_val, "cy_svc_lp_init is not initialized");
        goto CLEAN_RETURN;
    }

    switch (core_info)
    {
        case CY_SVC_BEFORE_HP_CORE_ENABLE:
        {
            cy_svc_log_info("BEFORE_HP_CORE_ENABLE");
            (void) svc_lp_clear_svc_hp_ready_sync_sem();
            lp_instance->svc_hp_state = CY_SVC_LP_HP_CORE_STATE_ENABLE_TRIGERRED;
            lp_instance->config_update_to_hp_done = false;
            ret_val = CY_RSLT_SUCCESS;
            break;
        }

        case CY_SVC_BEFORE_HP_CORE_DISABLE:
        {
            cy_svc_log_info("BEFORE_HP_CORE_DISABLE");
            lp_instance->svc_hp_state = CY_SVC_LP_HP_CORE_STATE_DISABLE_TRIGERRED;
            ret_val = CY_RSLT_SUCCESS;
            break;
        }
        default:
        {
            ret_val = CY_RSLT_SVC_BAD_ARG;
            break;
        }
    }
    CLEAN_RETURN:
    return ret_val;
}

#endif
