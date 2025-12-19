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
 * @file staged_voice_control_hp_ipc.c
 *
 */

#ifdef ENABLE_SVC_HP_MW
#include "staged_voice_control_ipc.h"
#include "staged_voice_control_hp_ipc.h"
#include "staged_voice_control_hp_thread.h"
#include "staged_voice_control_hp_hal_ipc.h"

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

svc_hp_instance_t global_hp_instance = { 0 };

__WEAK void cy_svc_hp_ipc_pre_send_hook(void)
{
    // cy_svc_log_dbg("[IPC-HP] pre_send_hook");
}

__WEAK void cy_svc_hp_ipc_post_send_hook(void)
{
    // cy_svc_log_dbg("[IPC-HP] pre_send_hook");
}

__WEAK void cy_svc_hp_ipc_post_recv_hook(void)
{

}

/**
 * Saves the config information from SVC LP.
 *
 * @param[in]  config_info      config information
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
static cy_rslt_t svc_hp_set_state_result_from_lp(
        svc_hp_instance_t *hp_instance,
        ipc_cmd_lp_to_hp_set_state_result_t *set_state_result)
{
    cy_rslt_t result = CY_RSLT_SVC_GENERIC_ERROR;

    hp_instance->lp_app_set_state_process_result =
            set_state_result->set_state_result;

    if(set_state_result->cur_stage != hp_instance->current_stage)
    {
        hp_instance->current_stage = set_state_result->cur_stage;
    }
    (void) cy_rtos_set_semaphore(&hp_instance->cmdProcSyncSVCLPSemaphore,
            is_in_isr());

    result = CY_RSLT_SUCCESS;
//    cy_svc_log_err_on_no_isr(result,
//            "Config update: 0x%x",hp_instance->stage_config_list_from_lp);
    return result;
}


/**
 * Saves the config information from SVC LP.
 *
 * @param[in]  config_info      config information
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
static cy_rslt_t svc_hp_save_frame_information_from_svc_lpk(
        svc_hp_instance_t *hp_instance,
        ipc_cmd_lp_to_hp_cfg_update_t *config_info)
{
    cy_rslt_t result = CY_RSLT_SVC_GENERIC_ERROR;

    hp_instance->stage_config_list_from_lp = config_info->stage_config_list;

    memcpy(&hp_instance->circular_shared_buffer,
            (svc_circular_buffer_header_t*) config_info->cbuf_information,
            sizeof(hp_instance->circular_shared_buffer));

    result = CY_RSLT_SUCCESS;
//    cy_svc_log_err_on_no_isr(result,
//            "Config update: 0x%x",hp_instance->stage_config_list_from_lp);
    return result;
}


#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE

#define IPC_BUSY_RETRY_COUNT (2)
#define IPC_BUSY_RETRY_SLEEP_INTERNAL_MS (5)

void svc_hp_ipc_send_done_callback(void)
{
    svc_hp_instance_t *hp_instance = svc_hp_get_instance();

    (void) cy_rtos_set_semaphore(&hp_instance->cmdProcIPCSendSemaphore,
            is_in_isr());
    return;
}

void svc_hp_ipc_wait_for_send_complete(void)
{
    svc_hp_instance_t *hp_instance = svc_hp_get_instance();

    (void) cy_rtos_get_semaphore(&hp_instance->cmdProcIPCSendSemaphore,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());

    return;
}



/**
 * Generic utility to send IPC message through IPC
 *
 * @param[in]  data      data to be sent through IPC
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
#ifdef COMPONENT_CM55
CY_SECTION_SHAREDMEM
#endif
ipc_msg_t ipc_msg_send = {0};


cy_rslt_t svc_hp_ipc_send_msg_to_lp(uint8_t *data)
{
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t cmd_id = 0;
    svc_hp_instance_t *hp_instance = svc_hp_get_instance();

    unsigned int retry = IPC_BUSY_RETRY_COUNT;

    memcpy(ipc_msg_send.msg_pay, data, sizeof(ipc_msg_send.msg_pay));
    cmd_id = ipc_msg_send.msg_pay[0];

    ipc_msg_send.client_id = (uint8_t)hp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_client_id;
    ipc_msg_send.intr_mask =  (uint16_t)hp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_intr_mask;

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    ipc_msg_send.msg_pay[1] = ++ipc_send_from_hp_to_lp_counter;
#endif

    cy_svc_hp_ipc_pre_send_hook();

    do
    {
        ipc_status = Cy_IPC_Pipe_SendMessage(
                hp_instance->init_params.ipc_pipe_config.lp_pipe_end_point_address,
                hp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_address,
                (void*) &ipc_msg_send,
                svc_hp_ipc_send_done_callback);
        if (CY_IPC_PIPE_SUCCESS != ipc_status)
        {
            retry--;
        }

        /**
         * Added, Retry to handle the error. 0x8a0207 error in IPC send.
         * Retry in case of busy is working and tested.
         */
    } while ((CY_IPC_PIPE_SUCCESS != ipc_status) && (retry > 0));

    if(CY_IPC_PIPE_SUCCESS == ipc_status)
    {
        svc_hp_ipc_wait_for_send_complete();
    }

    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        ret_val = CY_RSLT_SVC_IPC_SEND_FAIL;
        cy_svc_log_err(ipc_status, "[IPC] Send MSG [%d] fail", cmd_id);
        (void)cmd_id; //Warning unused varible fix if log is disabled.
    }
    else
    {
        ret_val = CY_RSLT_SUCCESS;
        cy_svc_log_info("[IPC] Send msg [%d] success", cmd_id);
    }

    cy_svc_hp_ipc_post_send_hook();

    return ret_val;
}
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
ipc_msg_t ipc_msg_send_msg_to_lp = { 0 };

