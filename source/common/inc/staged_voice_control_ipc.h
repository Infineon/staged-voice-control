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
 * @file staged_voice_control_ipc.h
 *
 */


#ifndef __CY_SVC_IPC_H__
#define __CY_SVC_IPC_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "cy_staged_voice_control_common.h"
/*******************************************************************************
 *                              Macros
 ******************************************************************************/

#define MAX_MSG_PAY_SIZE_IN_BYTES (12)

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>
#include "cy_ipc_pipe.h"

typedef struct __attribute__((packed, aligned(4)))
{
    uint8_t     client_id;
    uint8_t     cpu_status;
    uint16_t    intr_mask;
    uint8_t     msg_pay[MAX_MSG_PAY_SIZE_IN_BYTES];
} ipc_msg_t ;

#elif ENABLE_SVC_IPC_STUB_SINGLE_CORE
#include "ipc_communication.h"
#else
#define MAX_MSG_PAY_SIZE_IN_BYTES (12)
#endif


#ifdef ENABLE_APP_SIMULATION_FOR_EVENTS
/* *
 * Refer svc_simnulation.h for more detailed simulation commands.
 */
#define IPC_CMD_ID_SIMULATION_START                    ((uint8_t)0x50)
#define IPC_CMD_ID_SIMULATION_END                      ((uint8_t)0x60)
#endif

#define IPC_CMD_ID_HP_TO_LP_SET_STATE                   ((uint8_t)0x88)
#define IPC_CMD_ID_LP_TO_HP_CONFIG_UPDATE               ((uint8_t)0x89)
#define IPC_CMD_ID_LP_TO_HP_CIRCULAR_BUF_UPDATE         ((uint8_t)0x90)
#define IPC_CMD_ID_LP_TO_HP_SET_STATE_RESULT            ((uint8_t)0x91)
#define IPC_CMD_ID_HP_TO_LP_READY_STATE                 ((uint8_t)0x92)

#define IPC_SYNC_SEM_CHECK_DELAY_MS                     (10)

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

/**
 * IPC message for config update from SVC LP. Size of this message
 * should be equivalent to size of (ipc_msg_t.msg_pay) = 12 Bytes
 */
typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t		dbg_msg_counter;			/* Byte 2 		- Msg counter 			*/
    uint8_t     reserved1[2];               /* Byte 3-4     - Reserved              */
    cy_svc_set_state_t     set_state;       /* Byte 5-8     - Set state             */
    uint8_t     payload_internal[MAX_SET_STATE_INFO_SIZE_IN_BYTES]; /* Byte 9-12    - Reserved              */

} ipc_cmd_hp_to_lp_set_state_t ;


/**
 * IPC message for ready update from SVC HP. Size of this message
 * should be equivalent to size of (ipc_msg_t.msg_pay) = 12 Bytes
 */
typedef struct __attribute__((packed, aligned(4)))
{
    uint8_t cmd_id; /* Byte 1       - Command identifier    */
    uint8_t dbg_msg_counter; /* Byte 2 		- Msg counter 			*/
    uint8_t reserved1[10]; /* Byte 3-12     - Reserved             */

} ipc_cmd_hp_to_lp_ready_state_t;




/**
 * IPC message for config update from SVC LP. Size of this message
 * should be equivalent to size of (ipc_msg_t.msg_pay) = 12 Bytes
 */
typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t		dbg_msg_counter;            /* Byte 2 		- Msg counter 			*/
    uint8_t     reserved1[2];               /* Byte 3-4     - Reserved              */
    char        *cbuf_information;          /* Byte 5 - 8   - Circular buffer information */
    cy_svc_stage_config_t stage_config_list; /* Byte  9-12  - LP config list        */

} ipc_cmd_lp_to_hp_cfg_update_t ;


/**
 * IPC message for set state execution result from SVC LP to SVC HP.
 * Size of this message should be equivalent to size of (ipc_msg_t.msg_pay)
 * = 12 Bytes
 */
typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t		dbg_msg_counter;            /* Byte 2 		- Msg counter 			*/
    uint8_t     reserved1[2];               /* Byte 3-4     - Reserved              */
    cy_svc_stage_t cur_stage;               /* Byte 5-8     - Current stage         */
    cy_rslt_t   set_state_result;           /* Byte 9-12   - set state result list */

} ipc_cmd_lp_to_hp_set_state_result_t ;

typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t		dbg_msg_counter;            /* Byte 2 		- Msg counter 			*/
    uint8_t    	data_info_bitmask;          /* Byte 3       - Buffer info           */
    uint8_t     current_stage;              /* Byte 4       - Current Stage         */
    uint8_t      *data_pointer;             /* Byte 5 - 8   - Data pointer          */
    uint16_t     frame_count;               /* Byte 9 - 10  - Frame count           */
    uint16_t     insuff_preroll_frame_count;/* Byte 11 - 12 - In sufficient pre-roll Frame count */

} ipc_cmd_lp_to_hp_data_t ;


typedef struct
{
    /**
     * Buffer data information
     */
    unsigned int audio_type; /* Mono/stereo */
    unsigned int frame_size_in_bytes;
    unsigned int sameple_size_in_bytes;

    unsigned int wr_offset;
    unsigned int rd_offset;

    unsigned int circular_buf_size;

    unsigned char *original_allocated_buffer_address;
    unsigned int  original_allocated_buffer_size;

    unsigned char *buffer_start_address;  /** Used for cbuf management for audio data */

    unsigned char *actual_buffer_start_address_with_guard;

    unsigned char *buffer_share_access_tracker;
    unsigned int  buffer_share_access_tracker_size;

} svc_circular_buffer_header_t;


#ifdef __cplusplus
}
#endif

#endif /* __CY_SVC_IPC_H__ */
