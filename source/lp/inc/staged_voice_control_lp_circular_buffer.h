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
 * @file staged_voice_control_lp_circular_buffer.h
 *
 */

#ifndef CY_SVC_LP_CIRCULAR_BUFFER_H__
#define CY_SVC_LP_CIRCULAR_BUFFER_H__

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef ENABLE_SVC_LP_MW
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_resource.h"
#include "staged_voice_control_lp_queue.h"

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

cy_rslt_t svc_lp_create_circular_buf(
        svc_lp_instance_t *lp_instance,
        cy_svc_lp_config_t *create_domain);

cy_rslt_t svc_lp_delete_circular_buf(svc_lp_instance_t *lp_instance);

cy_rslt_t svc_lp_get_circular_buf_rd_wr_pointer(
        svc_lp_instance_t *lp_instance,
        uint8_t **buf_ptr,
        bool is_wr_operation);

cy_rslt_t svc_lp_circular_buffer_update_rd_wr_offset(
        svc_lp_instance_t *lp_instance,
        bool is_wr_operation);

cy_rslt_t svc_lp_start_circular_buf_update_on_transition_to_hp(
        svc_lp_instance_t *lp_instance,
        uint8_t *cur_pointer,
        svc_stage_trigger_t stage_trigger);

cy_rslt_t svc_lp_get_circular_buf_pre_shift_buffer_from_any_address(
        svc_lp_instance_t *lp_instance,
        uint8_t *cur_pointer,
        unsigned int number_frames_before_cur_pointer,
        uint8_t **pre_buffer);

cy_rslt_t svc_lp_get_circular_buf_post_shift_buffer_from_any_address(
        svc_lp_instance_t *lp_instance,
        uint8_t *cur_pointer,
        unsigned int number_frames_after_cur_pointer,
        uint8_t **post_buffer);

uint32_t svc_lp_create_crc_for_buffer(svc_lp_instance_t *lp_instance,
        char *data);

cy_rslt_t svc_lp_verify_crc_of_the_buffer(svc_lp_instance_t *lp_instance,
        char *crc_source_pointer,
        unsigned int crc, char *data);

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC
cy_rslt_t svc_lp_cbuf_marker_mark_ipc_send_buffer(
        svc_lp_instance_t *lp_instance, uint8_t *data_pointer,
        unsigned frame_count);
#endif

#ifdef ENABLE_SVC_LP_HP_BUFFER_SYNC
cy_rslt_t svc_lp_cbuf_marker_check_mark_on_buffer(
        svc_lp_instance_t *lp_instance, uint8_t *data_pointer);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif /* CY_SVC_LP_CIRCULAR_BUFFER_H__ */