cy_rslt_t svc_hp_ipc_send_msg_to_lp(
        uint8_t* data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    memcpy(&ipc_msg_send_msg_to_lp.msg_pay[0], data,
            sizeof(ipc_msg_send_msg_to_lp.msg_pay));

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    ipc_msg_send_msg_to_lp.msg_pay[1] = ++ipc_send_from_hp_to_lp_counter;
#endif

    ret_val = ipc_stub_post_msg_to_cm0(&ipc_msg_send_msg_to_lp);
    return ret_val;
}

#else
cy_rslt_t svc_hp_ipc_send_msg_to_lp(
        uint8_t* data)
{
    return svc_hp_ipc_send_msg_to_lp_using_hal_ipc(data);
}
#endif

/**
 * Sends the set state command to SVC LP through IPC
 *
 * @param[in]  set_state      state to set
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_hp_ipc_send_command_set_state(
        svc_hp_instance_t *hp_instance,
        cy_svc_set_state_t set_state,
        void *state_info)
{
    ipc_cmd_hp_to_lp_set_state_t ipc_set_state = { 0 };

    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    ipc_set_state.cmd_id = IPC_CMD_ID_HP_TO_LP_SET_STATE;
    ipc_set_state.set_state = set_state;

    if (NULL != state_info)
    {
        memcpy(ipc_set_state.payload_internal, state_info,
                sizeof(ipc_set_state.payload_internal));
    }

    ret_val = svc_hp_ipc_send_msg_to_lp((uint8_t*) &ipc_set_state);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "send msg to svc lp fail");
        goto CLEAN_RETURN;
    }

    (void) cy_rtos_get_semaphore(&hp_instance->cmdProcSyncSVCLPSemaphore,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());

    /**
     * The result is a volatile variable and it will be updated by the SVC
     * task after processing the set state request.
     */
    if (CY_RSLT_SUCCESS != hp_instance->lp_app_set_state_process_result)
    {
        cy_svc_log_err(hp_instance->lp_app_set_state_process_result,
                "set state fail");
    }
    else
    {
        cy_svc_log_info("set state success in core svc-lp");
    }

    ret_val = hp_instance->lp_app_set_state_process_result;

    CLEAN_RETURN: return ret_val;
}



