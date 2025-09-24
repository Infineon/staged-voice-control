/******************************************************************************
* File Name: cy_lpwwd.h
*
* Description: APIs & data structures for Low power wake word detection.
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

/*******************************************************************************
* Include header file
******************************************************************************/
#ifdef COMPONENT_PROFILER
#ifndef __CY_LPWWD_PROFILER_H__
#define __CY_LPWWD_PROFILER_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	LPWWD_PROFILE_CMD_INVALID,

	LPWWD_PROFILE_CMD_ENABLE_OVERALL,
	LPWWD_PROFILE_CMD_DISABLE_OVERALL,
	LPWWD_PROFILE_CMD_START_OVERALL,
	LPWWD_PROFILE_CMD_STOP_OVERALL,
	LPWWD_PROFILE_CMD_RESET_OVERALL,

	LPWWD_PROFILE_CMD_ENABLE_FE,
	LPWWD_PROFILE_CMD_DISABLE_FE,
	LPWWD_PROFILE_CMD_START_FE,
	LPWWD_PROFILE_CMD_STOP_FE,
	LPWWD_PROFILE_CMD_RESET_FE,

	LPWWD_PROFILE_CMD_ENABLE_ML,
	LPWWD_PROFILE_CMD_DISABLE_ML,
	LPWWD_PROFILE_CMD_START_ML,
	LPWWD_PROFILE_CMD_STOP_ML,
	LPWWD_PROFILE_CMD_RESET_ML,

	LPWWD_PROFILE_CMD_ENABLE_HMMS,
	LPWWD_PROFILE_CMD_DISABLE_HMMS,
	LPWWD_PROFILE_CMD_START_HMMS,
	LPWWD_PROFILE_CMD_STOP_HMMS,
	LPWWD_PROFILE_CMD_RESET_HMMS,

	LPWWD_PROFILE_CMD_GET_DATA,
	LPWWD_PROFILE_CMD_PRINT_STATS,

	LPWWD_PROFILE_CMD_MAX

} lpwwd_profile_command;

typedef struct
{
	unsigned int cycles_taken_overall;
    unsigned int profile_frame_counter_overall;
	unsigned int cycles_taken_fe;
    unsigned int profile_frame_counter_fe;
	unsigned int cycles_taken_ml;
    unsigned int profile_frame_counter_ml;
	unsigned int cycles_taken_hmms;
    unsigned int profile_frame_counter_hmms;

} lpwwd_profile_data_t;


cy_rslt_t cy_lpwwd_profile(lpwwd_profile_command cmd,
		lpwwd_profile_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __CY_LPWWD_PROFILER_H__ */
#endif
