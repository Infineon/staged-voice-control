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
 * @file cy_staged_voice_control_lp.h
 *
 * @brief This file is the header file for Staged voice control library
 * running on low power domain (M33).
 *
 * Abbreviations used in the header file
 *
 * SVC      -   Stage Voice Control
 * SVC-LP   -   Staged Voice Control MW running in low power domain (CM33)
 * SVC-HP   -   Staged Voice Control MW running in high performance domain (CM55)
 *
 */

#ifndef __CY_STAGED_VOICE_CONTROL_LP_H__
#define __CY_STAGED_VOICE_CONTROL_LP_H__

#include "cy_svc_model.h"
#include "cy_staged_voice_control_common.h"
#include "cy_sod.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \defgroup group_svc_lp_macros SVC_LP Macros
 * \defgroup group_svc_lp_enums SVC_LP Enumerated types
 * \defgroup group_svc_lp_typedefs SVC_LP Typedefs
 * \defgroup group_svc_lp_structures SVC_LP Structures
 * \defgroup group_svc_lp_functions SVC_LP Functions
 */

/*******************************************************************************
 *                          Macros
 ******************************************************************************/

/**
 * \addtogroup group_svc_lp_macros
 * \{
 */
/**
 * Maximum pre-roll buffer configuration. It is safety max check to request
 * for PreRoll Buffer. For actual usecase, the application should choice the
 * right pre-roll buffer count.
 */
#define MAX_PRE_ROLL_BUFFER_REQ_PERCENTAGE (90.0)

/** \} group_svc_lp_macros */
/*******************************************************************************
 *                          Constants
 ******************************************************************************/

/*******************************************************************************
 *                          Enumerations
 ******************************************************************************/
/**
 * \addtogroup group_svc_lp_enums
 * \{
 */
/**
 * External Low performance WWD detection state.
 */
typedef enum
{
    /**
     * External LPWWD State is invalid
     */
    CY_SVC_EXTERNAL_LPWWD_WWD_STATE_INVALID,

    /**
     * WWD detection is in progress with External LPWWD
     */
    CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_IN_PROGRESS,

    /**
     * WWD detection is detected with External LPWWD
     */
    CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_SUCCESS,

    /**
     * WWD detection failed with External LPWWD
     */
    CY_SVC_EXTERNAL_LPWWD_WWD_DETECION_FAILED,

    /**
     * WWD detection max - invalid enum
     */
    CY_SVC_EXTERNAL_LPWWD_WWD_STATE_MAX

} cy_svc_lp_external_lpwwd_state_t;


/**
 * Memory ID configuration
 */
typedef enum
{
    CY_SVC_MEM_ID_INVALID, /* Memory ID Invalid */
	CY_SVC_MEM_ID_ML_ARENA_BUFFER, /* ML internal scratch ARENA Buffer */
	CY_SVC_MEM_ID_FEATURE_BUFFER, /* ML internal scratch ARENA Buffer */
	CY_SVC_MEM_ID_INTERCORE_SHARED_BUFFER, /* Intercore shared buffer */
	CY_SVC_MEM_ID_INTERNAL_BUF_HDR, /* Internal Buf header shared buffer */
	CY_SVC_MEM_ID_GENERIC_MEMORY, /* Generic memory */
	CY_SVC_MEM_ID_ALGO_SCRATCH_MEMORY, /* Algo Scratch memory */
	CY_SVC_MEM_ID_ALGO_PERSISTENT_MEMORY, /* Algo persistent memory */
	CY_SVC_MEM_ID_MAX
} cy_svc_lp_mem_id_t;


/** \} group_svc_lp_enums */

/*******************************************************************************
 *                          Type Definitions
 ******************************************************************************/
/**
 * \addtogroup group_svc_lp_typedefs
 * \{
 */
