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
 * @file staged_voice_control_lp_data.c
 *
 */
#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_process_data.h"
#include "staged_voice_control_lp_lpwwd.h"
#include "staged_voice_control_profile.h"

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

cy_rslt_t svc_lp_feed_data_to_cbuf_and_notify_task(svc_lp_instance_t *lp_instance,
                    CY_SVC_DATA_T *input_data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t *circular_buffer_wr_pointer = NULL;
    data_q_msg_t q_msg = { 0 };
    bool b_successfully_posted = false;
    size_t num_q_spaces = 0;
    size_t num_q_count = 0;

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif

    cy_rtos_count_queue(&lp_instance->data_queue,&num_q_count);

    num_q_spaces =  lp_instance->init_params.pre_roll_frame_count_from_lpwwd_detect_frame +
    		MAX_DATA_Q_SUPPORTED_EXTRA_FROM_PREROLL_BUFFER - num_q_count;
    if(num_q_spaces < 1)
    {
#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    	return CY_RSLT_SVC_Q_FULL;
    }

    ret_val = svc_lp_get_circular_buf_rd_wr_pointer(lp_instance,
            &circular_buffer_wr_pointer, true);
    if ((CY_RSLT_SUCCESS == ret_val) && (NULL != circular_buffer_wr_pointer))
    {
#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC
        ret_val = svc_lp_cbuf_marker_check_mark_on_buffer(lp_instance,
                circular_buffer_wr_pointer);
        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err_on_no_isr(ret_val, "Buffer Overwrite Warning");
            return ret_val;
        }
#endif
        memcpy(circular_buffer_wr_pointer, input_data,
                lp_instance->circular_shared_buffer->frame_size_in_bytes);

        q_msg.cmd = SVC_LP_CMD_ID_DATA_RECEIVED;
        q_msg.data_crc_check = svc_lp_create_crc_for_buffer(lp_instance,
                (char*) circular_buffer_wr_pointer);
        q_msg.cbuf_pointer = (char *)circular_buffer_wr_pointer;

        /**
         * Timeout is 0, fail in the put Q gives clue on data
         * Q size of some processing power in some thread is problem.
         * or some thread is not releasing the CPU
         */
        ret_val = cy_rtos_put_queue(&lp_instance->data_queue, &q_msg, 0,
                is_in_isr());
        if (CY_RSLT_SUCCESS != ret_val)
        {
            size_t num_waiting = 0;
            cy_rtos_count_queue(&lp_instance->data_queue, &num_waiting);

//                cy_svc_log_err_on_no_isr(ret_val, "data input Q put fail, size:%d",
//                        num_waiting);
            goto CLEAN_RETURN;
        }
        else
        {
            b_successfully_posted = true;
        }
    }

    CLEAN_RETURN:

    if (NULL != circular_buffer_wr_pointer)
    {
        if (true == b_successfully_posted)
        {
            (void) svc_lp_circular_buffer_update_rd_wr_offset(lp_instance,
                    true);
        }
        else
        {
            /**
             * Skip the data written to the buffer and update the fail counter
             */
            lp_instance->stats.svc_lp_feed_post_fail_counter_dbg++;
        }
    }

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    return ret_val;
}

cy_rslt_t svc_lp_process_data(svc_lp_instance_t *lp_instance, uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_SOD)
    {
        ret_val = svc_lp_sod_process(lp_instance, data);
        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err(ret_val, "Process SOD fail");
            goto CLEAN_RETURN;
        }
    }

    if( (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_HPF) &&\
        (lp_instance->sod_detected == true) )
    {
#ifdef COMPONENT_PROFILER
        cy_svc_profile(SVC_HPF_PROFILE_CMD_START,NULL);
#endif /* COMPONENT_PROFILER */

        ret_val = svc_lp_hpf_process(lp_instance, (uint8_t*) data, data);

#ifdef COMPONENT_PROFILER
        cy_svc_profile(SVC_HPF_PROFILE_CMD_STOP,NULL);
#endif /* COMPONENT_PROFILER */

        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err_on_no_isr(ret_val, "HPF process fail");
            goto CLEAN_RETURN;
        }
    }

#ifdef ENABLE_SVC_LOW_LATENCY_PROFILE

    if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_LPWWD)
    {
        ret_val = svc_lp_process_data_lpwwd(lp_instance, data);
        if (CY_RSLT_SUCCESS != ret_val)
        {
            cy_svc_log_err(ret_val, "Process lpwwd fail");
            goto CLEAN_RETURN;
        }

        if (CY_SVC_STAGE_WAITING_FOR_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTION
                == lp_instance->current_stage)
        {
            lp_instance->post_lpwwd_received_frame_count++;

            if (0 != lp_instance->post_wwd_frame_count_req_by_hp)
            {
                svc_lp_start_circular_buf_update_on_transition_to_hp(
                        lp_instance, NULL,
                        SVC_TRIGGER_SEND_POST_WWD_HPWWD_DET_IN_PROGRESS);
            }
            else if (lp_instance->post_hpwwd_pending_frame_counter_to_hp > 0)
            {
                lp_instance->stats.hpwwd_feed_counter_dbg++;

                svc_lp_send_ipc_event_circular_buffer_update(lp_instance, data,
                        lp_instance->circular_shared_buffer->frame_size_in_bytes,
                        0, 0);

                svc_lp_get_circular_buf_post_shift_buffer_from_any_address(
                        lp_instance, data, 1,
                        &lp_instance->hpwwd_trigger_data_final_address);

                lp_instance->post_hpwwd_pending_frame_counter_to_hp--;

                if(0 == lp_instance->post_hpwwd_pending_frame_counter_to_hp)
                {
                    cy_svc_log_info("Sent post wwd frames");
                }
            }
        }
        else if (CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT
                == lp_instance->current_stage)
        {
        	if(CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT ==
        			lp_instance->stage_force_start)
        	{
                svc_lp_send_ipc_event_circular_buffer_update(lp_instance, data,
                        lp_instance->circular_shared_buffer->frame_size_in_bytes, 0,
						CY_SVC_BUF_INFO_ASR_FORCE_START);
                lp_instance->stage_force_start = CY_SVC_STAGE_INVALID;
        	}
        	else
        	{
                svc_lp_send_ipc_event_circular_buffer_update(lp_instance, data,
                        lp_instance->circular_shared_buffer->frame_size_in_bytes, 0,
                        0);
        	}
        }
        else if ((CY_SVC_STAGE_ASR_PROCESSING_QUERY_DETECTED
                == lp_instance->current_stage)
                && (true
                        == lp_instance->stream_requested_on_asr_processing_query_state))
        {
            svc_lp_send_ipc_event_circular_buffer_update(lp_instance, data,
                    lp_instance->circular_shared_buffer->frame_size_in_bytes, 0,
                    0);
        }
    }
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    CLEAN_RETURN:
    return ret_val;
}

#endif
