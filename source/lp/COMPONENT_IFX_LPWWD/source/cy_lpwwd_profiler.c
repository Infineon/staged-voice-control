/******************************************************************************
 * File Name: cy_lpwwd.c
 *
 * Description: Implementation of Low power wake word APIs
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
#include "cy_lpwwd_private.h"
#include "cy_lpwwd_profiler.h"
#include "cy_profiler.h"

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
bool lpwwd_profile_on_overall;
bool lpwwd_profile_on_fe;
bool lpwwd_profile_on_ml;
bool lpwwd_profile_on_hmms;

lpwwd_profile_data_t lpwwd_profile = {0};

cy_rslt_t cy_lpwwd_profile(lpwwd_profile_command cmd,
		lpwwd_profile_data_t *data)
{
	switch(cmd)
	{
		/* Overall profile */
		case LPWWD_PROFILE_CMD_ENABLE_OVERALL:
		{
			lpwwd_profile.cycles_taken_overall = 0;
			lpwwd_profile.profile_frame_counter_overall = 0;
			lpwwd_profile_on_overall = true;
			break;
		}
		case LPWWD_PROFILE_CMD_DISABLE_OVERALL:
		{
			lpwwd_profile_on_overall = false;
			break;
		}
		case LPWWD_PROFILE_CMD_START_OVERALL:
		{
			if(true == lpwwd_profile_on_overall )
			{
				lpwwd_profile.profile_frame_counter_overall++;
				cy_profiler_start();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_STOP_OVERALL:
		{
			if(true == lpwwd_profile_on_overall )
			{
				cy_profiler_stop();
				lpwwd_profile.cycles_taken_overall += cy_profiler_get_cycles();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_RESET_OVERALL:
		{
			if(true == lpwwd_profile_on_overall )
			{
				lpwwd_profile.cycles_taken_overall = 0;
				lpwwd_profile.profile_frame_counter_overall = 0;
			}
			break;
		}


		/* Feature Extraction profile */
		case LPWWD_PROFILE_CMD_ENABLE_FE:
		{
			lpwwd_profile.cycles_taken_fe = 0;
			lpwwd_profile.profile_frame_counter_fe = 0;
			lpwwd_profile_on_fe = true;
			break;
		}
		case LPWWD_PROFILE_CMD_DISABLE_FE:
		{
			lpwwd_profile_on_fe = false;
			break;
		}
		case LPWWD_PROFILE_CMD_START_FE:
		{
			if(true == lpwwd_profile_on_fe )
			{
				lpwwd_profile.profile_frame_counter_fe++;
				cy_profiler_start();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_STOP_FE:
		{
			if(true == lpwwd_profile_on_fe )
			{
				cy_profiler_stop();
				lpwwd_profile.cycles_taken_fe += cy_profiler_get_cycles();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_RESET_FE:
		{
			if(true == lpwwd_profile_on_fe )
			{
				lpwwd_profile.cycles_taken_fe = 0;
				lpwwd_profile.profile_frame_counter_fe = 0;
			}
			break;
		}

		/* ML process profile */
		case LPWWD_PROFILE_CMD_ENABLE_ML:
		{
			lpwwd_profile.cycles_taken_ml = 0;
			lpwwd_profile.profile_frame_counter_ml = 0;
			lpwwd_profile_on_ml = true;
			break;
		}
		case LPWWD_PROFILE_CMD_DISABLE_ML:
		{
			lpwwd_profile_on_ml = false;
			break;
		}
		case LPWWD_PROFILE_CMD_START_ML:
		{
			if(true == lpwwd_profile_on_ml )
			{
				lpwwd_profile.profile_frame_counter_ml++;
				cy_profiler_start();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_STOP_ML:
		{
			if(true == lpwwd_profile_on_ml )
			{
				cy_profiler_stop();
				lpwwd_profile.cycles_taken_ml += cy_profiler_get_cycles();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_RESET_ML:
		{
			if(true == lpwwd_profile_on_ml )
			{
				lpwwd_profile.cycles_taken_ml = 0;
				lpwwd_profile.profile_frame_counter_ml = 0;
			}
			break;
		}

		/* HMMS process profile */
		case LPWWD_PROFILE_CMD_ENABLE_HMMS:
		{
			lpwwd_profile.cycles_taken_hmms = 0;
			lpwwd_profile.profile_frame_counter_hmms = 0;
			lpwwd_profile_on_hmms = true;
			break;
		}
		case LPWWD_PROFILE_CMD_DISABLE_HMMS:
		{
			lpwwd_profile_on_hmms = false;
			break;
		}
		case LPWWD_PROFILE_CMD_START_HMMS:
		{
			if(true == lpwwd_profile_on_hmms )
			{
				lpwwd_profile.profile_frame_counter_hmms++;
				cy_profiler_start();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_STOP_HMMS:
		{
			if(true == lpwwd_profile_on_hmms )
			{
				cy_profiler_stop();
				lpwwd_profile.cycles_taken_hmms += cy_profiler_get_cycles();
			}
			break;
		}
		case LPWWD_PROFILE_CMD_RESET_HMMS:
		{
			if(true == lpwwd_profile_on_hmms )
			{
				lpwwd_profile.cycles_taken_hmms = 0;
				lpwwd_profile.profile_frame_counter_hmms = 0;
			}
			break;
		}


		case LPWWD_PROFILE_CMD_GET_DATA:
		{
			if(true == lpwwd_profile_on_overall)
			{
				if(NULL != data)
				{
					memcpy(data,&lpwwd_profile,sizeof(lpwwd_profile));
				}
			}
			break;
		}
		case LPWWD_PROFILE_CMD_PRINT_STATS:
		{
			cy_lpwwd_log_info("LPWWD-Profile Cycles:[Overall:%u:%u],[FE:%u:%u],[ML:%u:%u],[HMMS:%u:%u]",
					 lpwwd_profile.profile_frame_counter_overall,
					 lpwwd_profile.cycles_taken_overall,
					 lpwwd_profile.profile_frame_counter_fe,
					 lpwwd_profile.cycles_taken_fe,
					 lpwwd_profile.profile_frame_counter_ml,
					 lpwwd_profile.cycles_taken_ml,
					 lpwwd_profile.profile_frame_counter_hmms,
					 lpwwd_profile.cycles_taken_hmms);

			 printf("\nLPWWD-Profile Cycles:[Overall:%u:%u],[FE:%u:%u],[ML:%u:%u],[HMMS:%u:%u]\n",
					 lpwwd_profile.profile_frame_counter_overall,
					 lpwwd_profile.cycles_taken_overall,
					 lpwwd_profile.profile_frame_counter_fe,
					 lpwwd_profile.cycles_taken_fe,
					 lpwwd_profile.profile_frame_counter_ml,
					 lpwwd_profile.cycles_taken_ml,
					 lpwwd_profile.profile_frame_counter_hmms,
					 lpwwd_profile.cycles_taken_hmms);
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
