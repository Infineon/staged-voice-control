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

/**
 * @file staged_voice_control_lp_log_utils.c
 *
 */

#ifdef ENABLE_SVC_LP_MW
#include "staged_voice_control_lp_resource.h"
#include "staged_voice_control_lp_private.h"
#include "staged_voice_control_lp_stats.h"

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
svc_lp_check_points svc_lp_check_point = {0};

/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

unsigned int svc_lp_stats_get_feed_counter(void)
{
	svc_lp_instance_t *lp_instance = svc_lp_get_instance();
	return(lp_instance->stats.frame_counter_received_after_last_aad_dbg);
}


cy_rslt_t svc_lp_stats_update_and_and_print(
        svc_lp_instance_t *lp_instance,
        svc_stage_trigger_t trigger)
{
    bool print_stats = false;

    switch (trigger)
    {
        case SVC_TRIGGER_ACOUSTIC_ACTIVITY_FORCEFULLY:
        {
            print_stats = true;
            break;
        }
        case SVC_TRIGGER_PRINT_STATISTICS:
        {
            print_stats = true;
            break;
        }
        case SVC_TRIGGER_SPEECH_ONSET_DETECTED:
        {
            lp_instance->stats.sod_detect_counter_dbg++;

            if (lp_instance->stats.sod_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_SOD_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_DETECTED:
        {
            lp_instance->stats.lpwwd_detect_counter_dbg++;

            if (lp_instance->stats.lpwwd_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_LPWWD_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_LOW_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            lp_instance->stats.lpwwd_not_detect_counter_dbg++;

            if (lp_instance->stats.lpwwd_not_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_LPWWD_NOT_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_HIGH_POWER_WAKEUP_WORD_DETECTED:
        {
            lp_instance->stats.hpwwd_detect_counter_dbg++;
            if (lp_instance->stats.hpwwd_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_HPWWD_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_HIGH_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            lp_instance->stats.hpwwd_not_detect_counter_dbg++;
            if (lp_instance->stats.hpwwd_not_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_HPWWD_NOT_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_ASR_DETECTED:
        {
            lp_instance->stats.asr_detect_counter_dbg++;
            if (lp_instance->stats.asr_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_ASR_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_ASR_NOT_DETECTED:
        {
            lp_instance->stats.asr_not_detect_counter_dbg++;
            if (lp_instance->stats.asr_not_detect_counter_dbg
                    % PRINT_STATS_ON_EVERY_ASR_NOT_DETECT_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        case SVC_TRIGGER_ASR_PROCESSING_COMPLETED:
        {
            lp_instance->stats.asr_process_completed_counter_dbg++;
            if (lp_instance->stats.asr_process_completed_counter_dbg
                    % PRINT_STATS_ON_EVERY_ASR_PROCESS_COMPLETED_COUNTER == 0)
            {
                print_stats = true;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    if (true == print_stats)
    {
        cy_svc_log_info(
                "STATS:Feed(%d:%d:%d:%d)SOD(%d)LPW(%d:%d:%d)HPW(%d,%d:%d)ASR(%d:%d:%d)",
                lp_instance->stats.frame_counter_received_after_last_aad_dbg,
                lp_instance->stats.svc_lp_feed_post_fail_counter_dbg,
                lp_instance->stats.crc_check_fail_counter_dbg,
                (unsigned int)lp_instance->stats.last_frame_feed_time_ms,
                lp_instance->stats.sod_detect_counter_dbg,
                lp_instance->stats.lpwwd_feed_counter_dbg,
                lp_instance->stats.lpwwd_detect_counter_dbg,
                lp_instance->stats.lpwwd_not_detect_counter_dbg,
                lp_instance->stats.hpwwd_feed_counter_dbg,
                lp_instance->stats.hpwwd_detect_counter_dbg,
                lp_instance->stats.hpwwd_not_detect_counter_dbg,
                lp_instance->stats.asr_detect_counter_dbg,
                lp_instance->stats.asr_process_completed_counter_dbg,
                lp_instance->stats.asr_not_detect_counter_dbg)
    }

    return CY_RSLT_SUCCESS;
}

#endif
