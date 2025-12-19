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
 * @file staged_voice_control_lp_private.h
 *
 */

#ifndef __SVC_LP_PRIVATE_H__
#define __SVC_LP_PRIVATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_LP_MW
#include "cy_log.h"
#include "cy_result.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include <inttypes.h>
//#include "cy_buffer_pool.h"
#include "cy_pdl.h"
#include "cyabs_rtos.h"
#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_common.h"

#ifdef ENABLE_TIMELINE_MARKER
#include "audio_timeline_marker.h"
#endif

#ifdef ENABLE_USB_DBG_OUTPUT
#include "audio_usb_send_utils.h"
#endif

#ifdef ENABLE_IFX_LPWWD
#include "cy_lpwwd.h"
#endif

#if COMPONENT_MEMORY_CHECK_UTILS
#include "cy_mem_check_utils.h"
#endif

#include "cy_sod.h"

#include "staged_voice_control_dbg_control.h"
#include "cy_pdl.h"

#include "staged_voice_control_ipc.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/

#define ENABLE_SVC_LOW_LATENCY_PROFILE
//#define ENABLE_HPF_STUB
#define READABLE_SVC_LOG
//#define ENABLE_CBUF_GUARD_BYTES
//#define ENABLE_CBUF_SHARE_ACCCESS_TRACKER


#define ENABLE_SVC_LP_CHECK_POINT
#define ENABLE_SVC_LP_CHECK_POINT_HP_MSG
#define ENABLE_SVC_LP_CHECK_POINT_ISR
#define ENABLE_SVC_LP_CHECK_POINT_APP_API

typedef struct
{
	unsigned int svc_lp_check_point_line;
	unsigned int svc_lp_check_point_line_hp_msg;
	unsigned int svc_lp_check_point_line_isr;
	unsigned int svc_lp_check_point_line_app_api;
} svc_lp_check_points;

extern svc_lp_check_points svc_lp_check_point;

#ifdef ENABLE_SVC_LP_CHECK_POINT
#define SVC_LP_CHECK_POINT() { svc_lp_check_point.svc_lp_check_point_line =__LINE__;}
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT_HP_MSG
#define SVC_LP_CHECK_POINT_HP_MSG() { svc_lp_check_point.svc_lp_check_point_line_hp_msg =__LINE__;}
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT_ISR
#define SVC_LP_CHECK_POINT_ISR() { svc_lp_check_point.svc_lp_check_point_line_isr =__LINE__;}
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
#define SVC_LP_CHECK_POINT_APP_API() { svc_lp_check_point.svc_lp_check_point_line_app_api =__LINE__;}
#endif



#if ENABLE_SVC_LP_LOGS == 2
#define cy_svc_log_info(format,...)  printf ("[SLP] "format" \r\n",##__VA_ARGS__);
#define cy_svc_log_err(ret_val,format,...)  printf ("[SLP] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_svc_log_err_on_no_isr(ret_val,format,...)  if(false == is_in_isr()) printf ("[SLP] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_svc_log_dbg(format,...)  printf ("[SLP] "format" \r\n",##__VA_ARGS__);
#elif ENABLE_SVC_LP_LOGS
#define cy_svc_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SLP] "format" \r\n",##__VA_ARGS__);
#define cy_svc_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SLP] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_svc_log_err_on_no_isr(ret_val,format,...)  if(false == is_in_isr()) cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SLP] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(unsigned int)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_svc_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[SLP] "format" \r\n",##__VA_ARGS__);
#else
#define cy_svc_log_info(format,...)
#define cy_svc_log_err(format,...)
#define cy_svc_log_err_on_no_isr(format,...)
#define cy_svc_log_dbg(format,...)
#endif

#define SVC_PROCESS_THREAD_NAME              "svc_lp_thread"

#ifndef SVC_THREAD_PRIORITY
#define SVC_THREAD_PRIORITY                  (CY_RTOS_PRIORITY_ABOVENORMAL)
#endif

#ifndef COMPONENT_CM4
//#define ENABLE_IPC_ACCESS_BY_ISR
#endif

//#define CREATE_SVC_LP_THREAD_STACK_IN_SOCMEM_DATA

#ifndef MID_IPC_QUEUE_CHN
#if defined(COMPONENT_CM4) || defined (COMPONENT_CYW20829)
#define MID_IPC_QUEUE_CHN 1
#else
#define MID_IPC_QUEUE_CHN CYHAL_IPC_CHAN_7
#endif
#endif

#ifdef ENABLE_SVC_IPC_COUNTER_TRACK
extern uint8_t	ipc_send_from_lp_to_hp_counter;
extern uint8_t	ipc_recv_from_hp_to_lp_counter;
#endif

#ifndef SVC_THREAD_STACK_SIZE
#define SVC_THREAD_STACK_SIZE                (3*1024)
#endif

#define SAMPLE_SIZE_IN_BYTES (2)

