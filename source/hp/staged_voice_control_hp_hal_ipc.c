/******************************************************************************
 * File Name:   main.c
 *
 * Description: This is the CM0p core source code of the Dual CPU Cyberon code Example
 *              for ModusToolbox.
 *
 * Related Document: See README.md
 *
 *
 *******************************************************************************
 * (c) 2020-2025, Infineon Technologies AG, or an affiliate of Infineon
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
 *******************************************************************************/

#ifdef ENABLE_SVC_HP_MW
#ifdef ENABLE_SVC_IPC_BY_HAL
#include "staged_voice_control_ipc.h"
#include "staged_voice_control_hp_ipc.h"
#include "staged_voice_control_hp_thread.h"



/****************************************************************************
 * Macros
 *****************************************************************************/

/****************************************************************************
 * Functions Prototypes
 *****************************************************************************/

/****************************************************************************
 * Global variables
 *****************************************************************************/

/****************************************************************************
 * Constants
 *****************************************************************************/

#define SVC_HP_HAL_IPC_RCV_PROCESS_THREAD_NAME             "svc_hp_hal_ipc"
#define SVC_HP_HAL_IPC_RCV_THREAD_PRIORITY                 (CY_RTOS_PRIORITY_ABOVENORMAL)
#define HAL_IPC_SEND_LP_TO_HP_Q                            (CY_RSLT_MODULE_STAGED_VOICE_CONTROL_BASE + 1)
#define HAL_IPC_SEND_HP_TO_LP_Q                            (CY_RSLT_MODULE_STAGED_VOICE_CONTROL_BASE + 2)
#define SVC_HP_HAL_IPC_RCV_THREAD_STACK_SIZE                (2*1280) //TODO: Estimate the HAL IPC Stack size and modify the right value

volatile int svc_hp_isr_counter = 0;
volatile int svc_hp_isr_notify_counter = 0;
volatile int svc_hp_isr_get_ipc_q_recv_counter = 0;

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
uint8_t	ipc_send_from_hp_to_lp_counter = 0;
uint8_t	ipc_recv_from_lp_to_hp_counter = 0;
#endif

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC_VERIFY
#define IPC_FRAME_DBG_CHECK (600)
char verify_ipc_data [IPC_FRAME_DBG_CHECK] = {0};
unsigned int verify_ipc_recv_success_counter = 0;
unsigned int verify_ipc_recv_fail_counter = 0;
#endif


typedef struct
{
    cyhal_ipc_t hal_hp_ipc_lp_to_hp_q;
    cyhal_ipc_t hal_hp_ipc_hp_to_lp_q;
    cy_thread_t hal_hp_ipc_hp_receive_thread_instance;
    volatile bool hal_hp_ipc_rcv_quit_thread_instance;

} svc_hp_hal_ipc_resource_t ;

svc_hp_hal_ipc_resource_t hp_hal_ipc_rsrc = {0};


#ifdef ENABLE_IPC_ACCESS_BY_ISR

#define MAX_HP_ISR_NOTIFY_SUPPORTED_SIZE (10)

/* write event callback for async-callback-queue */
void svc_hp_async_queue_write_notification_cbk_from_lp(void *callback_arg, cyhal_ipc_event_t event)
{
	char notify = {0};
    svc_hp_instance_t *hp_instance = NULL;
    hp_instance = svc_hp_get_instance();

    SVC_HP_TRACK_HP_ISR_COUNTER();

    if(CYHAL_IPC_QUEUE_WRITE == event)
    {
    	cy_rtos_put_queue(&hp_instance->ipcisr_notify_queue, &notify, 0, true);
    }

    return;
}
#endif



