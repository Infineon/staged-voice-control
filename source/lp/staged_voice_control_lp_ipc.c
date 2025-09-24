/*
 * Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/**
 * @file staged_voice_control_lp_ipc.c
 *
 */

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_ipc.h"
#include "staged_voice_control_lp_ipc.h"
#include "staged_voice_control_lp_queue.h"
#include "staged_voice_control_lp_hal_ipc.h"

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

#ifndef ENABLE_LOW_POWER_SVC
void svc_lp_ipc_sync_with_hp_sem_deinit(void)
{
#ifdef SVC_HAL_IPC
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();
    //init semaphore
    (void)cyhal_ipc_semaphore_free(&lp_instance->semaphore0);
#endif
    return;
}


void svc_lp_ipc_sync_with_hp_sem_init(cy_svc_lp_config_t *init )
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();
    //init semaphore
#ifdef SVC_HAL_IPC
    (void)cyhal_ipc_semaphore_init(&lp_instance->semaphore0,
            init->ipc_communication_sync_sempahore, false);
#else
    lp_instance->semaNumber = init->ipc_communication_sync_sempahore;
#endif
    return;
}

void svc_lp_wait_for_sem_signal_sync_from_hp(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();
    //semaphore_take - wait for signal from CM4
#ifdef SVC_HAL_IPC
    if (cyhal_ipc_semaphore_take(&lp_instance->semaphore0, CYHAL_IPC_NEVER_TIMEOUT) != CY_RSLT_SUCCESS)
    {
        return;
    }
#else
    /* Lock the semaphore */
    Cy_IPC_Sema_Set(lp_instance->semaNumber, false);

    /* wait for semaphore be released in a loop */
    while (CY_IPC_SEMA_STATUS_UNLOCKED != Cy_IPC_Sema_Status(lp_instance->semaNumber))
    {
        cy_rtos_delay_milliseconds(IPC_SYNC_SEM_CHECK_DELAY_MS);
        cy_svc_log_dbg("LP> not yet released from wait");
    }
    cy_svc_log_dbg("LP> released from wait");
#endif
}

#endif

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
/**
 * Adding global variable to avoid the memory in stack as this API will be
 * called repeatedly from single task.
 */
#ifdef COMPONENT_CM33
CY_SECTION_SHAREDMEM
#endif
ipc_msg_t ipc_msg_send_msg = { 0 };

#define IPC_BUSY_RETRY_COUNT (2)
#define IPC_BUSY_RETRY_SLEEP_INTERNAL_MS (5)

__WEAK void cy_svc_lp_ipc_pre_send_hook(void)
{
    // cy_svc_log_dbg("[IPC-LP] pre_send_hook");
}

__WEAK void cy_svc_lp_ipc_post_send_hook(void)
{
    // cy_svc_log_dbg("[IPC-LP] pre_send_hook");
}

__WEAK void cy_svc_lp_ipc_post_recv_hook(void)
{

}

void svc_lp_clear_svc_hp_ready_sync_sem(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    (void) cy_rtos_get_semaphore(&lp_instance->cmdProcIPCSendSemaphore,
    0, is_in_isr());
}

void svc_lp_wait_for_hp_ready_complete(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    (void) cy_rtos_get_semaphore(&lp_instance->cmdProcIPCSendSemaphore,
            SVC_LP_WAIT_FOR_HP_READY_TIMEOUT, is_in_isr());

    return;
}

void svc_lp_notify_hp_ready_msg(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    (void) cy_rtos_set_semaphore(&lp_instance->cmdProcIPCSendSemaphore,
            is_in_isr());
    return;
}

void svc_lp_ipc_send_done_callback(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    (void) cy_rtos_set_semaphore(&lp_instance->cmdProcIPCSendSemaphore,
            is_in_isr());
    return;
}

void svc_lp_ipc_wait_for_send_complete(void)
{
    svc_lp_instance_t *lp_instance = svc_lp_get_instance();

    (void) cy_rtos_get_semaphore(&lp_instance->cmdProcIPCSendSemaphore,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());

    return;
}

