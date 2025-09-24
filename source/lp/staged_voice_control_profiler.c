/******************************************************************************
* File Name: staged_voice_control_profiler.c
*
* Description: This file contains functions for profiler code of SVC

*******************************************************************************/

/*******************************************************************************
* Include header file
******************************************************************************/

#ifdef COMPONENT_PROFILER
#include "staged_voice_control_profile.h"
#include "cy_profiler.h"
#include <inttypes.h>
#include <stdio.h>

/******************************************************************************
* Defines
*****************************************************************************/

/******************************************************************************
* Constants
*****************************************************************************/

/******************************************************************************
* Variables
*****************************************************************************/

/******************************************************************************
* Functions
*****************************************************************************/

bool svc_profile_on = false;
bool svc_hpf_profile_on = false;
svc_profile_data_t svc_profile = {0};

/* Enable only HPF or overall SVC profiling. Dont enable both otherwise result may not be
 * appropriate
 */
cy_rslt_t cy_svc_profile(svc_profile_command cmd,
        svc_profile_data_t *data)
{
    switch(cmd)
    {
        case SVC_PROCESS_PROFILE_CMD_ENABLE:
        {
            svc_profile.svc_cycles_taken = 0;
            svc_profile.svc_profile_frame_counter = 0;
            svc_profile_on = true;
            break;
        }
        case SVC_HPF_PROFILE_CMD_ENABLE:
        {
            svc_profile.hpf_cycles_taken = 0;
            svc_profile.hpf_profile_frame_counter = 0;
            svc_hpf_profile_on = true;
            break;
        }
        case SVC_PROFILE_CMD_DISABLE:
        {
            svc_profile_on = false;
            break;
        }
        case SVC_HPF_PROFILE_CMD_DISABLE:
        {
            svc_hpf_profile_on = false;
            break;
        }
        case SVC_HPF_PROFILE_CMD_START:
        {
            if(true == svc_hpf_profile_on)
            {
                svc_profile.hpf_profile_frame_counter++;
                cy_profiler_start();
            }
            break;
        }
        case SVC_PROFILE_CMD_START:
        {
            if(true == svc_profile_on)
            {
                svc_profile.svc_profile_frame_counter++;
                cy_profiler_start();
            }
            break;
        }
        case SVC_HPF_PROFILE_CMD_STOP:
        {
            if(true == svc_hpf_profile_on)
            {
                uint32_t cycles = 0;
                cy_profiler_stop();
                cycles = cy_profiler_get_cycles();
                svc_profile.hpf_cycles_taken += cycles;
            }
            break;
        }
        case SVC_PROFILE_CMD_STOP:
        {
            if(true == svc_profile_on)
            {
                uint32_t cycles = 0;
                cy_profiler_stop();
                cycles = cy_profiler_get_cycles();
                svc_profile.svc_cycles_taken += cycles;
            }
            break;
        }
        case SVC_PROFILE_CMD_GET_DATA:
        {
            if(NULL != data)
            {
                data->hpf_cycles_taken = svc_profile.hpf_cycles_taken;
                data->svc_cycles_taken = svc_profile.svc_cycles_taken;
                data->hpf_profile_frame_counter = svc_profile.hpf_profile_frame_counter;
                data->svc_profile_frame_counter = svc_profile.svc_profile_frame_counter;
            }
            break;
        }
        case SVC_PROFILE_CMD_RESET:
        {
            if(true == svc_profile_on)
            {
                svc_profile.svc_cycles_taken = 0;
                svc_profile.svc_profile_frame_counter = 0;
            }
            break;
        }
        case SVC_HPF_PROFILE_CMD_RESET:
        {
            if(true == svc_hpf_profile_on)
            {
                svc_profile.hpf_cycles_taken = 0;
                svc_profile.hpf_profile_frame_counter = 0;
            }
            break;
        }
        case SVC_PROFILE_CMD_PRINT_STATS:
        {
#ifdef ENABLE_SVC_PROFILER_PRINT
             printf("\r\nHPF-Profile:Cntr:%" PRIu32 ", Cycles:%" PRIu32 ", SVC-Profile:Cntr:%" PRIu32 ", Cycles:%f \r\n",
                     svc_profile.hpf_profile_frame_counter,
                     svc_profile.hpf_cycles_taken,
                     svc_profile.svc_profile_frame_counter,
                    (double)svc_profile.svc_cycles_taken);
#endif
             break;
        }
        default:
        {
            break;
        }
    }

    return CY_RSLT_SUCCESS;
}

#endif
