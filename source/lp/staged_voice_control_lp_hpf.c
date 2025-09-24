/*
 * Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/**
 * @file staged_voice_control_lp_hpf.c
 *
 */
#ifdef ENABLE_SVC_LP_MW

#include "staged_voice_control_lp_hpf.h"
#ifndef ENABLE_HPF_STUB
#include "ifx_sp_utils.h"
#include "ifx_pre_post_process.h"


#ifdef COMPONENT_HEX_DUMP
#include "cy_hex_dump.h"
#endif

/*
 * This is manually generated configuration
 */
static int32_t pre_proc_hpf_config_prms[] = {
  0,     /* manunally generated configuration file set configuration version to zero */
  16000, /* sampling rate */
  160,   /* input frmae size */
  IFX_PRE_PROCESS_IP_COMPONENT_HPF,     /* IP_compnent_id: preprocess HPF */
  0,     /* number of parameters */
};
#endif

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
#ifndef DISABLE_TEMP_HPF_COPY
/**
 * The High-Performance Filter (HPF) operates on a sample-by-sample basis,
 * processing each individual sample and writing the result to the output.
 * Temporary buffer in BSS is used intentionally, to achieve better performance.
 */
static uint8_t hpf_temp_out[640] = {0};
#endif
/*******************************************************************************
 *                              Function Declarations
 ******************************************************************************/

cy_rslt_t svc_lp_hpf_init(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
#ifndef ENABLE_HPF_STUB
    ifx_stc_pre_post_process_info_t *pre_proc_hpf_info = NULL;
    uint32_t ErrIdx = 0;
#endif
    if (NULL == lp_instance)
    {
        goto CLEAN_RETURN;
    }

#ifndef ENABLE_HPF_STUB

    if(svc_alloc_memory)
    {
    	svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
    			(uint32_t)sizeof(ifx_stc_pre_post_process_info_t),
				(void **)&pre_proc_hpf_info);

    	if(NULL != pre_proc_hpf_info)
    	{
    		memset(pre_proc_hpf_info,0,sizeof(ifx_stc_pre_post_process_info_t));
    	}
    }
    else
    {
    	pre_proc_hpf_info = (ifx_stc_pre_post_process_info_t*)calloc(1,sizeof(ifx_stc_pre_post_process_info_t));
    }
    if(NULL == pre_proc_hpf_info)
    {
        ret_val = CY_RSLT_SVC_OUT_OF_MEMORY;
        cy_svc_log_err(ret_val,"HPF init failed - out of mem");
        goto CLEAN_RETURN;
    }

    /* Step 1: Parse and get required memory for pre-process HPF configuration */
    ErrIdx = ifx_pre_post_process_parse(pre_proc_hpf_config_prms, pre_proc_hpf_info);
    if (ErrIdx != 0) /*Model Parsing successful*/
    {
        cy_svc_log_err(ret_val,"HPF Model Parsing failed");
        goto CLEAN_RETURN;
    }


    if(svc_alloc_memory)
    {
    	svc_alloc_memory(CY_SVC_MEM_ID_ALGO_PERSISTENT_MEMORY,
    			(uint32_t)pre_proc_hpf_info->memory.persistent_mem,
				(void **)&pre_proc_hpf_info->memory.persistent_mem_pt);

    	if(NULL != pre_proc_hpf_info->memory.persistent_mem_pt)
    	{
    		memset(pre_proc_hpf_info->memory.persistent_mem_pt,0,pre_proc_hpf_info->memory.persistent_mem);
    	}
    }
    else
    {
    /* Step 2: Allocate memory */
    	pre_proc_hpf_info->memory.persistent_mem_pt = (char *)malloc(pre_proc_hpf_info->memory.persistent_mem);
    }
    if (pre_proc_hpf_info->memory.persistent_mem_pt == NULL)
    {
        ret_val = CY_RSLT_SVC_OUT_OF_MEMORY;
        cy_svc_log_err(ret_val,"Error! Allocate persistent memory failed, exiting pre-process HPF!");
        goto CLEAN_RETURN;
    }

    if(svc_alloc_memory)
    {
    	svc_alloc_memory(CY_SVC_MEM_ID_ALGO_SCRATCH_MEMORY,
    			(uint32_t)pre_proc_hpf_info->memory.scratch_mem,
				(void **)&pre_proc_hpf_info->memory.scratch_mem_pt);

    	if(NULL != pre_proc_hpf_info->memory.scratch_mem_pt)
    	{
    		memset(pre_proc_hpf_info->memory.scratch_mem_pt,0,pre_proc_hpf_info->memory.scratch_mem);
    	}
    }
    else
    {
    	pre_proc_hpf_info->memory.scratch_mem_pt = (char*)malloc(pre_proc_hpf_info->memory.scratch_mem);
    }
    if (pre_proc_hpf_info->memory.scratch_mem_pt == NULL)
    {
        ret_val = CY_RSLT_SVC_OUT_OF_MEMORY;
        cy_svc_log_err(ret_val,"Error! Allocate scratch memory failed, exiting pre-process HPF!");
        goto CLEAN_RETURN;
    }

    /* Step 3: Initialize pre-process HPF and get pre-process HPF Container/object */
    ErrIdx = ifx_pre_post_process_init(pre_proc_hpf_config_prms, &lp_instance->hpf_handle, pre_proc_hpf_info);
    if (ErrIdx)
    {
        cy_svc_log_err(ret_val,"Error! Pre-process HPF initialization failed! Error code=%x, Component index=%d, Line number=%d, exiting pre-process HPF!",
                   IFX_SP_ENH_ERR_CODE(ErrIdx), IFX_SP_ENH_ERR_COMPONENT_INDEX(ErrIdx), IFX_SP_ENH_ERR_LINE_NUMBER(ErrIdx));
        goto CLEAN_RETURN;
    }


    lp_instance->hpf_info = pre_proc_hpf_info;
    cy_svc_log_dbg("HPF init success");
