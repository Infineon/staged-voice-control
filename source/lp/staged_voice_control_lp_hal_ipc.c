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
 * (c) 2020-2026, Infineon Technologies AG, or an affiliate of Infineon
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
#ifdef ENABLE_SVC_LP_MW
#ifdef ENABLE_SVC_IPC_BY_HAL
#include "cy_pdl.h"
#include "cyhal.h"
#include "staged_voice_control_ipc.h"
#include "staged_voice_control_lp_ipc.h"
#include "staged_voice_control_lp_queue.h"
#include "staged_voice_control_lp_hal_ipc.h"

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
uint8_t	ipc_send_from_lp_to_hp_counter = 0;
uint8_t	ipc_recv_from_hp_to_lp_counter = 0;
#endif

#define QUEUE2_MSG_LEN_MAX  25UL
#define QUEUE3_NUM          3UL
#define QUEUE3_ITEMS        5UL

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
#define SVC_HAL_IPC_RCV_PROCESS_THREAD_NAME             "svc_lp_hal_ipc"
#define SVC_HAL_IPC_RCV_THREAD_PRIORITY                 (CY_RTOS_PRIORITY_ABOVENORMAL)
#define HAL_IPC_SEND_LP_TO_HP_Q                         (CY_RSLT_MODULE_STAGED_VOICE_CONTROL_BASE + 1)
#define HAL_IPC_SEND_HP_TO_LP_Q                         (CY_RSLT_MODULE_STAGED_VOICE_CONTROL_BASE + 2)
#define HAL_IPC_Q_LP_TO_HP_SIZE                         (30)
#define HAL_IPC_Q_HP_TO_LP_SIZE                         (5)
#define SVC_HAL_IPC_RCV_THREAD_STACK_SIZE               (2*1280) //TODO: Estimate the HAL IPC Stack size and modify the right value
#define MAX_LP_ISR_NOTIFY_SUPPORTED_SIZE (10)
#ifdef CREATE_SVC_LP_THREAD_STACK_IN_SOCMEM_DATA
unsigned char hal_ipc_lp_thread_stack[SVC_HAL_IPC_RCV_THREAD_STACK_SIZE] __attribute__((section(".cy_socmem_data ")));
#endif

typedef struct
{
    cyhal_ipc_t hal_ipc_lp_to_hp_q;
    cyhal_ipc_t hal_ipc_hp_to_lp_q;
    cy_thread_t hal_ipc_lp_receive_thread_instance;
    volatile bool hal_ipc_lp_rcv_quit_thread_instance;

} svc_lp_hal_ipc_resource_t ;

svc_lp_hal_ipc_resource_t lp_hal_ipc_rsrc = {0};

#ifdef ENABLE_IPC_ACCESS_BY_ISR
/* write event callback for async-callback-queue */
void svc_lp_async_queue_write_notification_cbk_from_hp(void *callback_arg, cyhal_ipc_event_t event)
{
	char notify = {0};
    svc_lp_instance_t *lp_instance = NULL;
    lp_instance = svc_lp_get_instance();

    if(CYHAL_IPC_QUEUE_WRITE == event)
    	cy_rtos_put_queue(&lp_instance->ipcisr_notify_queue, &notify, 0, is_in_isr());

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
static void svc_lp_hal_ipc_receive_func(cy_thread_arg_t thread_input)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_lp_hal_ipc_resource_t *plp_hal_ipc_rsrc = (svc_lp_hal_ipc_resource_t *) thread_input;
    uint8_t temp_recv_buffer[MAX_MSG_PAY_SIZE_IN_BYTES] = {0};
#ifdef ENABLE_IPC_ACCESS_BY_ISR
    svc_lp_instance_t *lp_instance = NULL;
    char notify = {0};
    lp_instance = svc_lp_get_instance();
#endif

    cy_svc_log_info("[IPC] Entering HAL IPC Rcv thread");

    if (NULL != plp_hal_ipc_rsrc)
    {
        while (false == plp_hal_ipc_rsrc->hal_ipc_lp_rcv_quit_thread_instance)
        {
#ifdef ENABLE_IPC_ACCESS_BY_ISR
			cy_svc_log_info("[SVCLP:IPC] Wait ISR WR Notification");

			ret_val = cy_rtos_get_queue(&lp_instance->ipcisr_notify_queue, &notify,
					CY_RTOS_NEVER_TIMEOUT, false);

			cy_svc_log_info("[SVCLP:IPC] Got ISR WR Notification");

			do
			{
				ret_val = cyhal_ipc_queue_get(&plp_hal_ipc_rsrc->hal_ipc_hp_to_lp_q,
						temp_recv_buffer, 0);
				if(CY_RSLT_SUCCESS == ret_val)
				{
					cy_svc_log_info("[SVCLP:IPC] Received IPC msg from HP");
					svc_ipc_lp_msg_receive_callback((uint32_t *)temp_recv_buffer);
				}
				else
				{
					cy_svc_log_info("[SVCLP:IPC] No IPC msg from HP");
				}

			} while(CY_RSLT_SUCCESS == ret_val);
#else
            ret_val = cyhal_ipc_queue_get(&plp_hal_ipc_rsrc->hal_ipc_hp_to_lp_q,
                    temp_recv_buffer, CYHAL_IPC_NEVER_TIMEOUT);
            if(CY_RSLT_SUCCESS == ret_val)
            {
                cy_svc_log_info("[IPC] Received ipc msg from hp");
                (void)svc_ipc_lp_msg_receive_callback((uint32_t *)temp_recv_buffer);
            }
#endif
        }
    }

    cy_svc_log_info("[IPC] Exiting HAL IPC Rcv thread");

    cy_rtos_exit_thread();
}