cy_rslt_t svc_lp_send_ipc_msg_to_hp(
        svc_lp_instance_t *lp_instance,
        uint8_t* data)
{
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;
    uint32_t retry = IPC_BUSY_RETRY_COUNT;
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t cmd_id = 0;

#ifdef ENABLE_LOW_POWER_SVC

    if(CY_SVC_LP_HP_CORE_STATE_ENABLE_TRIGERRED
            == lp_instance->svc_hp_state )
    {
        cy_svc_log_info("Waiting for HP ready complete");
    	svc_lp_wait_for_hp_ready_complete( );

    	if(CY_SVC_LP_HP_CORE_STATE_SVC_HP_READY
    	        == lp_instance->svc_hp_state)
    	{
    	    cy_svc_log_info("Waiting for HP ready received");
    	}
    	else
    	{
            //Skipped sending IPC as other is not ready.
            cy_svc_log_info("HP ready timeout");
    	    lp_instance->svc_hp_state  = CY_SVC_LP_HP_CORE_STATE_DISABLE_TRIGERRED;
            return CY_RSLT_SUCCESS;
        }
    }
    else  if(CY_SVC_LP_HP_CORE_STATE_DISABLE_TRIGERRED ==
            lp_instance->svc_hp_state )
    {
        //Skipped sending IPC as other is not ready.
        cy_svc_log_info("HP disabled, Skipping IPC message");
        return CY_RSLT_SUCCESS;
    }
    else  if(CY_SVC_LP_HP_CORE_STATE_SVC_HP_READY ==
            lp_instance->svc_hp_state )
    {
        //cy_svc_log_info("HP Read, Ok to send IPC");
    }
#endif

    memcpy(&ipc_msg_send_msg.msg_pay[0], data,
            sizeof(ipc_msg_send_msg.msg_pay));

    cmd_id = ipc_msg_send_msg.msg_pay[0];


    ipc_msg_send_msg.client_id = (uint8_t)lp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_client_id;
    ipc_msg_send_msg.intr_mask =  (uint16_t)lp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_intr_mask;

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    ipc_msg_send_msg.msg_pay[1] = ++ipc_send_from_lp_to_hp_counter;
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    /* Pre-send hook for IPC communication in low power mode. */
    cy_svc_lp_ipc_pre_send_hook();

    do
    {
        ipc_status = Cy_IPC_Pipe_SendMessage(
                lp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_address,
                lp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_address,
                (void *) &ipc_msg_send_msg,
                svc_lp_ipc_send_done_callback);
        if (CY_IPC_PIPE_SUCCESS != ipc_status)
        {
            retry--;
        }

        /**
         * Added, Retry to handle the error. 0x8a0207 error in IPC send.
         * Retry in case of busy is working and tested.
         */
    } while ((CY_IPC_PIPE_SUCCESS != ipc_status) && (retry > 0));

    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        cy_svc_log_err(ipc_status, "[IPC] LP SEND msg[%d] fail", cmd_id);
        (void)cmd_id; //Warning unused varible fix if log is disabled.
    }
    else
    {
        svc_lp_ipc_wait_for_send_complete();
        //  cy_svc_log_info("[IPC] LP SEND success");
        ret_val = CY_RSLT_SUCCESS;
    }

    /* Post-send hook for IPC communication in low power mode. */
    cy_svc_lp_ipc_post_send_hook();

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    return ret_val;
}
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE

ipc_msg_t ipc_msg_send_msg_to_hp = { 0 };

cy_rslt_t svc_lp_send_ipc_msg_to_hp(
        svc_lp_instance_t *lp_instance,
        uint8_t* data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    memcpy(&ipc_msg_send_msg_to_hp.msg_pay[0], data,
            sizeof(ipc_msg_send_msg_to_hp.msg_pay));

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    ipc_msg_send_msg_to_hp.msg_pay[1] = ++ipc_send_from_lp_to_hp_counter;
#endif

    ret_val = ipc_stub_post_msg_to_cm4(&ipc_msg_send_msg_to_hp);
    return ret_val;
}
#else
cy_rslt_t svc_lp_send_ipc_msg_to_hp(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
    return svc_lp_send_ipc_msg_to_hp_using_hal_ipc( lp_instance,data);
}
#endif