#else
    cy_svc_log_dbg("HPF stub init success");
#endif

    ret_val = CY_RSLT_SUCCESS;

    CLEAN_RETURN:
        if(CY_RSLT_SUCCESS != ret_val)
        {
#ifndef ENABLE_HPF_STUB
			if(pre_proc_hpf_info)
			{
	            if( pre_proc_hpf_info->memory.scratch_mem_pt)
	            {
	            	if(svc_free_memory)
	            	{
	            		svc_free_memory(CY_SVC_MEM_ID_ALGO_SCRATCH_MEMORY,
	            				pre_proc_hpf_info->memory.scratch_mem_pt);
	            	}
	            	else
	            	{
	            		free(pre_proc_hpf_info->memory.scratch_mem_pt);
	            	}
	            }

	            if( pre_proc_hpf_info->memory.persistent_mem_pt)
	            {
	            	if(svc_free_memory)
	            	{
	            		svc_free_memory(CY_SVC_MEM_ID_ALGO_PERSISTENT_MEMORY,
	            		        pre_proc_hpf_info->memory.persistent_mem_pt);
	            	}
	            	else
	            	{
	            		free(pre_proc_hpf_info->memory.persistent_mem_pt);
	            	}
	            }

	            if(svc_free_memory)
	            {
	        		svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
	        				pre_proc_hpf_info);
	            }
	            else
	            {
	            	free(pre_proc_hpf_info);
	            }
	        }
#endif
        }
        return ret_val;
}

cy_rslt_t svc_lp_hpf_deinit(svc_lp_instance_t *lp_instance)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    if (NULL == lp_instance)
    {
        goto CLEAN_RETURN;
    }

#ifndef ENABLE_HPF_STUB

    if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_HPF)
    {
        ifx_stc_pre_post_process_info_t *pre_proc_hpf_info = lp_instance->hpf_info;
        if(pre_proc_hpf_info)
        {
            if( pre_proc_hpf_info->memory.scratch_mem_pt)
            {
            	if(svc_free_memory)
            	{
            		svc_free_memory(CY_SVC_MEM_ID_ALGO_SCRATCH_MEMORY,
            				pre_proc_hpf_info->memory.scratch_mem_pt);
            	}
            	else
            	{
            		free(pre_proc_hpf_info->memory.scratch_mem_pt);
            	}
            }

            if( pre_proc_hpf_info->memory.persistent_mem_pt)
            {
            	if(svc_free_memory)
            	{
            		svc_free_memory(CY_SVC_MEM_ID_ALGO_PERSISTENT_MEMORY,
            		        pre_proc_hpf_info->memory.persistent_mem_pt);
            	}
            	else
            	{
            		free(pre_proc_hpf_info->memory.persistent_mem_pt);
            	}
            }

            if(svc_free_memory)
            {
        		svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
        				pre_proc_hpf_info);
            }
            else
            {
            	free(pre_proc_hpf_info);
            }
            pre_proc_hpf_info = NULL;
        }

        lp_instance->hpf_handle = NULL;
    }

#else
    cy_svc_log_dbg("HPF stud deinit success");
#endif

    ret_val = CY_RSLT_SUCCESS;
    CLEAN_RETURN: return ret_val;
}

