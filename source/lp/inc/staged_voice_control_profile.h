 /******************************************************************************
* File Name: staged_voice_control_profile.h
*
* Description: Header file for SVC profilling
*
*******************************************************************************/

/*******************************************************************************
* Include guard
*******************************************************************************/
#ifndef __CY_SVC_PROFILER_H__
#define __CY_SVC_PROFILER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_result.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef COMPONENT_PROFILER
typedef enum
{
    SVC_PROFILE_CMD_INVALID,
    SVC_PROCESS_PROFILE_CMD_ENABLE,     /* Enables overall SVC profiling */
    SVC_HPF_PROFILE_CMD_ENABLE,         /* Enables only HPF profiling */
    SVC_PROFILE_CMD_DISABLE,
    SVC_HPF_PROFILE_CMD_DISABLE,
    SVC_PROFILE_CMD_START,
    SVC_HPF_PROFILE_CMD_START,
    SVC_PROFILE_CMD_STOP,
    SVC_HPF_PROFILE_CMD_STOP,
    SVC_PROFILE_CMD_GET_DATA,
    SVC_PROFILE_CMD_RESET,
    SVC_HPF_PROFILE_CMD_RESET,
    SVC_PROFILE_CMD_PRINT_STATS,
    SVC_PROFILE_CMD_MAX
} svc_profile_command;

typedef struct
{
    uint32_t hpf_cycles_taken;
    uint32_t hpf_profile_frame_counter;
    uint32_t svc_profile_frame_counter;
    uint64_t svc_cycles_taken;
} svc_profile_data_t;

cy_rslt_t cy_svc_profile(svc_profile_command cmd,
        svc_profile_data_t *data);

#endif /* COMPONENT_PROFILER */
#endif  /*__CY_SVC_PROFILER_H__ */

#ifdef __cplusplus
}
#endif

/* [] END OF FILE */
