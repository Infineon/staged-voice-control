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
 * @file cy_staged_voice_control.h
 *
 * @brief This file is the header file for Staged Voice Control (SVC) library.
 * Depends on the CORE selected, the staged voice control module operations
 * would change. Hence appropriate internal header files would be selected
 * based on the CORE selected.
 *
 * For Example:
 *
 * 1) For Low Power device (M33), the internal header file
 * cy_staged_voice_control_lp.h would be selected. The application developer
 * need to use the API interfaces defined under cy_staged_voice_control_lp.h to
 * develop application required for low power core.
 *
 * 2) For High Performance device (M55), the internal header file
 * cy_staged_voice_control_hp.h would be selected. The application developer
 * need to use the API interfaces defined under cy_staged_voice_control_hp.h
 * to develop application required for high performance core.
 *
 */

#ifndef __CY_STAGED_VOICE_CONTROL_H__
#define __CY_STAGED_VOICE_CONTROL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(ENABLE_SVC_LP_MW)
/**
 * Low Power device (M33) is selected, So including the corresponding header
 * file for M33
 */
#include "cy_staged_voice_control_lp.h"
#endif

#if defined(ENABLE_SVC_HP_MW)
/**
 * High Performance device (M55) is selected, So including the corresponding
 * header file for M55
 */
#include "cy_staged_voice_control_hp.h"
#else
/**
 * Invalid core has been selected. Throwing compile time error.
 */
//#error   "Invalid Core selected"
#endif


#ifdef __cplusplus
}
#endif

#endif /* __CY_STAGED_VOICE_CONTROL_H__ */
