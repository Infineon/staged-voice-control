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
 * @file staged_voice_control_lp_queue.c
 *
 */
#ifdef ENABLE_SVC_LP_MW
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_aad.h"
#include "staged_voice_control_lp_queue.h"
#include "staged_voice_control_lp_process_data.h"
#include "staged_voice_control_profile.h"
/*******************************************************************************
 *                              Macros
 ******************************************************************************/

/*
 * Data Q timeout is infinite. SVC task will always wait for data in data Q.
 * */
#define DATA_QUEUE_GET_DATA_WAIT_TIMEOUT_MS (CY_RTOS_NEVER_TIMEOUT)

#define CMD_QUEUE_GET_DATA_WAIT_TIMEOUT_MS (0)

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

cy_rslt_t svc_lp_post_to_wakeup_msg_to_data_q(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    data_q_msg_t    q_msg = {0};

    q_msg.cmd = SVC_LP_CMD_ID_SET_STATE_FROM_HP;
    ret_val = cy_rtos_put_queue(&lp_instance->data_queue, &q_msg,
    0, is_in_isr());
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err_on_no_isr(ret_val, "data dummy input Q put fail");

        /*
         * TODO: Adding retry to post dummy msg in case of fail.
         */
    }
    else
    {
        //  cy_svc_log_info("data dummy input Q post success");
     }

    return ret_val;
}

cy_rslt_t svc_lp_process_set_ready_state_from_hp(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    if (NULL == lp_instance)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "NULL params");
        goto CLEAN_RETURN;
    }

    if(lp_instance->svc_hp_state ==
            CY_SVC_LP_HP_CORE_STATE_ENABLE_TRIGERRED)
    {
        lp_instance->svc_hp_state =
                CY_SVC_LP_HP_CORE_STATE_SVC_HP_READY;
        svc_lp_notify_hp_ready_msg();
    }
    else
    {
        ;
    }

    cy_svc_log_info("HP ready notification");

	ret_val = CY_RSLT_SUCCESS;
    CLEAN_RETURN: return ret_val;
}

cy_rslt_t svc_lp_post_to_cmd_q_to_set_state_from_hp(
        svc_lp_instance_t *lp_instance,
        cy_svc_set_state_t set_state,
        void *set_state_info)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cmd_q_msg_t q_msg = { 0 };

    q_msg.cmd = SVC_LP_CMD_ID_SET_STATE_FROM_HP;
    q_msg.set_state = set_state;
    memcpy(q_msg.payload_internal, set_state_info, sizeof(q_msg.payload_internal));

#ifdef ENABLE_SVC_LP_CHECK_POINT_ISR
    SVC_LP_CHECK_POINT_ISR();
#endif
    lp_instance->set_state_last_dbg = set_state;
    //cy_svc_log_info("Attempt to cmd input Q put");

    ret_val = cy_rtos_put_queue(&lp_instance->cmd_queue, &q_msg,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "cmd input Q put fail");
        goto CLEAN_RETURN;
    }
    else
    {
        cy_svc_log_info("Attempt to cmd input Q success");
    }

    (void) svc_lp_post_to_wakeup_msg_to_data_q(lp_instance);

#ifdef ENABLE_SVC_LP_CHECK_POINT_ISR
    SVC_LP_CHECK_POINT_ISR();
#endif

    CLEAN_RETURN: return ret_val;
}


cy_rslt_t svc_lp_post_to_cmd_q_to_set_stage_from_lp_app(
        svc_lp_instance_t *lp_instance,
        cy_svc_stage_t set_stage)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cmd_q_msg_t q_msg = { 0 };

    q_msg.cmd = SVC_LP_CMD_ID_SET_STAGE_FROM_LP_APP;
    q_msg.set_value = (uint32_t)set_stage;

    ret_val = cy_rtos_put_queue(&lp_instance->cmd_queue, &q_msg,
    CY_RTOS_NEVER_TIMEOUT, is_in_isr());
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "cmd input Q put fail");
        goto CLEAN_RETURN;
    }

    (void) svc_lp_post_to_wakeup_msg_to_data_q(lp_instance);

    CLEAN_RETURN: return ret_val;
}