#define FRAME_DATA_SIZE(NO_OF_CHANNELS,BYTES_PER_SAMPLE,SAMPLE_RATE,SINGLE_FRAME_MS) \
       (NO_OF_CHANNELS * BYTES_PER_SAMPLE * SAMPLE_RATE * SINGLE_FRAME_MS / 1000 )

#ifndef SVC_MAX_CMD_Q_SUPPORTED_SIZE
#define SVC_MAX_CMD_Q_SUPPORTED_SIZE (10)
#endif

//TODO: Remove later, once the completed functionality is verified.
//Right now fixed to the PreRollBufferCount
#define MAX_DATA_Q_SUPPORTED_EXTRA_FROM_PREROLL_BUFFER (50)

#define FIND_MIN(a,b) (((a)<(b))?(a):(b))

extern bool is_in_isr();

#if 0
#define IPC_SEND_GAP_DELAY_SIMULATION(_X_) cy_rtos_delay_milliseconds(_X_);
#else
#define IPC_SEND_GAP_DELAY_SIMULATION(_X_)
#endif


/** Memory Callback to get memory from the application. */
extern cy_svc_lp_alloc_memory_callback_t svc_alloc_memory;

/** Memory Callback to free memory back to the application. */
extern cy_svc_lp_free_memory_callback_t  svc_free_memory;

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


/**
 * States set by SVC-LP through cy_svc_lp_set_state.
 */
typedef enum
{
    /*
     *  Invalid set state minimum value.
     */
    CY_SVC_LP_HP_CORE_STATE_INVALID,

    //HP core is trigerred to enable
    CY_SVC_LP_HP_CORE_STATE_ENABLE_TRIGERRED,

    //HP core is enabled and SVC HP is ready
    CY_SVC_LP_HP_CORE_STATE_SVC_HP_READY,

    //HP core is trigerred to disable
    CY_SVC_LP_HP_CORE_STATE_DISABLE_TRIGERRED,

    CY_SVC_LP_HP_CORE_STATE_MAX

} cy_svc_lp_hp_state_t;


/**
 * Staged voice control event types
 */
typedef enum
{
    //Invalid Event with value 0.
    SVC_TRIGGER_INVALID,

    SVC_TRIGGER_INSTANCE_INIT_DONE,

    SVC_TRIGGER_PUT_TO_DEEP_SLEEP,

    // Force the stage to acoustic activity detection
    SVC_TRIGGER_ACOUSTIC_ACTIVITY_FORCEFULLY,

    // Trigger the wait for Speech onset detection forcefully.
    SVC_TRIGGER_WAIT_FOR_SPEECH_DETECTION_FORCEFULLY,

    // Trigger to wait for LPWWD */.
    SVC_TRIGGER_WAIT_FOR_LOW_POWER_WAKEWORD_DETECTION_FORCEFULLY,

    SVC_TRIGGER_WAIT_FOR_HIGH_POWER_WAKEWORD_DETECTION_FORCEFULLY,

    // Trigger ASR Detection
    SVC_TRIGGER_ASR_DETECTION_FORCEFULLY,

    // Acoustic activity detected event or wake up from Deep sleep
    SVC_TRIGGER_ACOUSTIC_ACTIVITY_DETECTED,

    // Speech detected event
    SVC_TRIGGER_SPEECH_ONSET_DETECTED,

    // Low power wakeup word detected event
    SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED,

    // Low power wakeup word not detected event
    SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_NOT_DETECTED,

    // High power wakeup word detected event.
    // Actual source of this event is from High power domain.
    SVC_TRIGGER_HIGH_POWER_WAKEUP_WORD_DETECTED,

    // High power wakeup word not detected event
    // Actual source of this event is from High power domain.
    SVC_TRIGGER_HIGH_POWER_WAKEUP_WORD_NOT_DETECTED,

    // ASR detetced event
    // Actual source of this event is from High power domain.
    SVC_TRIGGER_ASR_DETECTED,

    // ASR not detected event
    // Actual source of this event is from High power domain.
    SVC_TRIGGER_ASR_NOT_DETECTED,

    // ASR processing completed event
    // Actual source of this event is from High power domain.
    SVC_TRIGGER_ASR_PROCESSING_COMPLETED,

    SVC_TRIGGER_PRINT_STATISTICS,

    SVC_TRIGGER_SEND_POST_WWD_HPWWD_DET_IN_PROGRESS,

    // Low noise detected event
    SVC_TRIGGER_LOW_NOISE_DETECTED,

    //Max Event supported
    SVC_TRIGGER_MAX

} svc_stage_trigger_t;

/* List of events */
#define SVC_LP_CMD_ID_DATA_RECEIVED (1)
#define SVC_LP_CMD_ID_SET_STATE_FROM_HP     (2)
#define SVC_LP_CMD_ID_SET_STAGE_FROM_LP_APP     (3)

