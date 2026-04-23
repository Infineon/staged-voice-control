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
 * @file staged_voice_control_hp_thread.c
 *
 */

#ifdef ENABLE_SVC_HP_MW
#include "staged_voice_control_hp_private.h"
#include "staged_voice_control_ipc.h"
#include "staged_voice_control_hp_ipc.h"
#include "staged_voice_control_hp_thread.h"

svc_hp_check_points svc_hp_check_point = {0};

#ifdef ENABLE_TASK_FOR_SVC_HP

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


typedef struct __attribute__((packed, aligned(4)))
{
    uint8_t      *data_pointer;              /* Byte 1 - 4 - Data pointer */
    uint32_t     frame_count;                /* Byte 5 - 8 - Frame count */
    cy_svc_buffer_info_t     buf_info_bitmask;
} hp_thread_int_data_pointer;

cy_rslt_t svc_hp_get_from_data_queue_and_process(
        svc_hp_instance_t *hp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    hp_thread_int_data_pointer ipc_cbuf = {0};

    ret_val = cy_rtos_get_queue(&hp_instance->data_queue, &ipc_cbuf,
            CY_RTOS_NEVER_TIMEOUT, false);
    if (CY_RSLT_SUCCESS == ret_val)
    {
#ifdef ENABLE_SVC_HP_CHECK_POINT
    	SVC_HP_CHECK_POINT()
#endif
        //if (NULL != hp_instance->init_params.data_callback)
        {
            ret_val = hp_instance->init_params.data_callback(
                    (CY_SVC_DATA_T*) ipc_cbuf.data_pointer,
                    ipc_cbuf.frame_count, ipc_cbuf.buf_info_bitmask,
                    hp_instance->init_params.callback_user_arg);

            if (CY_RSLT_SUCCESS != ret_val)
            {
                cy_svc_log_err(ret_val, "data_callback failed");
            }
        }

#ifdef ENABLE_SVC_HP_CHECK_POINT
    	SVC_HP_CHECK_POINT()
#endif

#ifdef DBG_SVC_HP_BUFFER_TRACK_RECIEVE
        /**
         * Print needed statistics
         */
        if (buffer_dropped != buffer_dropped_in_isr)
        {
            buffer_dropped = buffer_dropped_in_isr;
            cy_svc_log_info("buffer_dropped:%ld", buffer_dropped);
        }
#endif
    }
    return ret_val;
}

cy_rslt_t svc_hp_push_to_data_queue_from_ipc_cbk(
        svc_hp_instance_t *hp_instance,
        CY_SVC_DATA_T *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buf_info_bitmask)
{
    cy_rslt_t result = CY_RSLT_SVC_GENERIC_ERROR;
    hp_thread_int_data_pointer data_info = {0};

    data_info.data_pointer = (uint8_t*) frame_buffer;
    data_info.frame_count = frame_count;
    data_info.buf_info_bitmask = buf_info_bitmask;

#if 0
    result = cy_rtos_put_queue(&hp_instance->data_queue, &data_info,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());
#else
    result = cy_rtos_put_queue(&hp_instance->data_queue, &data_info,
    0, is_in_isr());
#endif

    if (CY_RSLT_SUCCESS != result)
    {
        cy_svc_log_err_on_no_isr(result, "put q fail");
        return result;
    }
    else
    {
        ;
    }

    return result;
}

/**
 * stage voice thread process function
 *
 * @param[in]  thread_input             argument to thread.
 * @param[in]  create_domain        Create low power domain configuration
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
static void svc_hp_thread_func(cy_thread_arg_t thread_input)
{
    svc_hp_instance_t *hp_instance = (svc_hp_instance_t *) thread_input;

    cy_svc_log_info("Entering SVC thread proc");

    if (NULL != hp_instance)
    {
        cy_svc_log_info("proc SVC thread proc started");
#ifndef ENABLE_LOW_POWER_SVC
        svc_hp_hal_ipc_signal_lp_to_start();
#else
        svc_hp_ipc_send_command_ready_state(hp_instance);
#endif

        while (false == hp_instance->quit_thread_instance)
        {
            (void)  svc_hp_get_from_data_queue_and_process(hp_instance);
        }
    }

    cy_svc_log_info("Exiting SVC thread proc [%d]",
            hp_instance->quit_thread_instance);

    cy_rtos_exit_thread();
}

cy_rslt_t svc_hp_create_thread_resouces(svc_hp_instance_t *hp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    ret_val = cy_rtos_init_queue(&hp_instance->data_queue,
            MAX_HP_CMD_Q_SUPPORTED_SIZE, sizeof(hp_thread_int_data_pointer));
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "Create input data queue fail");
        goto CLEAN_RETURN;
    }

    ret_val = cy_rtos_create_thread(&hp_instance->thread_instance,
            svc_hp_thread_func,
            SVC_HP_THREAD_NAME, NULL, SVC_HP_THREAD_STACK_SIZE,
            SVC_HP_THREAD_PRIORITY, (cy_thread_arg_t) hp_instance);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "Unable to create the SVC thread");
        goto CLEAN_RETURN;
    }

    CLEAN_RETURN: return ret_val;
}

cy_rslt_t svc_hp_delete_thread_resouces(svc_hp_instance_t *hp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (NULL != hp_instance->thread_instance)
    {
        /* Delete thread */
        hp_instance->quit_thread_instance = true;

        ret_val = cy_rtos_terminate_thread(&hp_instance->thread_instance);
        if (ret_val != CY_RSLT_SUCCESS)
        {
            cy_svc_log_err(ret_val, "Terminate thread fail");

            /**
             * Note: Intentionally continuing to cleanup the other resources on fail,
             * since it is deallocate path. This is applicable for this function
             */
        }

        ret_val = cy_rtos_join_thread(&hp_instance->thread_instance);
        if (ret_val != CY_RSLT_SUCCESS)
        {
            cy_svc_log_err(ret_val, "Join thread fail");
        }
        else
        {
            cy_svc_log_dbg("Thread cleanup success");
        }
        hp_instance->thread_instance = NULL;
    }
    else
    {
        cy_svc_log_dbg("Thread is not yet created");
    }

    if (NULL != hp_instance->data_queue)
    {
        ret_val = cy_rtos_deinit_queue(&hp_instance->data_queue);
        if (ret_val != CY_RSLT_SUCCESS)
        {
            cy_svc_log_err(ret_val, "Deinit data Q fail");
        }
        else
        {
            cy_svc_log_info("Delete Data Q success");
        }
        hp_instance->data_queue = NULL;
    }
    else
    {
        cy_svc_log_info("Data Q is not created");
    }

    return ret_val;
}

#endif
#endif