/**
 * Callback to get notification on different events from staged voice control
 *
 * Application should handle the event from staged voice control without
 * blocking the event callback.
 *
 * @param[in] event                 Event type which occurred
 * @param[in] callback_user_arg     User argument passed in cy_svc_lp_init
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
typedef cy_rslt_t (*cy_svc_lp_event_callback_t)(
        cy_svc_event_t event,
        void *callback_user_arg);

/**
 * Callback to provide the data to SVC's application for any
 * external low power wake word detection.
 *
 * Application should return the buffer as soon as the callback ends. Application
 * should not use this buffer after callback returns). Application may decide
 * to copy the buffer or process in the callback itself and it depends the
 * external application implementation. Holding this callback for longer time
 * may results in overwrite of the internal buffer due to data feed through
 * cy_svc_lp_feed()
 *
 * @param[in] frame_buffer          frame buffer
 * @param[in] frame_count           frame count
 * @param[in] callback_user_arg     application passed user argument
 * @param[out] lpwwd_state          application returns the lpwwd stage to SVC.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
typedef cy_rslt_t (*cy_svc_lp_data_callback_t)(
        CY_SVC_DATA_T *frame_buffer,
        uint32_t frame_count,
        void *callback_user_arg,
        cy_svc_lp_external_lpwwd_state_t *lpwwd_state);



/**
 * Memory Callback to get memory from the application. Application may choose the location of the memory,
 * the memory can be from any section, sections like, DTCM, SOCMEM, SRAM1, SRAM0, ITCM, RRAM, etc.,
 *
 * Depends the memory choice the performance of algorithm will vary. The application needs to be
 * chose wisely the best available memory for the purpose.
 *
 * @param[in] mem_id               Memory Id, Application may choose the memory location depends on
 *                                 the memory id.
 * @param[in] size                 Size of the memory
 * @param[out] buffer              Pointer to Pointer of the memory.
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_svc_lp_alloc_memory_callback_t)(cy_svc_lp_mem_id_t mem_id,
		uint32_t size, void **buffer);

/**
 * Memory Callback to free memory back to the application. The memory which is allocated
 * using cy_svc_lp_alloc_memory_callback_t will be freed back to the application using this
 * callback.
 *
 * @param[in] mem_id               Memory Id.
 * @param[in] buffer               Pointer to the memory.
 *
 * @return    cy_rslt_t
 */
typedef cy_rslt_t (*cy_svc_lp_free_memory_callback_t)(cy_svc_lp_mem_id_t mem_id,
		void *buffer);


/** \} group_svc_lp_typedefs */

/*******************************************************************************
 *                          Structures
 ******************************************************************************/
/**
 * \addtogroup group_svc_lp_structures
 * \{
 */
/**
 * SVC configuration structure for low power domain.
 */