typedef struct
{
    union {
    	uint32_t			   set_value;
        cy_svc_set_state_t     set_state;
        cy_svc_stage_t         set_stage;
    };

    uint8_t payload_internal[MAX_SET_STATE_INFO_SIZE_IN_BYTES];

    uint8_t cmd;

    uint8_t reserved[3];
} cmd_q_msg_t;


typedef struct
{
    uint8_t cmd;

    uint8_t reserved[3];

    unsigned int data_crc_check;

    char *cbuf_pointer;

} data_q_msg_t;


typedef struct
{
    /**
     * Statistics at various levels
     */
    unsigned int frame_counter_received_after_last_aad_dbg;

    volatile unsigned int svc_lp_feed_post_fail_counter_dbg;
    volatile unsigned int svc_lp_feed_post_fail_counter_track_dbg;
    unsigned int crc_check_fail_counter_dbg;

    unsigned int sod_detect_counter_dbg;

    unsigned int lpwwd_detect_counter_dbg;
    unsigned int lpwwd_not_detect_counter_dbg;
    unsigned int lpwwd_feed_counter_dbg;

    unsigned int hpwwd_detect_counter_dbg;
    unsigned int hpwwd_not_detect_counter_dbg;
    unsigned int hpwwd_feed_counter_dbg;

    unsigned int asr_detect_counter_dbg;
    unsigned int asr_not_detect_counter_dbg;
    unsigned int asr_process_completed_counter_dbg;

    cy_time_t last_frame_feed_time_ms;

    /**
     * Remove below params
     */
    volatile unsigned int svc_lp_feed_post_fail_reason_dbg;

} cy_svc_lp_stats_t;

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
    cy_svc_lp_config_t init_params;

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

    /**
     * CMD Queue handle
     */
    cy_queue_t cmd_queue;

    /**
     * Stage on which the SVC module is operating.
     */
    volatile cy_svc_stage_t current_stage;

    /**
     * cmd process semaphore, helps to synchronous command processing
     */
    cy_semaphore_t cmdProcSyncSemaphore;

#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
    cy_semaphore_t cmdProcIPCSendSemaphore;
#endif

    cy_queue_t		ipcisr_notify_queue;

    volatile cy_rslt_t lp_app_set_stage_process_result;

    /**
     * SOD informations
     */
    cy_sod_t sod_handle;

    bool sod_detected;

    /**
     * Circular buffer information
     */
    svc_circular_buffer_header_t *circular_shared_buffer;

    unsigned char *last_processed_frame_start_address;

    bool lpwwd_detected;
    volatile bool config_update_to_hp_done;
    bool lpwwd_feed_past_2_buffers_on_start_completed;

    unsigned char *hpwwd_trigger_data_final_address;

    cy_svc_lp_stats_t stats;

#ifdef ENABLE_IFX_LPWWD
    /* LPWWD handle */
    cy_lpwwd_handle_t lpwwd_handle;
#endif

    bool stream_requested_on_asr_processing_query_state;

    uint16_t post_wwd_frame_count_req_by_hp;
    uint16_t post_lpwwd_received_frame_count;

    /**
     * Deprecated parameter from init, if init, it has to be added later.
     * This will help to provide automatic post wwd buffer.
     */
    unsigned int post_roll_frame_count_from_lpwwd_detect_frame;
    unsigned int post_hpwwd_pending_frame_counter_to_hp;

    /**
     * Enable automatic detection of frame feed discontinuity. In case if the
     * LP core goes to <DeepSleep> and <wake up>. There will be discontinuity in the
     * data feed time. Discontinuity in the data can be detected using the time
     * of the current audio data feed and the last audio data feed.
     */
    bool enable_auto_detect_frame_feed_discontinuity;


    /**
     * Automatic frame feed discontinuity detect timeout. if application
     * sets 1000, then feed discontinuity will be detected if the last frame
     * and the current feed differs by 1sec. If there is a discontinuity, then
     * SVC MW will send an event #CY_SVC_EVENT_ACOUSTIC_ACTIVITY_DETECTED.
     *
     * Acoustic activity is assumed as there is first frame feed after deep sleep.
     *
     * Detection of frame feed discontinuity helps to internal reset of SOD and
     * LPWWD.
     */
    unsigned int auto_frame_feed_discontinuity_timeout_ms;

#ifdef SVC_HAL_IPC
    cyhal_ipc_t semaphore0;
#else
    unsigned int semaNumber;
#endif

    /**
     * High pass filter info
     */
    void *hpf_info;

    /**
	 * High pass filter Handle
	 */
    void *hpf_handle;

    /**
     * The stage forced by the SVC LP APP
     */
    cy_svc_stage_t stage_force_start;

    cy_svc_set_state_t set_state_last_dbg;

    volatile cy_svc_lp_hp_state_t svc_hp_state;

} svc_lp_instance_t;



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

#endif /* __SVC_LP_PRIVATE_H__ */
