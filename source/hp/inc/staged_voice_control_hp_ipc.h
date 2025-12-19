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
 * @file staged_voice_control_hp_ipc.h
 *
 */

#ifndef __CY_SVC_HP_IPC_H__
#define __CY_SVC_HP_IPC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_HP_MW
#include "staged_voice_control_hp_private.h"

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

cy_rslt_t svc_hp_ipc_init(svc_hp_instance_t *hp_instance, cy_svc_hp_config_t *init);

cy_rslt_t svc_hp_ipc_deinit(svc_hp_instance_t *hp_instance);

cy_rslt_t svc_hp_ipc_send_command_set_state(
        svc_hp_instance_t *hp_instance,
        cy_svc_set_state_t set_state,
        void *state_info);

svc_hp_instance_t* svc_hp_get_instance(void);

void svc_hp_ipc_msg_receive_callback(uint32_t *msg);

#ifndef ENABLE_LOW_POWER_SVC
void svc_hp_hal_ipc_sem_init(cy_svc_hp_config_t *init);

void svc_hp_hal_ipc_signal_lp_to_start(void);

void svc_hp_hal_ipc_sem_deinit( );
#endif

cy_rslt_t svc_hp_ipc_send_command_ready_state(
        svc_hp_instance_t *hp_instance);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __CY_SVC_HP_IPC_H__ */
