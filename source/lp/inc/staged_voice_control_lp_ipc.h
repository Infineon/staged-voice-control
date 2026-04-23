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
 * @file staged_voice_control_lp_hpf.h
 *
 */

#ifndef __CY_SVC_LP_IPC_H__
#define __CY_SVC_LP_IPC_H__

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
#ifndef SVC_LP_WAIT_FOR_HP_READY_TIMEOUT
#define SVC_LP_WAIT_FOR_HP_READY_TIMEOUT (10*1000)
#endif

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

cy_rslt_t svc_lp_ipc_init(svc_lp_instance_t *lp_instance, cy_svc_lp_config_t *init );

cy_rslt_t svc_lp_ipc_deinit(svc_lp_instance_t *lp_instance);

cy_rslt_t svc_lp_send_ipc_event_circular_buffer_update(
        svc_lp_instance_t *lp_instance,
        uint8_t *data_pointer,
        uint32_t data_len,
        uint16_t insuff_preroll_frame_count,
        cy_svc_buffer_info_t buffer_info);

cy_rslt_t svc_lp_notify_svc_hp_for_state_execution_result(
        svc_lp_instance_t *lp_instance,
        cy_rslt_t set_state_execution_ret_val);

void svc_ipc_lp_msg_receive_callback(uint32_t *msg);

#ifndef ENABLE_LOW_POWER_SVC
void svc_lp_ipc_sync_with_hp_sem_init(cy_svc_lp_config_t *init);
void svc_lp_wait_for_sem_signal_sync_from_hp(void);
void svc_lp_ipc_sync_with_hp_sem_deinit(void);
#endif

void svc_lp_ipc_send_done_callback(void);
void svc_lp_ipc_wait_for_send_complete(void);

void svc_lp_clear_svc_hp_ready_sync_sem(void);
void svc_lp_wait_for_hp_ready_complete(void);
void svc_lp_notify_hp_ready_msg(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __CY_SVC_LP_IPC_H__ */