/**
 * stage voice thread process function
 *
 * @param[in]  thread_input             argument to thread.
 * @param[in]  create_domain        Create low power domain configuration
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
static void svc_hp_hal_ipc_receive_func(cy_thread_arg_t thread_input)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_hal_ipc_resource_t *hp_hal_instance = (svc_hp_hal_ipc_resource_t *) thread_input;
    uint8_t temp_recv_buffer[MAX_MSG_PAY_SIZE_IN_BYTES] = {0};
#ifdef ENABLE_IPC_ACCESS_BY_ISR
    char notify = {0};
    svc_hp_instance_t *hp_instance = NULL;
    hp_instance = svc_hp_get_instance();
#endif

    cy_svc_log_info("[IPC] Entering HAL IPC Rcv thread");

    if (NULL != hp_hal_instance)
    {
        while (false == hp_hal_instance->hal_hp_ipc_rcv_quit_thread_instance)
        {
#ifdef ENABLE_IPC_ACCESS_BY_ISR

			cy_svc_log_info("[SVCHP-IPC] Wait ISR WR Notification");

			ret_val = cy_rtos_get_queue(&hp_instance->ipcisr_notify_queue, &notify,
					CY_RTOS_NEVER_TIMEOUT, false);

			SVC_HP_TRACK_HP_ISR_NOTIFY_COUNTER();

			cy_svc_log_info("[IPC] Got ISR WR Notification");

			do
			{
				ret_val = cyhal_ipc_queue_get(&hp_hal_instance->hal_hp_ipc_lp_to_hp_q,
						temp_recv_buffer, 0);
				if(CY_RSLT_SUCCESS == ret_val)
				{
					SVC_HP_TRACK_HP_ISR_GET_IPC_Q_RECV_COUNTER();

					cy_svc_log_info("[IPC] Received IPC msg from LP");
					svc_hp_ipc_msg_receive_callback((uint32_t *)temp_recv_buffer);
				}
				else
				{
					cy_svc_log_info("[IPC] No IPC msg from LP");
				}

			} while(CY_RSLT_SUCCESS == ret_val);
#else
			ret_val = cyhal_ipc_queue_get(&hp_hal_instance->hal_hp_ipc_lp_to_hp_q,
					temp_recv_buffer, CY_RTOS_NEVER_TIMEOUT);
			if(CY_RSLT_SUCCESS == ret_val)
			{
				cy_svc_log_info("[IPC] Received ipc msg from LP");
				svc_hp_ipc_msg_receive_callback((uint32_t *)temp_recv_buffer);
			}
			else
			{
				cy_svc_log_info("[IPC] No ipc msg from LP");
			}
#endif
        }
    }

    cy_svc_log_info("[IPC] Exiting HAL IPC Rcv thread");
}


cy_rslt_t svc_hp_create_hal_ipc_resources(svc_hp_instance_t *hp_instance, cy_svc_hp_config_t *init)
{
	cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

    svc_hp_hal_ipc_sem_init(init);

    ret_val = cyhal_ipc_queue_get_handle(&hp_hal_ipc_rsrc.hal_hp_ipc_lp_to_hp_q,
    		MID_IPC_QUEUE_CHN,
             HAL_IPC_SEND_LP_TO_HP_Q);
     if(CY_RSLT_SUCCESS != ret_val)
     {
         cy_svc_log_err(ret_val, "[IPC] IPC init Q (LP->HP) fail")
         goto CLEAN_RETURN;
     }
     else
     {
         cy_svc_log_info("[IPC] Get LP to HP Q done");
     }

     ret_val = cyhal_ipc_queue_get_handle(&hp_hal_ipc_rsrc.hal_hp_ipc_hp_to_lp_q,
    		 MID_IPC_QUEUE_CHN,
             HAL_IPC_SEND_HP_TO_LP_Q);
     if(CY_RSLT_SUCCESS != ret_val)
     {
         cy_svc_log_err(ret_val, "[IPC] HAL HP IPC init Q (HP->LP) fail")
          goto CLEAN_RETURN;
     }
     else
     {
         cy_svc_log_info("[IPC] Get HP to LP Q done");
     }

#ifdef ENABLE_IPC_ACCESS_BY_ISR
     ret_val = cy_rtos_init_queue(&hp_instance->ipcisr_notify_queue,
    		 MAX_HP_ISR_NOTIFY_SUPPORTED_SIZE, sizeof(char));
     if (ret_val != CY_RSLT_SUCCESS)
     {
     	cy_svc_log_err(ret_val, "Create ipcisr_notify_queue fail");
     	goto CLEAN_RETURN;
     }
#endif

 	/* Start HAL HP IPC Receive data process thread */
     ret_val = cy_rtos_create_thread(
             &hp_hal_ipc_rsrc.hal_hp_ipc_hp_receive_thread_instance,
             svc_hp_hal_ipc_receive_func, SVC_HP_HAL_IPC_RCV_PROCESS_THREAD_NAME,
             NULL, SVC_HP_HAL_IPC_RCV_THREAD_STACK_SIZE,
             SVC_HP_HAL_IPC_RCV_THREAD_PRIORITY, (cy_thread_arg_t ) &hp_hal_ipc_rsrc);
     if (ret_val != CY_RSLT_SUCCESS)
     {
         cy_svc_log_err(ret_val, "[IPC] IPC Rcv thread create fail");
         goto CLEAN_RETURN;
     }

