/******************************************************************************
 * File Name: cy_lpwwd_prewwd.h
 *
 * Description: Header file with APIs & Data structures for pre wake-word.
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
#ifndef __CY_LPWWD_DBG_UTILS_H
#define __CY_LPWWD_DBG_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cy_lpwwd_private.h"
#include "cy_lpwwd.h"

/******************************************************
 *                      Typedefs
 ******************************************************/

#define ENABLE_LPWWD_DBG_FEED_PRINT                         (0x0001)
#define ENABLE_LPWWD_DBG_FEATURE_FRAME_PRINT                (0x0002)
#define ENABLE_LPWWD_DBG_FEATURE_BUF_MATRIX_PRINT           (0x0004)
#define ENABLE_LPWWD_DBG_SCORE_PRINT                        (0x0008)
#define ENABLE_LPWWD_DBG_ML_MODEL_PRINT                     (0x0010)
#define ENABLE_LPWWD_DBG_HMMS_MODEL_PRINT                   (0x0020)
#define ENABLE_LPWWD_DBG_FEED_PRINT_SOD_SHIFT               (0x0040)
#define ENABLE_LPWWD_DBG_FORCE_ML_PROC_ALWAYS               (0x0080)

/******************************************************
 *                      Function Prototypes
 ******************************************************/

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_ML_MODEL_PRINT)
void print_ml_model(const char *model);
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
void print_output_score(cy_lpwwd_context_t *lpwwd_context);
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
void print_output_score_reduced(int16_t *output_score_fixed);
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_FRAME_PRINT)
void cy_lpwwd_print_feature_frame(cy_lpwwd_context_t *lpwwd_context);
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_BUF_MATRIX_PRINT)
void printf_feature_buffer_matrix(const char *identifier, const char *data,
        unsigned int num_features, unsigned int num_feature_frames);
#endif

#endif /*__CY_LPWWD_PREWWD_H */

/* [] END OF FILE */
