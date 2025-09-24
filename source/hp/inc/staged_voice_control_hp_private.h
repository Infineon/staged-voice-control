/*
 * Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/**
 * @file staged_voice_control_hp_private.h
 *
 */

#ifndef __SVC_HP_PRIVATE_H__
#define __SVC_HP_PRIVATE_H__

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
#include "cy_pdl.h"

#ifdef ENABLE_TIMELINE_MARKER
#include "audio_timeline_marker.h"
#endif

#include "staged_voice_control_dbg_control.h"
#include "staged_voice_control_ipc.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
/**
 * Task is needed to handle the IPC Pipe communication
 */
#define ENABLE_TASK_FOR_SVC_HP
#else
/**
 * TODO: Check if it is really needed
 */
//#define ENABLE_TASK_FOR_SVC_HP
#endif

/**
 * Debug logs control
 */
#if ENABLE_SVC_HP_LOGS == 2
#define cy_svc_log_info(format,...)  printf ("[SHP] "format" \r\n",##__VA_ARGS__);
#define cy_svc_log_err(ret_val,format,...)  printf ("[SHP] [Err:0x%x, Line:%d] "format" \r\n",(unsigned int)ret_val,__LINE__,##__VA_ARGS__);
#define cy_svc_log_err_on_no_isr(ret_val,format,...)  if(false == is_in_isr()) printf ("[SHP] [Err:0x%x, Line:%d] "format" \r\n",(unsigned int)ret_val,__LINE__,##__VA_ARGS__);
#define cy_svc_log_dbg(format,...)  printf ("[SHP] "format" \r\n",##__VA_ARGS__);
#elif ENABLE_SVC_HP_LOGS == 1
#define cy_svc_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SHP] "format" \r\n",##__VA_ARGS__);
#define cy_svc_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SHP] [Err:0x%x, Line:%d] "format" \r\n",(unsigned int)ret_val,__LINE__,##__VA_ARGS__);
#define cy_svc_log_err_on_no_isr(ret_val,format,...)  if(false == is_in_isr()) cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SHP] [Err:0x%x, Line:%d] "format" \r\n",(unsigned int)ret_val,__LINE__,##__VA_ARGS__);
#define cy_svc_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SHP] "format" \r\n",##__VA_ARGS__);
#else
#define cy_svc_log_info(format,...)
#define cy_svc_log_err_on_no_isr(ret_val,format,...)
#define cy_svc_log_err(format,...)
#define cy_svc_log_dbg(format,...)
#endif

#define ENABLE_SVC_HP_CHECK_POINT
#define ENABLE_SVC_HP_CHECK_POINT_APP_API
#define ENABLE_SVC_HP_CHECK_POINT_ISR_CBK

typedef struct
{
	unsigned int svc_hp_check_point_line;
	unsigned int svc_hp_check_point_line_app_api;
	unsigned int svc_hp_check_point_line_isr_cbk;
} svc_hp_check_points;

extern svc_hp_check_points svc_hp_check_point;

#ifdef ENABLE_SVC_HP_CHECK_POINT
#define SVC_HP_CHECK_POINT() { svc_hp_check_point.svc_hp_check_point_line =__LINE__;}
#endif

#ifdef ENABLE_SVC_HP_CHECK_POINT_APP_API
#define SVC_HP_CHECK_POINT_APP_API() { svc_hp_check_point.svc_hp_check_point_line_app_api =__LINE__;}
#endif

#ifdef ENABLE_SVC_HP_CHECK_POINT_ISR_CBK
#define SVC_HP_CHECK_POINT_ISR() { svc_hp_check_point.svc_hp_check_point_line_isr_cbk =__LINE__;}
#endif

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
#ifdef ENABLE_TASK_FOR_SVC_HP
#define SVC_HP_THREAD_NAME              "svc_hp_thread"
#ifndef SVC_HP_THREAD_PRIORITY
#define SVC_HP_THREAD_PRIORITY          (CY_RTOS_PRIORITY_NORMAL)
#endif
#ifndef SVC_HP_THREAD_STACK_SIZE
#define SVC_HP_THREAD_STACK_SIZE        (2*1280)
#endif
#ifndef MAX_HP_CMD_Q_SUPPORTED_SIZE
#define MAX_HP_CMD_Q_SUPPORTED_SIZE (40)
#endif
#endif