#ifdef ENABLE_IPC_ACCESS_BY_ISR
     /* Register callback for write operation in async callback handling queue */
     cyhal_ipc_queue_register_callback(&hp_hal_ipc_rsrc.hal_hp_ipc_lp_to_hp_q,
    		 svc_hp_async_queue_write_notification_cbk_from_lp, NULL);

     cyhal_ipc_queue_enable_event(&hp_hal_ipc_rsrc.hal_hp_ipc_lp_to_hp_q,
    		 CYHAL_IPC_QUEUE_WRITE, CYHAL_ISR_PRIORITY_DEFAULT, true);
#endif

    svc_hp_hal_ipc_signal_lp_to_start();


#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC_VERIFY
   memset(verify_ipc_data,1,sizeof(verify_ipc_data));
#endif


    CLEAN_RETURN:
    return ret_val;
}

cy_rslt_t svc_hp_delete_hal_ipc_resources(svc_hp_instance_t *hp_instance)
{
	cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;

	if(NULL == hp_instance)
	{
		cy_svc_log_err(ret_val, "Invalid Arg %p", hp_instance);
		goto CLEAN_RETURN;
	}

	hp_hal_ipc_rsrc.hal_hp_ipc_rcv_quit_thread_instance = true;

    ret_val = cy_rtos_terminate_thread(&hp_hal_ipc_rsrc.hal_hp_ipc_hp_receive_thread_instance);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        cy_svc_log_err(ret_val, "Terminate thread fail");

        /**
         * Note: Intentionally continuing to cleanup the other resources on fail,
         * since it is deallocate path. This is applicable for this function
         */
    }

#ifdef ENABLE_IPC_ACCESS_BY_ISR
    if (NULL != hp_instance->ipcisr_notify_queue)
    {
        ret_val = cy_rtos_deinit_queue(&hp_instance->ipcisr_notify_queue);
        if (ret_val != CY_RSLT_SUCCESS)
        {
            cy_svc_log_err(ret_val, "IPC ISR notify Q fail");
        }
        else
        {
            cy_svc_log_info("IPC ISR notify Q success");
        }
        hp_instance->ipcisr_notify_queue = NULL;
    }
    else
    {
        cy_svc_log_info("IPC ISR Q is not created");
    }
#endif


	memset(&hp_hal_ipc_rsrc.hal_hp_ipc_lp_to_hp_q, 0,
			sizeof(hp_hal_ipc_rsrc.hal_hp_ipc_lp_to_hp_q));
	memset(&hp_hal_ipc_rsrc.hal_hp_ipc_hp_to_lp_q, 0,
			sizeof(hp_hal_ipc_rsrc.hal_hp_ipc_hp_to_lp_q));

    CLEAN_RETURN:
    return ret_val;
}