/**
 * Sends the set state command to SVC LP through IPC
 *
 * @param[in]  set_state      state to set
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_hp_ipc_send_command_ready_state(svc_hp_instance_t *hp_instance)
{
    ipc_cmd_hp_to_lp_ready_state_t ipc_ready_state = { 0 };

    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    ipc_ready_state.cmd_id = IPC_CMD_ID_HP_TO_LP_READY_STATE;

    ret_val = svc_hp_ipc_send_msg_to_lp((uint8_t*) &ipc_ready_state);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "send msg to svc lp fail");
        goto CLEAN_RETURN;
    }

    CLEAN_RETURN:
    return ret_val;
}

/**
 * Receives the msg callback from IPC
 *
 * @param[in]  msg      Pointer to message. In PSoC6, the length of the
 * message is assume to be of structure size ipc_msg_t. This structure
 * needs to be defined for Explore IPC (recommeded to have size as 12 bytes)
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
void svc_hp_ipc_msg_receive_callback(uint32_t *msg)
{
    ipc_cmd_lp_to_hp_data_t *ipc_circular = NULL;
    ipc_cmd_lp_to_hp_cfg_update_t *config_info = NULL;
    ipc_cmd_lp_to_hp_set_state_result_t *set_state_rslt = NULL;
    svc_hp_instance_t *hp_instance = NULL;
    uint8_t cmd_id = 0;
    uint8_t *pay_load = NULL;
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    uint8_t ipc_recv_counter = 0;
#endif

    cy_svc_hp_ipc_post_recv_hook();

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
        ipc_recv_counter = ((uint8_t *)msg)->msg_pay[1];
#endif
#else
        cmd_id = *((uint8_t *)msg);
        pay_load = (uint8_t *)(msg);
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
        ipc_recv_counter = *((uint8_t *)((uint8_t *)msg + 1) );
#endif
#endif

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK

        if((ipc_recv_from_lp_to_hp_counter+1) != ipc_recv_counter)
        {
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK_LOG
//        	cy_svc_log_err(CY_RSLT_SVC_IPC_COUNTER_MISMATCH, "HP-IPC RCV COUNT mismatch, Cur:%d, Prev:%d",
//        			ipc_recv_counter, ipc_recv_from_lp_to_hp_counter);
            printf("\r\nError !! HP-IPC RCV COUNT mismatch, Cur:%d, Prev:%d\r\n",
                    ipc_recv_counter, ipc_recv_from_lp_to_hp_counter);
#endif
        }
        else
        {
#ifdef ENABLE_SVC_IPC_COUNTER_TRACK_LOG
            cy_svc_log_info("HP-IPC RCV COUNT:%d",ipc_recv_counter);
#endif
        }

        ipc_recv_from_lp_to_hp_counter = ipc_recv_counter;
#endif

        hp_instance = svc_hp_get_instance();

        if (false == hp_instance->init_done)
        {
            cy_svc_log_err(CY_RSLT_SVC_NOT_INITIALIZED
                    , "[IPC] Err !! Drop MSG. HP init not done");
            return;
        }

        if (IPC_CMD_ID_LP_TO_HP_CIRCULAR_BUF_UPDATE == cmd_id)
        {
            ipc_circular = (ipc_cmd_lp_to_hp_data_t *)pay_load;

            hp_instance->current_stage = ipc_circular->current_stage;

#ifdef ENABLE_TASK_FOR_SVC_HP

            if (0 != ipc_circular->insuff_preroll_frame_count)
            {
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                (void) svc_hp_push_to_data_queue_from_ipc_cbk(
                        svc_hp_get_instance(),
                        (CY_SVC_DATA_T*) NULL,
                        (uint32_t) ipc_circular->insuff_preroll_frame_count,
                        (cy_svc_buffer_info_t) ipc_circular->data_info_bitmask |
                        CY_SVC_BUF_INFO_PREROLL_INSUFFICIENT_BUF);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                (void) svc_hp_push_to_data_queue_from_ipc_cbk(
                        svc_hp_get_instance(),
                        (CY_SVC_DATA_T*) ipc_circular->data_pointer,
                        (uint32_t) ipc_circular->frame_count,0);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            }
            else
            {
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                (void) svc_hp_push_to_data_queue_from_ipc_cbk(
                        svc_hp_get_instance(),
                        (CY_SVC_DATA_T*) ipc_circular->data_pointer,
                        (uint32_t) ipc_circular->frame_count,
                        (cy_svc_buffer_info_t) ipc_circular->data_info_bitmask);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            }


#else
            if (0 != ipc_circular->insuff_preroll_frame_count)
            {
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                hp_instance->init_params.data_callback(
                    (CY_SVC_DATA_T*) NULL,
                    ipc_circular->insuff_preroll_frame_count,
                    ipc_circular->data_info_bitmask |
                        CY_SVC_BUF_INFO_PREROLL_INSUFFICIENT_BUF,
                    hp_instance->init_params.callback_user_arg);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif

                cy_svc_log_info("DataPtr:0x%x Fcnt:%d bm:0x%x PreRolIns:%d",ipc_circular->data_pointer,
                         ipc_circular->frame_count,
                        ipc_circular->data_info_bitmask |
                        CY_SVC_BUF_INFO_PREROLL_INSUFFICIENT_BUF,
                        ipc_circular->insuff_preroll_frame_count);

#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                hp_instance->init_params.data_callback(
                    (CY_SVC_DATA_T*) ipc_circular->data_pointer,
                    (uint32_t) ipc_circular->frame_count,
                    0,
                    hp_instance->init_params.callback_user_arg);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            }
            else
            {
                cy_svc_log_info("DataPtr:0x%x, Fcnt:%d, bm:0x%x",ipc_circular->data_pointer,
                         ipc_circular->frame_count,
                         ipc_circular->data_info_bitmask);

#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
                hp_instance->init_params.data_callback(
                    (CY_SVC_DATA_T*) ipc_circular->data_pointer,
                    (uint32_t) ipc_circular->frame_count,
                    (cy_svc_buffer_info_t) ipc_circular->data_info_bitmask,
                    hp_instance->init_params.callback_user_arg);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            }
#endif
        }
        else if (IPC_CMD_ID_LP_TO_HP_SET_STATE_RESULT == cmd_id)
        {
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            set_state_rslt = (ipc_cmd_lp_to_hp_set_state_result_t *)pay_load;
            (void) svc_hp_set_state_result_from_lp(
                    svc_hp_get_instance(),
                    set_state_rslt);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
        }
        else if (IPC_CMD_ID_LP_TO_HP_CONFIG_UPDATE == cmd_id)
        {
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
            config_info = (ipc_cmd_lp_to_hp_cfg_update_t *)pay_load;
            (void) svc_hp_save_frame_information_from_svc_lpk(
                    svc_hp_get_instance(),
                    config_info);
#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
                SVC_HP_CHECK_POINT_ISR()
#endif
        }
#ifdef ENABLE_APP_SIMULATION_FOR_EVENTS
        else if ((IPC_CMD_ID_SIMULATION_START <= cmd_id) &&
                (IPC_CMD_ID_SIMULATION_END >= cmd_id))
        {
            extern cy_rslt_t cy_svc_hp_app_simulation_app_callback_t(
                    uint8_t cmd_id,
                    uint8_t *data);

            (void) cy_svc_hp_app_simulation_app_callback_t(cmd_id,
                    pay_load);
        }
#endif
    }
}


#ifndef ENABLE_LOW_POWER_SVC
void svc_hp_hal_ipc_sem_init(cy_svc_hp_config_t *init)
{
    svc_hp_instance_t *hp_instance = NULL;
    hp_instance = svc_hp_get_instance();

    //init semaphore
#ifdef SVC_HAL_IPC
    (void)cyhal_ipc_semaphore_init(&hp_instance->semaphore1,
            init->ipc_communication_sync_sempahore, false);
#else
    hp_instance->semaNumber = init->ipc_communication_sync_sempahore;
#endif
    return;
}

void svc_hp_hal_ipc_signal_lp_to_start(void)
{
    svc_hp_instance_t *hp_instance = NULL;
    hp_instance = svc_hp_get_instance();

#ifdef SVC_HAL_IPC
    cy_svc_log_info("Signal LP to start...");
    cyhal_ipc_semaphore_give(&hp_instance->semaphore1);
    cy_svc_log_info("Signal LP to start done");
#else
    /* Wait for semaphore be locked in a loop */
    while (CY_IPC_SEMA_STATUS_LOCKED != Cy_IPC_Sema_Status(hp_instance->semaNumber))
    {
        cy_rtos_delay_milliseconds(IPC_SYNC_SEM_CHECK_DELAY_MS);
        cy_svc_log_dbg("HP> Not yet locked from LP");
    }

    /* unlock the IPC semaphore */
    Cy_IPC_Sema_Clear(hp_instance->semaNumber, false);

    cy_svc_log_dbg("Cm55> signalled");