#ifndef COMPONENT_CM4
//#define ENABLE_IPC_ACCESS_BY_ISR
#endif

#ifndef MID_IPC_QUEUE_CHN
#if defined(COMPONENT_CM4) || defined (COMPONENT_CYW20829)
#define MID_IPC_QUEUE_CHN 1
#else
#define MID_IPC_QUEUE_CHN CYHAL_IPC_CHAN_7
#endif
#endif

extern bool is_in_isr();

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
extern uint8_t	ipc_send_from_hp_to_lp_counter;
extern uint8_t	ipc_recv_from_lp_to_hp_counter;
#endif

extern volatile int svc_hp_isr_counter;
extern volatile int svc_hp_isr_notify_counter;
extern volatile int svc_hp_isr_get_ipc_q_recv_counter;

#if 1
#define SVC_HP_TRACK_HP_ISR_COUNTER()	svc_hp_isr_counter++
#define SVC_HP_TRACK_HP_ISR_NOTIFY_COUNTER() svc_hp_isr_notify_counter++
#define SVC_HP_TRACK_HP_ISR_GET_IPC_Q_RECV_COUNTER() svc_hp_isr_get_ipc_q_recv_counter++
#else
#define SVC_HP_TRACK_HP_ISR_COUNTER()
#define SVC_HP_TRACK_HP_ISR_NOTIFY_COUNTER()
#define SVC_HP_TRACK_HP_ISR_GET_IPC_Q_RECV_COUNTER()
#endif


typedef struct
{
    /*
     * Status to maintain the SVC module init done or not
     */
    volatile bool init_done;

    /*
     * is_deinit_done flag is used to stop user from invoking init API after deinit API is called.
     * Since SVC doesnt support init->deinit->init cycle. Once user invoked deinit API, init API
     * should return error
     */
    volatile bool is_deinit_done;

    volatile bool api_set_allowed;

    /**
     * Instance create parameters (sent by the application)
     */
    cy_svc_hp_config_t init_params;

    /**
     * Stage on which the SVC module is operating.
     */
    volatile cy_svc_stage_t current_stage;


    volatile cy_svc_stage_config_t stage_config_list_from_lp;

    /**
     * Set state sync semaphore, helps to execute
     * state set to SVC LP synchronously.
     */
    cy_semaphore_t cmdProcSyncSVCLPSemaphore;

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_semaphore_t  cmdProcIPCSendSemaphore;
#endif

    cy_queue_t		ipcisr_notify_queue;

    /**
     * Set state result from the SVC LP core for the
     * set state request from SVC HP core.
     */
    volatile cy_rslt_t lp_app_set_state_process_result;

#ifdef ENABLE_TASK_FOR_SVC_HP
    /**
     * SVC thread instance
     */
    cy_thread_t thread_instance;

    /**
     * Quit the thread instance. Required to exit the thread and perform the
     * cleanup required.
     */
    volatile bool quit_thread_instance;

    /**
     * Data Queue handle
     */
    cy_queue_t data_queue;

#endif

#ifdef SVC_HAL_IPC
    cyhal_ipc_t semaphore1;
#else
    unsigned int semaNumber;
#endif
    svc_circular_buffer_header_t circular_shared_buffer;

    cy_svc_set_state_t set_state_dbg;

} svc_hp_instance_t;

/*******************************************************************************
 *                              Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

#endif

#ifdef __cplusplus
}
#endif

#endif /* __SVC_HP_PRIVATE_H__ */