cy_rslt_t svc_lp_create_hal_ipc_resources(cy_svc_lp_config_t *init )
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    void *queue_pool_lp_to_hp = NULL;
     void *queue_pool_hp_to_lp = NULL;
     cyhal_ipc_queue_t *queue_lp_to_hp_handle = NULL;
     cyhal_ipc_queue_t *queue_hp_to_lp_handle = NULL;
#ifdef ENABLE_IPC_ACCESS_BY_ISR
     svc_lp_instance_t *lp_instance = NULL;
     lp_instance = svc_lp_get_instance();
#endif

     CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool_lp_to_hp, HAL_IPC_Q_LP_TO_HP_SIZE, MAX_MSG_PAY_SIZE_IN_BYTES);

     CYHAL_IPC_QUEUE_POOL_ALLOC(queue_pool_hp_to_lp, HAL_IPC_Q_HP_TO_LP_SIZE,MAX_MSG_PAY_SIZE_IN_BYTES);

     CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_lp_to_hp_handle);

     CYHAL_IPC_QUEUE_HANDLE_ALLOC(queue_hp_to_lp_handle);

     queue_lp_to_hp_handle->channel_num = MID_IPC_QUEUE_CHN;
     queue_lp_to_hp_handle->queue_num = HAL_IPC_SEND_LP_TO_HP_Q;
     queue_lp_to_hp_handle->queue_pool = queue_pool_lp_to_hp;
     queue_lp_to_hp_handle->num_items = HAL_IPC_Q_LP_TO_HP_SIZE;
     queue_lp_to_hp_handle->item_size = MAX_MSG_PAY_SIZE_IN_BYTES;

     ret_val = cyhal_ipc_queue_init(&lp_hal_ipc_rsrc.hal_ipc_lp_to_hp_q,
            queue_lp_to_hp_handle);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "[IPC] IPC LP to HP init fail");
        goto CLEAN_RETURN;
    }

     queue_hp_to_lp_handle->channel_num = MID_IPC_QUEUE_CHN;
     queue_hp_to_lp_handle->queue_num = HAL_IPC_SEND_HP_TO_LP_Q;
     queue_hp_to_lp_handle->queue_pool = queue_pool_hp_to_lp;
     queue_hp_to_lp_handle->num_items = HAL_IPC_Q_HP_TO_LP_SIZE;
     queue_hp_to_lp_handle->item_size = MAX_MSG_PAY_SIZE_IN_BYTES;

     ret_val = cyhal_ipc_queue_init(&lp_hal_ipc_rsrc.hal_ipc_hp_to_lp_q,
             queue_hp_to_lp_handle);
     if (CY_RSLT_SUCCESS != ret_val)
     {
         cy_svc_log_err(ret_val, "[IPC] IPC HP to LP init fail");
         goto CLEAN_RETURN;
     }

