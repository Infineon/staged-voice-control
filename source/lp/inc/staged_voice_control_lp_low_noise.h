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

#ifndef __STAGED_VOICE_CONTROL_LP_LOW_NOISE_H__
#define __STAGED_VOICE_CONTROL_LP_LOW_NOISE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_LP_MW
/*******************************************************************************
 *                              Includes
 ******************************************************************************/
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_sod.h"
#include "cy_lpwwd_defines.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/
#ifndef CY_SVC_MIN_SUPPORTED_TIMEOUT_MS
#define CY_SVC_MIN_SUPPORTED_TIMEOUT_MS    (2000) /* 2sec */
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

/*******************************************************************************
 *                              Global Variables
 ******************************************************************************/

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

/**
 * @brief Detects low noise conditions in a mono audio frame
 *
 * @param mono_audio_frame Pointer to the mono audio frame data buffer containing 16-bit signed audio samples
 *
 * @return true if low noise is detected in the audio frame, false otherwise
 *
 * @note This function analyzes the provided audio frame to determine if the noise level
 *       is below a certain threshold, which may be used for voice activity detection
 *       or audio processing optimization in staged voice control systems.
 */

bool svc_lp_low_noise_detected(int16_t* mono_audio_frame);

/**
 * @brief Configure low noise parameters for staged voice control low power mode
 *
 * This function configures the low noise detection feature for the staged voice control
 * system operating in low power mode. It sets the timeout period, noise threshold level,
 * and enables or disables the low noise detection feature.
 *
 * @param[in] timeout_ms        Timeout period in milliseconds for low noise detection
 * @param[in] low_noise_threshold  Threshold level for determining low noise conditions
 * @param[in] enable_feature    Flag to enable (true) or disable (false) the low noise feature
 *
 * @return cy_rslt_t           Result code indicating success or failure
 * @retval CY_RSLT_SUCCESS     Configuration completed successfully
 * @retval CY_RSLT_TYPE_ERROR  Invalid parameter provided
 *
 * @note This function should be called before enabling the staged voice control system
 * @warning Ensure timeout_ms is within valid range to prevent system instability
 */
cy_rslt_t svc_lp_low_noise_config(uint32_t timeout_ms, uint32_t low_noise_threshold, bool enable_feature);

#endif /* ENABLE_SVC_LP_MW */

#ifdef __cplusplus
}
#endif

#endif // __STAGED_VOICE_CONTROL_LP_LOW_NOISE_H__
