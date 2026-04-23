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

#ifdef ENABLE_SVC_LP_MW
#include "staged_voice_control_lp_low_noise.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/
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
static uint32_t low_noise_fixed_threshold = 0; // Default threshold
static uint32_t silent_frame_timeout_counter = 0; // timeout counter
static uint32_t silent_frame_max_timeout_counter = 10000; // timeout value in milliseconds - default 10sec
static bool g_low_noise_feature_enabled = false; // AAD feature enable flag - default disabled

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/
bool svc_lp_low_noise_detected(int16_t* mono_audio_frame)
{
    bool is_low_noise_detected = false;
    bool is_silence_frame = true;

    if (mono_audio_frame == NULL)
    {
        cy_svc_log_err(CY_RSLT_SVC_BAD_ARG, "NULL audio frame");
        return is_low_noise_detected;
    }

    if (false == g_low_noise_feature_enabled)
    {
        return is_low_noise_detected;
    }

    for(uint16_t Idx = 0; Idx < MONO_FRAME_SIZE; Idx++)
    {
        /* Check if any value exceeds the AAD threshold */
        if (abs((int16_t)mono_audio_frame[Idx]) > low_noise_fixed_threshold)
        {
            is_silence_frame = false;
            break;
        }
    }

    if(true == is_silence_frame)
    {
        if(silent_frame_timeout_counter >= silent_frame_max_timeout_counter)
        {
            is_low_noise_detected = true;
            silent_frame_timeout_counter = 0;
        }
        else
        {
            silent_frame_timeout_counter++;
        }
    }
    else
    {
        silent_frame_timeout_counter = 0;
    }

    return is_low_noise_detected;
}

cy_rslt_t svc_lp_low_noise_config(uint32_t timeout_ms, uint32_t low_noise_threshold, bool enable_feature)
{
    if (timeout_ms < CY_SVC_MIN_SUPPORTED_TIMEOUT_MS)
    {
        cy_svc_log_err(CY_RSLT_SVC_BAD_ARG, "Invalid timeout_ms:%d", timeout_ms);
        return CY_RSLT_SVC_BAD_ARG;
    }

    low_noise_fixed_threshold = low_noise_threshold;
    silent_frame_max_timeout_counter = timeout_ms/CY_SVC_SUPPORTED_FRAME_TIME_MS;
    g_low_noise_feature_enabled = enable_feature;
    cy_svc_log_info("AAD Config: silent_frame_max_timeout_counter:%"PRIu32", Threshold:%"PRIu32", Enable:%s",
                    silent_frame_max_timeout_counter, low_noise_fixed_threshold, enable_feature ? "true" : "false");
    return CY_RSLT_SUCCESS;
}

#endif /* ENABLE_SVC_LP_MW */