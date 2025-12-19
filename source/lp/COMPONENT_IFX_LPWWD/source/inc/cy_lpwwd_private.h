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

#ifndef __CY_LPWWD_PRIVATE_H__
#define __CY_LPWWD_PRIVATE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cy_lpwwd_prewwd.h"
#include "cy_lpwwd_ml.h"
#include "cy_lpwwd_postwwd.h"
#ifdef ENABLE_LPWWD_MW_WITH_SOD
#include "cy_sod.h"
#endif
#include "cy_lpwwd_defines.h"
#include "cy_log.h"
#ifdef COMPONENT_MEMORY_CHECK_UTILS
#include "cy_mem_check_utils.h"
#endif
#include "math.h"
#ifdef ENABLE_SOD_PROFILLING
#include "profiler.h"
#endif
#include "cy_lpwwd_common.h"

#include "staged_voice_control_lp_private.h"

#if ENABLE_LPWWD_LOGS == 2
#define cy_lpwwd_log_info(format,...)  printf ("[LPWWD] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_log_err(ret_val,format,...)  printf ("[LPWWD] [Err:0x%"PRIx32", %s:%d] "format" \r\n",(uint32_t)ret_val,__FUNCTION__,__LINE__,##__VA_ARGS__);
#define cy_lpwwd_log_dbg(format,...)  printf ("[LPWWD] "format" \r\n",##__VA_ARGS__);
#elif ENABLE_LPWWD_LOGS == 1
#define cy_lpwwd_log_info(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_INFO,"[LPWWD] "format" \r\n",##__VA_ARGS__);
#define cy_lpwwd_log_err(ret_val,format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_ERR,"[LPWWD] [Err:0x%"PRIx32"] "format" \r\n",(uint32_t)ret_val,##__VA_ARGS__);
#define cy_lpwwd_log_dbg(format,...)  cy_log_msg (CYLF_MIDDLEWARE,CY_LOG_DEBUG,"[LPWWD] "format" \r\n",##__VA_ARGS__);
#else
#define cy_lpwwd_log_info(format,...)
#define cy_lpwwd_log_err(ret_val,format,...)
#define cy_lpwwd_log_dbg(format,...)
#endif

#ifdef ENABLE_SOD_PROFILLING
#define FRAME_COUNT 100 // 100 frames each with 10ms data
#endif

#define PRINT_ARR_AS_CHAR(__X__,__Y__) \
{ \
    const unsigned char *temp = (const unsigned char *) __Y__; \
    cy_lpwwd_log_info( "%s [0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X]", __X__,\
    *temp, *(temp + 1), *(temp + 2), *(temp + 3), *(temp + 4), \
    *(temp + 5), *(temp + 6), *(temp + 6), *(temp + 8), *(temp + 9)); \
}


typedef struct {
    cy_lpwwd_config_params_t config_input_param;

    uint16_t number_of_features;                       /* Number of features output from PreWWD */
    uint16_t number_of_feature_frames;                 /* Number of feature frames accumulated before feeding ML invoke for score generation */
    uint16_t number_of_tokens;                         /* Number of output tokens from ML */

    uint16_t num_audio_frame_shifts;
    uint16_t audio_frame_shift_count;                   /* Running counter for frame shift */
    uint16_t feature_frame_count;                       /* Current available feature frame counter*/

    uint16_t audio_frame_count;                         /* Current LPWWD frame detection feed counter used for max timeout,
                                                           This has to reset after every lpwwd detection result */

    cy_lpwwd_feature_buffer feature_buffer;             /* Output buffer from PreWWD/Input to ML */
    cy_lpwwd_output_score output_score;                 /* Output Score from ML */
    LPWWD_POSTWWD_IP_DATA_TYPE_T *output_score_fixed;   /* Input Score to PostWWD */

    /**
     * stats
     */
    uint16_t wwd_detected_count;
    uint32_t audio_frame_index;                     /* Overall lpwwd feed frame counter, this will never reset */
    uint32_t audio_frame_index_after_sod;                  /** feed counter after SOD detect, this will never reset*/

    cy_lpwwd_postwwd_config_params_t  postwwd_config;
    cy_lpwwd_prewwd_config_params_t   prewwd_config;

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    cy_sod_t sod_handle;
    short circular_buffer[CIRCULAR_BUFFER_SIZE];
    short wwd_audio_frame[WWD_CAPTURE_BUFFER_SIZE];
    unsigned long write_index;
    unsigned long read_index;
    unsigned long wwd_audio_read_idx;
    uint16_t sod_hit_count;
    bool sod_detected;
    bool sod_trigger_status;
#ifdef ENABLE_SOD_PROFILLING
	bool first_sod_detection;
	int total_count;
	int total_count_sod;
	int frame_counter_for_sod;
	int frame_counter_after_sod;
#endif
#endif
} cy_lpwwd_context_t;


#endif