#endif
}

void svc_hp_hal_ipc_sem_deinit( )
{
#ifdef SVC_HAL_IPC
    svc_hp_instance_t *hp_instance = NULL;
    hp_instance = svc_hp_get_instance();

    //init semaphore
    (void)cyhal_ipc_semaphore_free(&hp_instance->semaphore1);
#endif
    return;
}
#endif


/**
 * Initializes the IPC
 *
 * @param[in]  hp_instance      HP instance
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_hp_ipc_init(svc_hp_instance_t *hp_instance, cy_svc_hp_config_t *init)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

#ifndef ENABLE_LOW_POWER_SVC
    svc_hp_hal_ipc_sem_init(init);
#endif

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;

    ipc_status = Cy_IPC_Pipe_RegisterCallback(
            init->ipc_pipe_config.hp_pipe_end_point_address,
            &svc_hp_ipc_msg_receive_callback,
            init->ipc_pipe_config.hp_pipe_end_point_client_id);

    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        cy_svc_log_err(ipc_status, "[IPC] register fail");
    }
    else
    {
        ret_val = CY_RSLT_SUCCESS;
    }
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
    cy_svc_log_dbg("IPC stub on same core");
    ret_val = CY_RSLT_SUCCESS;
#else
    ret_val = svc_hp_create_hal_ipc_resources( hp_instance, init);
    if(CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "[IPC] create hal ipc rsrc fail");
    }
    else
    {
        cy_svc_log_info("[IPC] hal ipc rscr create success");
    }
#endif

    return ret_val;
}

/**
 * Deinitializes the IPC
 *
 * @param[in]  hp_instance      HP instance
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t svc_hp_ipc_deinit(svc_hp_instance_t *hp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_en_ipc_pipe_status_t ipc_status = CY_IPC_PIPE_SUCCESS;

    if((0 == hp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_address) &&
        (0 == hp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_client_id))
    {
        cy_svc_log_dbg("IPC deinit is not required");
        return CY_RSLT_SUCCESS;
    }

    ipc_status = Cy_IPC_Pipe_RegisterCallback(
            hp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_address, NULL,
            hp_instance->init_params.ipc_pipe_config.hp_pipe_end_point_client_id);
    if (CY_IPC_PIPE_SUCCESS != ipc_status)
    {
        cy_svc_log_err(ipc_status, "[IPC] deregister fail");
    }
    else
    {
        ret_val = CY_RSLT_SUCCESS;
    }
#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
    cy_svc_log_dbg("IPC deinit stub on same core success");
    ret_val = CY_RSLT_SUCCESS;
#else
    ret_val = svc_hp_delete_hal_ipc_resources(hp_instance);
    if(ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "delete hp hal ipc rsrc fail");
    }
#endif

#ifndef ENABLE_LOW_POWER_SVC
    svc_hp_hal_ipc_sem_deinit();
#endif
    return ret_val;
}

/**
 * Gets the HP instance handle
 *
 * @param none
 *
 * @return    HP instance handle
 */
svc_hp_instance_t* svc_hp_get_instance(void)
{
    return &global_hp_instance;
}

#ifndef COMPONENT_IPC_SINGLE_CORE_STUB
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

#endif