cy_rslt_t svc_lp_notify_svc_hp_for_state_execution_result(
        svc_lp_instance_t *lp_instance,

        cy_rslt_t set_state_execution_ret_val)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    ipc_cmd_lp_to_hp_set_state_result_t ipc_set_state_rslt = {0};

    ipc_set_state_rslt.cmd_id = IPC_CMD_ID_LP_TO_HP_SET_STATE_RESULT;
    ipc_set_state_rslt.set_state_result = set_state_execution_ret_val;
    ipc_set_state_rslt.cur_stage = lp_instance->current_stage;

    ret_val = svc_lp_send_ipc_msg_to_hp(lp_instance,
            (uint8_t*) &ipc_set_state_rslt);
    if (CY_RSLT_SUCCESS == ret_val)
    {
//        cy_svc_log_info("[IPC] cmdid:0x%x ,result:0x%x, stage:%d",
//                ipc_set_state_rslt.cmd_id, ipc_set_state_rslt.set_state_result,
//                ipc_set_state_rslt.cur_stage);
    }
    return ret_val;
}

cy_rslt_t svc_lp_send_ipc_config_update(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    ipc_cmd_lp_to_hp_cfg_update_t ipc_config_update = {0};

    ipc_config_update.cmd_id = IPC_CMD_ID_LP_TO_HP_CONFIG_UPDATE;

    ipc_config_update.stage_config_list =
            lp_instance->init_params.stage_config_list;
    ipc_config_update.cbuf_information =
            (char*) lp_instance->circular_shared_buffer;

    ret_val = svc_lp_send_ipc_msg_to_hp(lp_instance,
            (uint8_t *) &ipc_config_update);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        lp_instance->config_update_to_hp_done = true;
        cy_svc_log_info("[IPC] config update: 0x%x",
                ipc_config_update.stage_config_list);
    }

    return ret_val;
}

cy_rslt_t svc_lp_send_ipc_event_circular_buffer_update(
        svc_lp_instance_t *lp_instance,
        uint8_t *data_pointer,
        uint32_t data_len,
        uint16_t insuff_preroll_frame_count,
        cy_svc_buffer_info_t buffer_info)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    ipc_cmd_lp_to_hp_data_t ipc_circular_buffer = { 0 };

    if (false == lp_instance->config_update_to_hp_done)
    {
//        cy_svc_log_info("[IPC] performing config update");

        (void) svc_lp_send_ipc_config_update(lp_instance);

        IPC_SEND_GAP_DELAY_SIMULATION(10);
    }

    ipc_circular_buffer.cmd_id = IPC_CMD_ID_LP_TO_HP_CIRCULAR_BUF_UPDATE;
    ipc_circular_buffer.current_stage = lp_instance->current_stage;
    ipc_circular_buffer.data_pointer = data_pointer;
    ipc_circular_buffer.frame_count = (uint16_t)(
            data_len
                    / lp_instance->circular_shared_buffer->frame_size_in_bytes);
    ipc_circular_buffer.insuff_preroll_frame_count = insuff_preroll_frame_count;

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC
    (void) svc_lp_cbuf_marker_mark_ipc_send_buffer(
            lp_instance,ipc_circular_buffer.data_pointer,
            ipc_circular_buffer.frame_count);
#endif

    /**
     * TODO: Converting uint32_t to uint8_t knowingly. To be changed
     * once the IPC size increased.
     */
    ipc_circular_buffer.data_info_bitmask = (uint8_t) buffer_info;

    ret_val = svc_lp_send_ipc_msg_to_hp(lp_instance,
            (uint8_t *) &ipc_circular_buffer);

    return ret_val;
}