#ifdef ENABLE_IPC_ACCESS_BY_ISR
     ret_val = cy_rtos_init_queue(&lp_instance->ipcisr_notify_queue,
    		 MAX_LP_ISR_NOTIFY_SUPPORTED_SIZE, sizeof(char));
     if (ret_val != CY_RSLT_SUCCESS)
     {
     	cy_svc_log_err(ret_val, "Create ipcisr_notify_queue fail");
     	goto CLEAN_RETURN;
     }
#endif

#ifdef CREATE_SVC_LP_THREAD_STACK_IN_SOCMEM_DATA
     memset(hal_ipc_lp_thread_stack, 0, sizeof(hal_ipc_lp_thread_stack));

     /* Start HAL IPC Receive data process thread */
     ret_val = cy_rtos_create_thread(
             &lp_hal_ipc_rsrc.hal_ipc_lp_receive_thread_instance,
             svc_lp_hal_ipc_receive_func, SVC_HAL_IPC_RCV_PROCESS_THREAD_NAME,
			 hal_ipc_lp_thread_stack, SVC_HAL_IPC_RCV_THREAD_STACK_SIZE,
             SVC_HAL_IPC_RCV_THREAD_PRIORITY, (cy_thread_arg_t ) &lp_hal_ipc_rsrc);
     if (ret_val != CY_RSLT_SUCCESS)
     {
         cy_svc_log_err(ret_val, "[IPC] Create the HAL LP IPC rcv thrd fail");
         goto CLEAN_RETURN;
     }
     else
     {
    	  cy_svc_log_info("Create the HAL LP IPC (socmem stack) success");
     }
#else
     /* Start HAL IPC Receive data process thread */
     ret_val = cy_rtos_create_thread(
             &lp_hal_ipc_rsrc.hal_ipc_lp_receive_thread_instance,
             svc_lp_hal_ipc_receive_func, SVC_HAL_IPC_RCV_PROCESS_THREAD_NAME,
             NULL, SVC_HAL_IPC_RCV_THREAD_STACK_SIZE,
             SVC_HAL_IPC_RCV_THREAD_PRIORITY, (cy_thread_arg_t ) &lp_hal_ipc_rsrc);
     if (ret_val != CY_RSLT_SUCCESS)
     {
         cy_svc_log_err(ret_val, "[IPC] Create the HAL LP IPC rcv thrd fail");
         goto CLEAN_RETURN;
     }

#endif



#ifdef ENABLE_IPC_ACCESS_BY_ISR
     /* Register callback for write operation in async callback handling queue */
     cyhal_ipc_queue_register_callback(&lp_hal_ipc_rsrc.hal_ipc_hp_to_lp_q,
    		 svc_lp_async_queue_write_notification_cbk_from_hp, NULL);

     cyhal_ipc_queue_enable_event(&lp_hal_ipc_rsrc.hal_ipc_hp_to_lp_q,
    		 CYHAL_IPC_QUEUE_WRITE, CYHAL_ISR_PRIORITY_DEFAULT, true);
#endif

     cy_svc_log_info("[IPC] HAL IPC create resource success");

     CLEAN_RETURN:
     return ret_val;
}


cy_rslt_t svc_lp_send_ipc_msg_to_hp_using_hal_ipc(
        svc_lp_instance_t *lp_instance,
        uint8_t *data)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    if (NULL == data)
    {
        ret_val = CY_RSLT_SVC_BAD_ARG;
        cy_svc_log_err(ret_val, "[IPC] Invalid Arg %p", data);
        return ret_val;
    }

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
    *(data+1) = ++ipc_send_from_lp_to_hp_counter;
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif
    //cy_svc_log_info("[IPC] Attempt to put in IPC Q");

    ret_val = cyhal_ipc_queue_put(&lp_hal_ipc_rsrc.hal_ipc_lp_to_hp_q, data,
            CYHAL_IPC_NEVER_TIMEOUT);
    if (CY_RSLT_SUCCESS != ret_val)
    {
        cy_svc_log_err(ret_val, "[IPC] Put to IPC Q fail");
    }
    else
    {
        //cy_svc_log_info("[IPC] Put to IPC Q success");
    }
#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif
    return ret_val;
}
#endif /* ENABLE_SVC_IPC_BY_HAL */
#endif

/* [] END OF FILE */
