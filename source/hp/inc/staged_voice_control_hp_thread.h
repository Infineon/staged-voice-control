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
 * @file staged_voice_control_hp_thread.h
 *
 */

#ifndef __SVC_HP_THREAD_H__
#define __SVC_HP_THREAD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_HP_MW
#include "cy_log.h"
#include "cy_result.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "cyabs_rtos.h"
#include "cy_staged_voice_control.h"
#include "staged_voice_control_hp_private.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/
cy_rslt_t svc_hp_create_thread_resouces(svc_hp_instance_t *hp_instance);

cy_rslt_t svc_hp_delete_thread_resouces(svc_hp_instance_t *hp_instance);

cy_rslt_t svc_hp_push_to_data_queue_from_ipc_cbk(
        svc_hp_instance_t *hp_instance,
        CY_SVC_DATA_T *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buf_info_bitmask);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __SVC_HP_THREAD_H__ */