typedef struct
{
    /**
     * List of the features enabled/disabled depends on the application's choice.
     * It is a bit-mask, Application can enable one or more multiple features.
     */
    cy_svc_stage_config_t stage_config_list;

    /**
     * User argument for callback.
     */
    void *callback_user_arg;

    /**
     * Callback functions for event from SVC module to application. Registering
     * this event callback is recommended. Alternatively, the application may
     * chose not to register and application can use the API cy_svc_lp_get_current_stage
     * to get the state.
     *
     * \note Application should not call any SVC LP apis from this callback. Calling
     * SVC apis may results in blocking the SVC internal thread.
     */
     cy_svc_lp_event_callback_t event_callback;

    /**
     * Audio feed Data Info: Audio Data Type
     */
    cy_svc_audio_input_type_t audio_input_type;

    /**
     * Audio feed Data Info: Sample rate of the input data.
     * Currently only 16KHz is supported.
     */
    cy_svc_sample_rate_t sample_rate;

    /**
     * Audio feed Data Info: Single audio frame is assumed to be of 10ms worth
     * of data. Currently only 10ms is supported. Hence application needs to
     * submit audio frames for every 10ms worth of data CY_SVC_LP_SUPPORTED_FRAME_TIME_MS
     */
    unsigned int single_frame_time_ms;

    /**
     * Setting this flag to "true" indicate that, IFX LPWWD will not be used
     * instead, application will use third party LPWWD.
     */
    bool is_lpwwd_external;

    /**
     * Callback function needs to register to get audio data. This callback
     * registration is mandatory only if "is_lpwwd_external" is set to true.
     * Application can feed the data to the third party LPWWD processing.
     */
    cy_svc_lp_data_callback_t   lpwwd_external_data_callback;

    /**
     * Once the LPWWD is detected, the pre-roll frame count will be used to
     * send the saved buffer to the high performance domain.
     *
     *      |--------------------------------|
     *      |       Buffer of X sec          |
     *      |--------------------------------|
     *      t0 t1 ..                        tn
     *                      ^
     *                      |
     *                      LWWD detect frame pointer
     *
     *         |<--PreRoll->|
     *
     *  Error will be thrown if the PreRollBuffer occupies more than
     *  MAX_PRE_ROLL_BUFFER_REQ_PERCENTAGE % of of total frame
     *  buffers (in (SVC_SHARED_BUFFER_SIZE))
     */
    unsigned int pre_roll_frame_count_from_lpwwd_detect_frame;

    /**
     * Sensitivity, The detection sensitivity (0- 32767)
     * 0 = least sensitive
     * 32767 (MAX_SOD_SENSITIVITY) = most sensitive
     * 16384 = nominal sensitive
     */
    int sod_sensitivity;

    /**
     * Onset gap settings. Generally, a talker will pause momentarily before
     * addressing a person, or in this case, a device.  The pause is
     * approximately in the range of 200-500ms in conversational speech.
     *
     * Allowed gap settings are:
     * CY_SOD_ONSET_GAP_SETTING_1000_MS,
     * CY_SOD_ONSET_GAP_SETTING_500_MS,
     * CY_SOD_ONSET_GAP_SETTING_400_MS,
     * CY_SOD_ONSET_GAP_SETTING_300_MS
     * CY_SOD_ONSET_GAP_SETTING_200_MS
     * CY_SOD_ONSET_GAP_SETTING_100_MS
     * CY_SOD_ONSET_GAP_SETTING_0_MS
     *
     * For wake-word detection, better behavior is seen with a
     * CY_SOD_ONSET_GAP_SETTING_400_MS gap setting.
     */
    int sod_onset_gap_setting_ms;

    /**
     * SOD onset detect late hit delay. After SOD detect, the
     * buffer would be look backed with this delay, to get the
     * correct/approximate close onset of the speech.
     */
    unsigned int sod_onset_detect_max_late_hit_delay_ms;

#ifndef  ENABLE_LOW_POWER_SVC
    /**
     * IPC communication sync semaphore. It should be any one of semaphore
     * from the range (CYHAL_IPC_SEMA_COUNT)
     *
     * Application should take care of assigning unique IPC sync semaphore
     * number for staged voice control MW. Additionally the application needs to
     * ensure by passing the same IPC communication sync semaphore in both
     * SVC-LP and SVC-HP init.
     */
	unsigned int ipc_communication_sync_sempahore;

#endif


#ifdef ENABLE_SVC_IPC_BY_PDL_PIPE
	/**
	 * IPC configuration using PDL PIPE
	 */
	cy_svc_ipc_pipe_configuration_t	ipc_pipe_config;
#endif

    /** Memory Callback to get memory from the application. */
    cy_svc_lp_alloc_memory_callback_t alloc_memory;

    /** Memory Callback to free memory back to the application. */
    cy_svc_lp_free_memory_callback_t  free_memory;

    /** InterCore shared buffer size */
    unsigned int inter_core_shared_buffer_size;

    /** Mel filter bank low frequency */
    int32_t mel_low_freq;

    /** Mel filter bank high frequency */
    int32_t mel_high_freq;

} cy_svc_lp_config_t;