cy_rslt_t svc_lp_hpf_process(
        svc_lp_instance_t *lp_instance,
        uint8_t *input,
        uint8_t *output)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    unsigned int size_to_process = 0;
    IFX_SP_DATA_TYPE_T* hpf_ouput1 = NULL;
    IFX_SP_DATA_TYPE_T* hpf_ouput2 = NULL;
#ifndef ENABLE_HPF_STUB
    uint32_t ErrIdx = 0;
#endif

    if ((NULL == lp_instance) || (NULL == input) || (NULL == output))
    {
        goto CLEAN_RETURN;
    }

#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif

    if (CY_SVC_AUDIO_INPUT_TYPE_STEREO
            == lp_instance->init_params.audio_input_type)
    {
        size_to_process =
                lp_instance->circular_shared_buffer->frame_size_in_bytes
                        / 2;
#ifndef ENABLE_HPF_STUB
        if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_HPF)
        {
            /**
             *  Perform HPF
             */
#ifndef DISABLE_TEMP_HPF_COPY
            hpf_ouput1 = (IFX_SP_DATA_TYPE_T*)hpf_temp_out;
            hpf_ouput2 = (IFX_SP_DATA_TYPE_T*)(hpf_temp_out + size_to_process);
#else
            hpf_ouput1 = (IFX_SP_DATA_TYPE_T*)output;
            hpf_ouput2 = (IFX_SP_DATA_TYPE_T*)(output + size_to_process);
#endif
            ErrIdx = ifx_time_pre_process((IFX_SP_DATA_TYPE_T*)input, (IFX_SP_DATA_TYPE_T*)(input + size_to_process),
                                            lp_instance->hpf_handle, IFX_PRE_PROCESS_IP_COMPONENT_HPF,
                                            hpf_ouput1, hpf_ouput2);
            if (ErrIdx != IFX_SP_ENH_SUCCESS)
            {
                cy_svc_log_err(ret_val,"Pre-process HPF channel 2 error: Error code=%x, Component index=%d, Line number=%d",
                                IFX_SP_ENH_ERR_CODE(ErrIdx), IFX_SP_ENH_ERR_COMPONENT_INDEX(ErrIdx), IFX_SP_ENH_ERR_LINE_NUMBER(ErrIdx));
                goto CLEAN_RETURN;
            }
#ifndef DISABLE_TEMP_HPF_COPY
            memcpy(output, hpf_ouput1, lp_instance->circular_shared_buffer->frame_size_in_bytes);
#endif
        }
        else
#endif
        {
            memcpy(output, input, size_to_process);
            memcpy(output + size_to_process, input + size_to_process,
                    size_to_process);
        }
    }
    else
    {
        /**
         * Assumed to be Mono type
         */
        size_to_process =
                lp_instance->circular_shared_buffer->frame_size_in_bytes;


#ifndef ENABLE_HPF_STUB
        if (lp_instance->init_params.stage_config_list & CY_SVC_ENABLE_HPF)
        {
            /**
             *  Perform HPF
             */
#ifndef DISABLE_TEMP_HPF_COPY
            hpf_ouput1 = (IFX_SP_DATA_TYPE_T*)hpf_temp_out;
#else
            hpf_ouput1 = (IFX_SP_DATA_TYPE_T*)output;
#endif
            ErrIdx = ifx_time_pre_process((IFX_SP_DATA_TYPE_T*)input, NULL, lp_instance->hpf_handle,
                                            IFX_PRE_PROCESS_IP_COMPONENT_HPF, hpf_ouput1, NULL);
#ifndef DISABLE_TEMP_HPF_COPY
            memcpy(output, hpf_ouput1, lp_instance->circular_shared_buffer->frame_size_in_bytes);
#endif
            if (ErrIdx != IFX_SP_ENH_SUCCESS)
            {
                cy_svc_log_err(ret_val,"Pre-process HPF channel 2 error: Error code=%x, Component index=%d, Line number=%d",
                                IFX_SP_ENH_ERR_CODE(ErrIdx), IFX_SP_ENH_ERR_COMPONENT_INDEX(ErrIdx), IFX_SP_ENH_ERR_LINE_NUMBER(ErrIdx));
                goto CLEAN_RETURN;
            }
#ifdef COMPONENT_HEX_DUMP
//			cy_hex_dump(HEX_DUMP_ID_3 ,output, size_to_process);
#endif
        }
        else
#endif
        {
            /**
             *  Perform simulated (memcopy) operation for HPF
             */
            memcpy(output, input, size_to_process);
        }
    }

    ret_val = CY_RSLT_SUCCESS;

    CLEAN_RETURN:
#ifdef ENABLE_SVC_LP_CHECK_POINT_APP_API
    SVC_LP_CHECK_POINT_APP_API();
#endif
    return ret_val;
}
#endif