void svc_ipc_lp_msg_receive_callback(uint32_t *msg)
{
    ipc_cmd_hp_to_lp_set_state_t *ipc_set_state = NULL;
    uint8_t cmd_id = 0;
    uint8_t *pay_load = NULL;
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    uint8_t ipc_recv_counter = 0;
#endif

    /* Post-receive hook for IPC communication in low power mode. */
    cy_svc_lp_ipc_post_recv_hook();

    if (msg != NULL)
    {
#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
        cmd_id = ((ipc_msg_t *)msg)->msg_pay[0];
        pay_load = (uint8_t *)((ipc_msg_t *)msg)->msg_pay;
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
        ipc_recv_counter = ((ipc_msg_t *)msg)->msg_pay[1];
#endif
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
        cmd_id = ((ipc_msg_t *)msg)->msg_pay[0];
        pay_load = (uint8_t *)((ipc_msg_t *)msg)->msg_pay;
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
        ipc_recv_counter = ((ipc_msg_t *)msg)->msg_pay[1];
#endif
//        cy_svc_log_dbg("Received msg on LP");
#else
        cmd_id = *(uint8_t *)msg;
        pay_load = (uint8_t *)msg;
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
        ipc_recv_counter = *((uint8_t *)((uint8_t *)msg + 1) );
#endif
#endif

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
        if((ipc_recv_from_hp_to_lp_counter+1) != ipc_recv_counter)
        {
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK_LOG
//        	cy_svc_log_err(CY_RSLT_SVC_IPC_COUNTER_MISMATCH, "LP-IPC RCV COUNT mismatch, Cur:%d, Prev:%d",
//        			ipc_recv_counter, ipc_recv_from_hp_to_lp_counter);
            printf("\r\nError !!! LP-IPC RCV COUNT, Cur:%d, Prev:%d\r\n",
                    ipc_recv_counter, ipc_recv_from_hp_to_lp_counter);
#endif
        }
        else
        {
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK_LOG
            cy_svc_log_info("LP-IPC RCV COUNT:%d",ipc_recv_counter);
#endif
        }

        ipc_recv_from_hp_to_lp_counter = ipc_recv_counter;
#endif

        if (IPC_CMD_ID_HP_TO_LP_SET_STATE == cmd_id)
        {
            ipc_set_state = (ipc_cmd_hp_to_lp_set_state_t *)pay_load;

            (void) svc_lp_post_to_cmd_q_to_set_state_from_hp(svc_lp_get_instance(),
                    ipc_set_state->set_state,
                    ipc_set_state->payload_internal);
        }

        if (IPC_CMD_ID_HP_TO_LP_READY_STATE == cmd_id)
        {
            (void) svc_lp_process_set_ready_state_from_hp(svc_lp_get_instance());
        }
    }
}

cy_rslt_t svc_lp_ipc_init(svc_lp_instance_t *lp_instance, cy_svc_lp_config_t *init )
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

#ifndef ENABLE_LOW_POWER_SVC
    svc_lp_ipc_sync_with_hp_sem_init(init);
#endif

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;

    ipc_status = Cy_IPC_Pipe_RegisterCallback(
            init->ipc_pipe_config.lp_pipe_end_point_address,
            svc_ipc_lp_msg_receive_callback,
            init->ipc_pipe_config.lp_pipe_end_point_client_id);

    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        cy_svc_log_err(ipc_status, "[IPC] register fail");
    }
    else
    {
        ret_val = CY_RSLT_SUCCESS;
    }
    return ret_val;
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
    cy_svc_log_dbg("Enabled IPC stub on same core");
    ret_val = CY_RSLT_SUCCESS;
    return ret_val;
#else
    ret_val = svc_lp_create_hal_ipc_resources( init);
    if(ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "[IPC] HAL IPC create resource fail");
        goto CLEAN_RETURN;
    }
    cy_svc_log_info("[IPC] HAL IPC LP init success");

    CLEAN_RETURN:
    return ret_val;
#endif
}

cy_rslt_t svc_lp_ipc_deinit(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;

    ipc_status = Cy_IPC_Pipe_RegisterCallback(
            lp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_address,
            NULL,
            lp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_client_id);
    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        cy_svc_log_err(ipc_status, "[IPC] deregister fail");
    }
    else
    {
        cy_svc_log_info("IPC deinit success");
        ret_val = CY_RSLT_SUCCESS;
    }
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
    cy_svc_log_dbg("Deinit IPC stub");
    ret_val = CY_RSLT_SUCCESS;
#else
    cy_svc_log_dbg("TODO IPC deinit");
    ret_val = CY_RSLT_SUCCESS;
#endif

#ifndef ENABLE_LOW_POWER_SVC
    svc_lp_ipc_sync_with_hp_sem_deinit();
#endif

    return ret_val;
}

/** Checks to see if code is currently executing within an interrupt context.
 *
 * @return Boolean indicating whether this was executed from an interrupt context.
 */
bool is_in_isr()
#if 1
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}
#else
{
    #if defined(COMPONENT_CR4) // Can work for any Cortex-A & Cortex-R
    uint32_t mode = __get_mode();
    return (mode == 0x11U /*FIQ*/) || (mode == 0x12U /*IRQ*/) || (mode == 0x13U /*SVC*/) ||
           (mode == 0x17U /*ABT*/) || (mode == 0x1BU /*UND*/);
    #else // Cortex-M
    return (__get_IPSR() != 0);
    #endif
}
#endif

#endif