/**
 * Low noise detection configuration structure for low power domain.
 */
typedef struct
{
    /**
     * Timeout in milliseconds to declare audio as silence.
     */
    uint32_t timeout_ms;

    /**
     * Amplitude threshold to declare audio as silence.
     */
    uint16_t low_noise_threshold;

    /**
     * Enable/Disable low noise detection feature.
     */
    bool enable_feature;

} cy_svc_lp_low_noise_config_t;

/**
 * Gain configuration structure for low power domain.
 */
typedef struct
{
    /**
     * Gain value to be applied post High Pass Filter (HPF) processing.
     * Typically represented as a linear gain factor.
     * A value of '1' or '0' means unity gain (no change).
     * A value of '3' amplifies the signal by 10db
     */
    uint8_t post_hpf_gain;

    /**
     * Gain value to be applied before SOD (Speech On Detection) processing.
     * Typically represented as a linear gain factor. This is applied to
     * ensure the SOD works optimally at various input signal levels.
     * A value of '1' or '0' means unity gain (no change).
     * A value of '3' amplifies the signal by
     *
     * This gain is only for SOD processing and does not affect the output audio
     * sent to the high performance core.
     */
    uint8_t sod_processing_gain;

} cy_svc_lp_gain_config_t;

/** \} group_svc_lp_structures */

/*******************************************************************************
 *                          Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                          Function Declarations
 ******************************************************************************/
/**
 * \addtogroup group_svc_lp_functions
 * \{
 */
/**
 * Initializes the staged voice control module in low power device, creates the
 * required resources based on the input configuration, the resource could be
 * buffer, internal task, IPC communication, etc.,
 *
 * \note: The IPC resources required for SVC MW is created by SVC-LP MW in
 * cy_svc_lp_init(), the SVC-HP MW will get the created resource in cy_svc_hp_init.
 * Application needs to ensure the sequence between these two inits across cores.
 *
 * Preferably, Application needs to perform cy_svc_lp_init and then it can enable
 * the next core (CM55) through the API  Cy_SysEnableCM55(CY_CORTEX_M55_APPL_ADDR, 10U).
 * This will help ensure the sequence between these two APIs across cores.
 *
 * @param[in]  init                 Staged voice control module init
 *                                  configuration parameter in low power domain.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_init(cy_svc_lp_config_t *init);

/**
 * Get the current stage of staged voice control module. Get stage API
 * returns immediately with the current stage of SVC LP MW.
 *
 * By any chance, If there is any stage transition is happening in SVC,
 * this API will return the current stage and it will not return the new
 * stage which is being applied. In that case application has to call
 * this API again.
 *
 * Additionally the cy_svc_lp_set_stage API is synchronous. Application
 * can call the  cy_svc_lp_get_current_stage api after the
 * cy_svc_lp_set_stage API from single thread/task.
 *
 * @param[out]  stage                staged voice control module's current stage
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_get_current_stage(cy_svc_stage_t *stage);

/**
 * Feed input audio data to staged voice control module in low power domain.
 *
 * @param[in]  data                 Pointer to the input audio data.
 *                                  Application can free/reuse the buffer pointer
 *                                  passed in this API once the API is returned.
 *                                  (i.e., Staged voice module will make a copy
 *                                  of the input data internally in its buffer).
 *                                  Length of the input audio data passed in the
 *                                  buffer is constant should not vary for every
 *                                  feed.(i.e, Application should feed always the
 *                                  complete buffer).
 *                                  If Application needs to feed stereo data,
 *                                  then the data format must be non-interleaved
 *                                  format. example: For stereo data of 10ms
 *                                  worth, first 320bytes must be of channel-1
 *                                  and then next 320bytes must be channel-2.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_feed(CY_SVC_DATA_T *data);

/**
 * SVC-LP application can force the stage in SVC module.
 *
 * Set stage API is synchronous, It successfully applies the stage requested
 * internally in the SVC LP (internal thread) MW and then this API will be
 * returned to the caller with the appropriate return results.
 *
 * @param[in] stage                 Required stage to set.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_set_stage(cy_svc_stage_t stage);

/**
 * Deinit the staged voice control module in low power device, deletes the
 * resources created during cy_svc_lp_init.
 *
 * \note After invoking \ref cy_svc_lp_deinit on either core,
 * \ref cy_svc_lp_init cannot be invoked again. The expectation is that
 * \ref cy_svc_lp_init and \ref cy_svc_lp_deinit should be invoked only once.
 *
 * \note Application should take care of not calling any of SVC MW LP APIs (cy_svc_lp_feed
 * ,cy_svc_lp_get_current_stage and cy_svc_lp_set_stage APIs) when the
 * cy_svc_lp_deinit is in progress. During deinit if the application calls any
 * of the API, it may results in crash. Adding protection internally between
 * these apis and cy_svc_lp_deinit results in unwanted synchronous mechanism and adds
 * load to the system. The usecase of cy_svc_lp_deinit is very rare. Hence application
 * should take care of this condition.
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_deinit(void);

/**
 * Sets the info of the HP core. This is needed to synchronize the
 * IPC communication between SVC LP and SVC HP core.
 *
 * @param[in]  core_info                HP core information
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_svc_lp_set_hp_core_info(cy_svc_lp_set_hp_core_info_t core_info);

/**
 * @brief Pre-send hook for IPC communication in low power mode.
 *
 * This weak function can be overridden by the user to perform actions
 * before sending IPC messages in low power mode.
 */