cy_rslt_t svc_hp_ipc_send_msg_to_lp_using_hal_ipc(
        uint8_t* data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    if (NULL == data)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "Invalid Arg %p", data);
        return ret_val;
    }

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    *(data+1) = ++ipc_send_from_hp_to_lp_counter;
#endif

    ret_val = cyhal_ipc_queue_put(&hp_hal_ipc_rsrc.hal_hp_ipc_hp_to_lp_q, data,
            CYHAL_IPC_NEVER_TIMEOUT);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "Put to IPC Q (LP->HP) fail");
    }
    else
    {
        //cy_svc_log_info("Put to IPC Q (LP->HP) success");
    }

    return ret_val;
}

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC
cy_rslt_t cy_svc_hp_buffer_process_complete_notification(uint8_t *data_pointer,
		unsigned frame_count)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    unsigned int frame_offset_from_buffer_start = 0;
    svc_hp_instance_t *hp_instance = NULL;

    hp_instance = svc_hp_get_instance();

    if (NULL != hp_instance->circular_shared_buffer.buffer_share_access_tracker)
    {
        if (data_pointer
                >= hp_instance->circular_shared_buffer.buffer_start_address)
        {
            frame_offset_from_buffer_start =
                    ((char*) data_pointer
                            - (char*) hp_instance->circular_shared_buffer.buffer_start_address)
                            / hp_instance->circular_shared_buffer.frame_size_in_bytes;
            if (1 == frame_count)
            {

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC_VERIFY
                if(*(hp_instance->circular_shared_buffer.buffer_share_access_tracker +
                        frame_offset_from_buffer_start) == 1)
                {
                    cy_svc_log_info("CBUF-MARK-CLR: Set done correctly, Clearing now");
                    // printf("CBUF-MARK-CLR: Set done correctly, Clearing now\r\n");
                    verify_ipc_recv_success_counter++;
                }
                else
                {
                    cy_svc_log_err(ret_val, "CBUF-MARK-CLR: Fail Set done incorrectly");
                    // printf("CBUF-MARK-CLR: Fail Set done incorrectly\r\n");
                    verify_ipc_recv_fail_counter++;
                }
#endif

                *(hp_instance->circular_shared_buffer.buffer_share_access_tracker
                        + frame_offset_from_buffer_start) = 0;

//                cy_svc_log_info("CBUF-MARK-CLR: Offset:%d, Count:%d",
//                        frame_offset_from_buffer_start, frame_count);

                ret_val = CY_RSLT_SUCCESS;
            }
            else if (frame_count > 1)
            {

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC_VERIFY
                if ( 0 == memcmp(verify_ipc_data, (char *)hp_instance->circular_shared_buffer.buffer_share_access_tracker +
                        frame_offset_from_buffer_start, frame_count))
                {
                    cy_svc_log_info("CBUF-MARK-CLR: Set done correctly:%d, Clearing now",frame_count);
                    // printf("CBUF-MARK-CLR: Set done correctly:%d, Clearing now\r\n",frame_count);
                    verify_ipc_recv_success_counter++;
                }
                else
                {
                    cy_svc_log_err(ret_val, "CBUF-MARK-CLR: Fail Set done incorrectly: %d",frame_count);
                    // printf(ret_val, "CBUF-MARK-CLR: Fail Set done incorrectly: %d\r\n",frame_count);
                    verify_ipc_recv_fail_counter++;
                }
#endif

                memset(
                        ((char*) hp_instance->circular_shared_buffer.buffer_share_access_tracker
                                + frame_offset_from_buffer_start), 0,
                        frame_count);

//                cy_svc_log_info("CBUF-MARK-CLR: Offset:%d, Count:%d",
//                        frame_offset_from_buffer_start, frame_count);

                ret_val = CY_RSLT_SUCCESS;
            }
            else
            {
                cy_svc_log_err(ret_val, "CBUF-MARK-CLR: Invalid frame count");
            }
        }
        else
        {
            cy_svc_log_err(ret_val, "CBUF-MARK-CLR: Invalid frame buffer");
        }
    }
    else
    {
        cy_svc_log_info("CBUF-MARK-CLR: Buffer access marker disabled");
    }
    return ret_val;
}
#endif

#endif /* ENABLE_SVC_IPC_BY_HAL */
#endif

/* [] END OF FILE */
