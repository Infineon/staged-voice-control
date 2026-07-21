/******************************************************************************
* File Name: cy_lpwwd.h
*
* Description: APIs & data structures for Low power wake word detection.
*
*******************************************************************************
* (c) 2021, Infineon Technologies Company. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Infineon Technologies Company (Infineon) or one of its
* subsidiaries and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Infineon hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Infineon's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Infineon.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Infineon
* reserves the right to make changes to the Software without notice. Infineon
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Infineon does
* not authorize its products for use in any products where a malfunction or
* failure of the Infineon product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Infineon's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Infineon against all liability.
*******************************************************************************/

/*******************************************************************************
* Include header file
******************************************************************************/

#ifndef __CY_LPWWD_H__
#define __CY_LPWWD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cy_lpwwd_error.h"
#ifdef ENABLE_LPWWD_MW_WITH_SOD
#include "cy_sod.h"
#endif

/**
 * \defgroup group_lpwwd_macros LPWWD Macros
 * \defgroup group_lpwwd_enums LPWWD Enumerated types
 * \defgroup group_lpwwd_typedefs LPWWD Typedefs
 * \defgroup group_lpwwd_structures LPWWD Structures
 * \defgroup group_lpwwd_functions LPWWD Functions
 */

/**
 * LPWWD handle
 * */
typedef void* cy_lpwwd_handle_t;

/*******************************************************************************
 *                          Macros
 ******************************************************************************/
/**
 * \addtogroup group_lpwwd_macros
 * \{
 */
/**
 * Max LPWWD detect timeout counter samples, (no. of the times the cy_lpwwd_feed
 * is used to feed the data).
 */
#define MAX_LPWWD_FEED_FRAMES_COUNT_FOR_WW_DETECT_TIMEOUT (4 * 100)    //4 second

/**
 * HMM post processing max WW series. These values are only relevant
 * if HMM based post processing is enabled.
 */
#ifndef SVC_WW_SERIES
#define SVC_WW_SERIES (1)
#endif

/**
 * HMM post processing max WW tokens. These values are only relevant
 * if HMM based post processing is enabled.
 */
#ifndef SVC_WW_TOKENS
#define SVC_WW_TOKENS (2)
#endif


/** \} group_lpwwd_macros */

/*******************************************************************************
 *                          Constants
 ******************************************************************************/

/*******************************************************************************
 *                          Enumerations
 ******************************************************************************/

/**
 * \addtogroup group_lpwwd_enums
 * \{
 */

/**
 * Wakeword detected status
 * */
typedef enum {
    /**
     * WW detection result invalid
     * */
    CY_LPWWD_WAKE_WORD_INVALID,
    /**
     * WW detection is in progress. Application needs to feed more data to
     * get the desired WW detection result
     **/
    CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS,
    /**
     * WW detected
     **/
    CY_LPWWD_WAKE_WORD_DETECTED,
    /**
     * WW not detected
     **/
    CY_LPWWD_WAKE_WORD_NOT_DETECTED,
    /**
     * WW rejected
     **/
    CY_LPWWD_WAKE_WORD_REJECTED,
    /**
     * Timeout in detecting the WW. Timeout is internally handled
     **/
    CY_LPWWD_TIMEOUT,
    /**
     * WW detection is failed for unknown reason
     **/
    CY_LPWWD_FAIL_REASON_NOT_KNOWN

} cy_lpwwd_wwd_detect_status_t;


/** \} group_lpwwd_enums */

/*******************************************************************************
 *                          Structures
 ******************************************************************************/
/**
 * \addtogroup group_lpwwd_structures
 * \{
 */
/**
* Configuration parameters needed for the low power wake word init
*/
typedef struct {

    /**
     * Dual Wakeword detection required or not.
     * True: Dual Wakeword detection
     * False: Single Wakeword detection
     */
    bool    dual_wake_word_detection;

    /** Warning !!!
     * All model memory must be persistent and MW will directly use the memory
     * passed and it shall not do any internal memory copy. ie., Application
     * should not free or corrupt the model passed memory.
     * */

    /**
     * ML model binary data
     */
    const char *ml_model_binary_buf;

    /**
     * ML model meta data buffer. This is needed only for IFX Inference
     * Engine. (For this TFliteU Inference engine this buffer will NULL)
     */
    const char *ml_model_meta_buf;


    /**
     * PostProcess HMM model for Keyword on wakeword-1
     */
    const char *hmm_model1_keyword_binary_buf;
    /**
     * PostProcess HMM model for Garbage on wakeword-1
     */
    const char *hmm_model1_garbage_binary_buf;
    /**
     * PostProcess HMM model for Noise on wakeword-1
     */
    const char *hmm_model1_noise_binary_buf;

    /**
     * PostProcess HMM model for Keyword on wakeword-2
     */
    const char *hmm_model2_keyword_binary_buf;
    /**
     * PostProcess HMM model for Garbage on wakeword-2
     */
    const char *hmm_model2_garbage_binary_buf;
    /**
     * PostProcess HMM model for Noise on wakeword-2
     */
    const char *hmm_model2_noise_binary_buf;

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    /**
     * SOD configuration parameters
     */
    cy_sod_config_params              sod_config;
#endif

    /**
     * Feature scale for the model
     */
    float feature_scale;

    /**
     * Feature offset for the model
     */
    int16_t feature_offset;

    /**
     * Lookback buffer length
     */
    int lookback_buffer_length;

    /**
     * Mel filter bank low frequency
     */
    int32_t mel_low_freq;
    /**
     * Mel filter bank high frequency
     */
    int32_t mel_high_freq;

} cy_lpwwd_config_params_t;

/** \} group_lpwwd_structures */

/*******************************************************************************
 *                      Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                      Function Declarations
 ******************************************************************************/
/**
 * \addtogroup group_lpwwd_functions
 * \{
 */
/**
 * Initialize low power wake word middleware.
 *
 * @param[in] config_params   Configuration parameters needed for the low
 *                            power wake word
 *
 * @param[out] handle         Handle for the low power wake word middleware
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_init(cy_lpwwd_config_params_t *config_params,
        cy_lpwwd_handle_t *handle);

/**
 * Initialize low power wake word
 *
 * @param[in] handle           Handle for the low power wake word
 *
 * @param[in] data             Audio input data (~10ms audio frame). Data must be
 *                             of mono input of worth 320bytes (160 samples
 *                             each sample of size 2 bytes). It is always assumed
 *                             that 320 bytes of data is present. Passing
 *                             less data length is incorrect and it can have
 *                             unexpected behavior, Data more than 320bytes will
 *                             not be parsed inside the MW.
 *
 *
 * @param[out] wwd_status      Low power wake word detection status.
 *                             Refer cy_lpwwd_wwd_detect_status_t for the
 *                             various wakeword detection results
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_feed(cy_lpwwd_handle_t handle, char *data,
        cy_lpwwd_wwd_detect_status_t *wwd_status);

/**
 * Reset the low power wake word middleware.
 *
 * @param[in] handle           Handle for the low power wake word.
 *                             Application  should call this api only from the
 *                             task/thread which feeds data using cy_lpwwd_feed
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_reset(cy_lpwwd_handle_t handle);

/**
 * De-Initialize low power wake word middleware
 *
 * @param[in, out] handle      Handle for the low power wake word.
 *                             handle will be set to NULL once returns
 *                             from the API
 *
 * @return    CY_RSLT_SUCCESS on success; an error code on failure.
 */
cy_rslt_t cy_lpwwd_deinit(cy_lpwwd_handle_t *handle);

/** \} group_lpwwd_functions */

#endif
