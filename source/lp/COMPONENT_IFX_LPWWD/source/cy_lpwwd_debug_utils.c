/******************************************************************************
 * File Name: cy_lpwwd_prewwd.c
 *
 * Description: Implements functions for pre wake-word detection.
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
#include "cy_lpwwd_private.h"
#include "cy_lpwwd_prewwd.h"
#include "cy_lpwwd.h"
#include "cy_lpwwd_error.h"
#include "cy_fe.h"
#include "cy_lpwwd_debug_utils.h"


/******************************************************
 *                      Typedefs
 ******************************************************/

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                      Variables
 ******************************************************/

/******************************************************
 *                      Function Prototypes
 ******************************************************/

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
void print_output_score(cy_lpwwd_context_t *lpwwd_context)
{
    cy_lpwwd_log_info("SCORE:0x%04X 0x%04X 0x%04X 0x%04X",
        lpwwd_context->output_score_fixed[0],
        lpwwd_context->output_score_fixed[1],
        lpwwd_context->output_score_fixed[2],
        lpwwd_context->output_score_fixed[3]);
}
#endif


#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
void print_output_score_reduced(int16_t *output_score_fixed)
{
    cy_lpwwd_log_info("SCORE-R:0x%04X 0x%04X 0x%04X 0x%04X",
        output_score_fixed[0],
        output_score_fixed[1],
        output_score_fixed[2],
        output_score_fixed[3]);
}
#endif




#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_FRAME_PRINT)
void cy_lpwwd_print_feature_frame( cy_lpwwd_context_t *lpwwd_context)
{
#ifdef RUN_FLOAT_FE
    printf("FF:");
    for (int i = 0; i < lpwwd_context->number_of_features; i++) {
        printf("%e, ",
                lpwwd_context->feature_buffer.feature_frame_buffer[i]);
    }
    printf("\n");
#else
	PRINT_ARR_AS_CHAR("FF:",lpwwd_context->feature_buffer.feature_frame_buffer);
#endif
	return;
}
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_BUF_MATRIX_PRINT)
void printf_feature_buffer_matrix(const char *identifier, const char *data, unsigned int num_features, unsigned int num_feature_frames)
{
    const unsigned char *temp = (const unsigned char *)data;
    if(NULL != data)
    {
        cy_lpwwd_log_info("%s",identifier);
        for (unsigned int i = 0; i < num_features; i++)
        {
            uint32_t offset = i * num_feature_frames;
            cy_lpwwd_log_info("[0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0X%02X 0x%02X 0x%02X 0x%02X 0X%02X 0x%02X 0x%02X]",
            temp[offset + 0],
            temp[offset + 1],
            temp[offset + 2],
            temp[offset + 3],
            temp[offset + 4],
            temp[offset + 5],
            temp[offset + 6],
            temp[offset + 7],
            temp[offset + 8],
            temp[offset + 9],
            temp[offset + 10],
            temp[offset + 11],
            temp[offset + 12],
            temp[offset + 13],
            temp[offset + 14],
            temp[offset + 15],
            temp[offset + 16] );
        }
    }
    return;
}
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_ML_MODEL_PRINT)
void print_ml_model(const char *model)
{
    const unsigned char *temp =  (const unsigned char *) model;
    if(temp)
    {
        cy_lpwwd_log_info("ModelBinaryAddress:%p",temp);
        for (unsigned int i = 0; i < 10; i++)
        {
            cy_lpwwd_log_info("[0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0X%02x, 0x%02x, 0x%02x, 0x%02x, 0X%02x, 0x%02x]",
                temp[0],
                temp[1],
                temp[2],
                temp[3],
                temp[4],
                temp[5],
                temp[6],
                temp[7],
                temp[8],
                temp[9],
                temp[10],
                temp[11],
                temp[12],
                temp[13],
                temp[14],
                temp[15] );
            temp = temp + 16;
        }
    }
    return;
}
#endif

/* [] END OF FILE */
