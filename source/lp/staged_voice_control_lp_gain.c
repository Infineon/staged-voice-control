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
 * @file staged_voice_control_lp_gain.c
 *
 */
#ifdef ENABLE_SVC_LP_MW


 /*******************************************************************************
 *                              Includes
 ******************************************************************************/
#include "staged_voice_control_lp_gain.h"
#include <limits.h>
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
 *                              Global Variables
 ******************************************************************************/


cy_rslt_t svc_lp_apply_gain_with_clip(uint16_t gain_factor, int16_t* input_frame, int16_t* output_frame, uint16_t frame_length)
{
    if (input_frame == NULL || output_frame == NULL || frame_length == 0)
    {
        cy_svc_log_err(CY_RSLT_SVC_BAD_ARG, "Invalid input_frame:%p output_frame:%p frame_length:%u", input_frame, output_frame, frame_length);
        return CY_RSLT_SVC_BAD_ARG;
    }

    for (uint16_t i = 0; i < frame_length; i++)
    {
        int32_t sample = (int32_t)input_frame[i] * gain_factor;
        if (sample > INT16_MAX)
        {
            sample = INT16_MAX;
        }
        else if (sample < INT16_MIN)
        {
            sample = INT16_MIN;
        }
        output_frame[i] = (int16_t)sample;
    }

    return CY_RSLT_SUCCESS;
}
#endif