cy_rslt_t svc_lp_get_from_msg_from_cmd_input_queue_and_process(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cmd_q_msg_t    q_msg = {0};
    cy_svc_stage_t temp_current_stage = CY_SVC_STAGE_INVALID;

    ret_val = cy_rtos_get_queue(&lp_instance->cmd_queue, &q_msg,
            CMD_QUEUE_GET_DATA_WAIT_TIMEOUT_MS, false);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        if(SVC_LP_CMD_ID_SET_STATE_FROM_HP ==  q_msg.cmd)
        {
#ifdef ENABLE_SVC_LP_CHECK_POINT_HP_MSG
        	SVC_LP_CHECK_POINT_HP_MSG();
#endif
            cy_svc_log_info("Received CMD Q msg: CMD %d,m State:%d", q_msg.cmd,
                    q_msg.set_state);

            temp_current_stage = lp_instance->current_stage;

            ret_val = svc_lp_trigger_state_from_hp_set_state(lp_instance,
                    q_msg.set_state,
                    q_msg.payload_internal);

#ifdef SIMULATE_SOD_IPC_TRIGGER_TEST
            ret_val = CY_RSLT_SUCCESS;
#endif

            (void) svc_lp_notify_svc_hp_for_state_execution_result(lp_instance,
                    ret_val);

            if (CY_RSLT_SUCCESS != ret_val)
            {
                cy_svc_log_err(ret_val, "set state from hp fail");
                goto CLEAN_RETURN;
            }
            else
            {
                cy_svc_log_info("setstate:%d,result:0x%x,cstage:[%d->%d]",
                        q_msg.set_state, (unsigned int)ret_val,
                        temp_current_stage,
                        lp_instance->current_stage);
                (void)(temp_current_stage);
            }

#ifdef ENABLE_SVC_LP_CHECK_POINT_HP_MSG
        	SVC_LP_CHECK_POINT_HP_MSG();
#endif
        }
        else if (SVC_LP_CMD_ID_SET_STAGE_FROM_LP_APP == q_msg.cmd)
        {
#ifdef ENABLE_SVC_LP_CHECK_POINT
        	SVC_LP_CHECK_POINT();
#endif
//            cy_svc_log_info("Received.. CMD Q msg: CMD %d,m Stage:%d", q_msg.cmd,
//                    q_msg.set_stage);

            ret_val = svc_lp_trigger_stage_from_lp_app_set_stage(lp_instance,
                    q_msg.set_stage);

            lp_instance->lp_app_set_stage_process_result = ret_val;

            (void) cy_rtos_set_semaphore(&lp_instance->cmdProcSyncSemaphore,
                    is_in_isr());

        	lp_instance->stage_force_start =  q_msg.set_stage;

#ifdef ENABLE_SVC_LP_CHECK_POINT
        	SVC_LP_CHECK_POINT();
#endif

//            cy_svc_log_info("Signal the cmd completion");
        }
    }

    CLEAN_RETURN:
    return ret_val;
}

cy_rslt_t svc_lp_get_data_from_data_queue_and_process(
        svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    data_q_msg_t q_msg = { 0 };
    uint8_t *circular_buffer_rd_pointer = NULL;

    ret_val = cy_rtos_get_queue(&lp_instance->data_queue, &q_msg,
    DATA_QUEUE_GET_DATA_WAIT_TIMEOUT_MS, false);
    if (CY_RSLT_SUCCESS == ret_val)
    {
        (void) svc_lp_get_from_msg_from_cmd_input_queue_and_process(
                lp_instance);

        if (SVC_LP_CMD_ID_DATA_RECEIVED == q_msg.cmd)
        {
#ifdef ENABLE_SVC_LP_CHECK_POINT
        	SVC_LP_CHECK_POINT();
#endif
            (void) svc_lp_auto_aad_detection_check(lp_instance);

            lp_instance->stats.frame_counter_received_after_last_aad_dbg++;

            if (lp_instance->stats.svc_lp_feed_post_fail_counter_track_dbg
                    != lp_instance->stats.svc_lp_feed_post_fail_counter_dbg)
            {
                lp_instance->stats.svc_lp_feed_post_fail_counter_dbg++;
                lp_instance->stats.svc_lp_feed_post_fail_counter_track_dbg =
                        lp_instance->stats.svc_lp_feed_post_fail_counter_dbg;

//                cy_svc_log_err(ret_val, "SVC feed drop counter:%d, err:0x%x",
//                        lp_instance->stats.svc_lp_feed_post_fail_counter_dbg,
//                        lp_instance->stats.svc_lp_feed_post_fail_reason_dbg);

                lp_instance->stats.svc_lp_feed_post_fail_reason_dbg = 0;
            }

            ret_val = svc_lp_get_circular_buf_rd_wr_pointer(lp_instance,
                    &circular_buffer_rd_pointer, false);
            if ((CY_RSLT_SUCCESS == ret_val)
                    && (NULL != circular_buffer_rd_pointer))
            {
                ret_val = svc_lp_verify_crc_of_the_buffer(lp_instance,
                        q_msg.cbuf_pointer, q_msg.data_crc_check,
                        (char*) circular_buffer_rd_pointer);
                if (CY_RSLT_SUCCESS != ret_val)
                {
                    cy_svc_log_err(ret_val, "CRC fail-Ignore Buffer");
                    goto CLEAN_RETURN;
                }

#ifdef ENABLE_USB_DBG_OUTPUT
                usb_send_out_dbg_put(2, (short*) circular_buffer_rd_pointer);
#endif

#ifdef COMPONENT_PROFILER
                cy_svc_profile(SVC_PROFILE_CMD_START,NULL);
#endif
                ret_val = svc_lp_process_data(lp_instance,
                        circular_buffer_rd_pointer);
                lp_instance->last_processed_frame_start_address =
                        (unsigned char*) circular_buffer_rd_pointer;

#ifdef COMPONENT_PROFILER
                cy_svc_profile(SVC_PROFILE_CMD_STOP,NULL);
#endif

                if (CY_RSLT_SUCCESS != ret_val)
                {
                    cy_svc_log_err(ret_val, "Process data fail");
                    goto CLEAN_RETURN;
                }
            }

#ifdef ENABLE_SVC_LP_CHECK_POINT
        	SVC_LP_CHECK_POINT();
#endif
        }
        else
        {
            /**
             * Drop the MSG.
             *
             * Possible other message id could be
             * SVC_LP_CMD_ID_SET_STATE_FROM_HP. This is used to wake up from
             * Data Q and process the command Q.
             */
        }
    }
    else
    {
        cy_svc_log_err(ret_val, "Invalid Get Q return");
    }

    CLEAN_RETURN:

    /**
     * Process buffer completed, Hence updating the read offset
     */
    if (NULL != circular_buffer_rd_pointer)
    {
        (void) svc_lp_circular_buffer_update_rd_wr_offset(lp_instance, false);
    }

    return ret_val;
}

#endif