void cy_svc_lp_ipc_pre_send_hook(void);

/**
 * @brief Post-send hook for IPC communication in low power mode.
 *
 * This weak function can be overridden by the user to perform actions
 * after sending IPC messages in low power mode.
 */
void cy_svc_lp_ipc_post_send_hook(void);

/**
 * @brief Post-receive hook for IPC communication in low power mode.
 *
 * This weak function can be overridden by the user to perform actions
 * after receiving IPC messages in low power mode.
 */
void cy_svc_lp_ipc_post_recv_hook(void);


/**
 * @brief Thread start hook for staged voice control in low power mode.
 *
 * This weak function can be overridden by the user to perform actions
 * when the staged voice control thread starts in low power mode.
 */
void cy_svc_lp_thread_start_hook(void);


/**
 * @brief Configure low noise settings for the staged voice control low power module.
 *
 * This function applies the specified low noise configuration parameters to optimize
 * the voice control system for low noise environments. The configuration affects
 * noise reduction algorithms and sensitivity thresholds.
 *
 * @param[in] low_noise_config Pointer to the low noise configuration structure
 *                            containing the desired noise reduction parameters.
 *                            Must not be NULL.
 *
 * @return cy_rslt_t Result code indicating the success or failure of the operation.
 *
 */
cy_rslt_t cy_svc_lp_low_noise_config(cy_svc_lp_low_noise_config_t *low_noise_config);

/**
 * @brief Configure gain settings for the staged voice control low power module.
 *
 * This function applies the specified gain configuration parameters to adjust
 * the audio signal levels within the voice control system. The configuration
 * affects the amplification applied at various stages of audio processing.
 *
 * @param[in] gain_config Pointer to the gain configuration structure
 *                        containing the desired gain parameters.
 *                        Must not be NULL.
 *
 * @return cy_rslt_t Result code indicating the success or failure of the operation.
 *
 */
cy_rslt_t cy_svc_lp_gain_config(cy_svc_lp_gain_config_t *gain_config);

/** \} group_svc_lp_functions */

#ifdef __cplusplus
}
#endif

#endif /* __CY_STAGED_VOICE_CONTROL_LP_H__ */
