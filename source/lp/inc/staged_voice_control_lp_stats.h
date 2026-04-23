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
 * @file staged_voice_control_lp_stats.h
 *
 */

#ifndef __CY_SVC_LP_STATS_H_
#define __CY_SVC_LP_STATS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_private.h"

/*******************************************************************************
 *                              Macros
 ******************************************************************************/

#define PRINT_STATS_ON_EVERY_SOD_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_LPWWD_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_LPWWD_NOT_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_HPWWD_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_HPWWD_NOT_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_ASR_DETECT_COUNTER (10)
#define PRINT_STATS_ON_EVERY_ASR_NOT_DETECT_COUNTER (1)
#define PRINT_STATS_ON_EVERY_ASR_PROCESS_COMPLETED_COUNTER (10)

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

cy_rslt_t svc_lp_stats_update_and_and_print(svc_lp_instance_t *lp_instance,
        svc_stage_trigger_t trigger);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __CY_SVC_LP_LOG_UTILS_H_ */
