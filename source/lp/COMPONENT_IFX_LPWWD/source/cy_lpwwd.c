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
#include "cy_lpwwd_private.h"
#include "cy_lpwwd.h"
#include "cy_lpwwd_debug_utils.h"
#include "lpwwd_post.h"
#if defined(COMPONENT_CM33)
#if defined(COMPONENT_NNLITE)
#include "cy_lpwwd_ml_kernel.h"
#endif
#endif

#ifdef COMPONENT_PROFILER
#include "cy_lpwwd_profiler.h"
#endif

#ifndef HMMS_CONFIG_MODEL
/**
* This needs to be global variable, as this variable would be directly
* accessed by the system algorithm for models if the the define HMMS_CONFIG_MODEL
* is enabled.
*/
int16_t *ppkwmodel = NULL;
int16_t *ppgmodel = NULL;
int16_t *ppnmodel = NULL;
#endif


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

#ifdef ENABLE_LPWWD_MW_WITH_SOD
static int circular_index(int index, int stride, int size)
{
    index += stride;
    if (index >= size)
    {
        index -= size;
    }
    else if (index < 0)
    {
        index += size;
    }

    return index;
}
#endif

#ifdef ENABLE_LPWWD_LOGS

static char *get_print_detection_str(cy_lpwwd_wwd_detect_status_t *wwd_status)
{
    if(NULL == wwd_status)
    {
        return "";
    }
    switch(*wwd_status)
    {
        case CY_LPWWD_WAKE_WORD_DETECTED:
        {
            return "LPWW_DETD";
        }
        case CY_LPWWD_WAKE_WORD_NOT_DETECTED:
        {
            return "LPWW_NDETD";
        }
        case CY_LPWWD_WAKE_WORD_REJECTED:
        {
            return "LPWW_REJD";
        }
        case CY_LPWWD_TIMEOUT:
        {
            return "LPWW_TOUT";
        }
        case CY_LPWWD_FAIL_REASON_NOT_KNOWN:
        {
            return "LPWW_NDRU";
        }
        case CY_LPWWD_WAKE_WORD_INVALID:
        {
            return "LPWW_INVA";
        }
        case CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS:
        {
            return "LPWW_INPR";
        }
        default:
        {
            return "LPWWD_UNKN";
        }
    }
}
#endif

extern unsigned int  svc_lp_stats_get_feed_counter(void);

static void print_wwd_detection_result(cy_lpwwd_context_t *lpwwd_context,
        cy_lpwwd_wwd_detect_status_t *wwd_status)
{
    extern int hmm_pp_detections;
    extern int hmm_pp_kwrejections;
    extern int hmm_pp_rejections;
    extern int hmm_pp_sptimeouts;
    extern int hmm_pp_ntimeouts;
    extern int hmm_pp_other_cases;
//    extern int hmm_pp_no_decisions;

    cy_lpwwd_log_info("[%s]->stats[%d,%d,%d,%d,%d,%d,%d,%d]",
            get_print_detection_str(wwd_status), hmm_pp_detections,
            hmm_pp_kwrejections, hmm_pp_rejections, hmm_pp_sptimeouts,
            hmm_pp_ntimeouts, hmm_pp_other_cases,
			svc_lp_stats_get_feed_counter(),
            lpwwd_context->audio_frame_count)

    ((void)(hmm_pp_detections));
    ((void)(hmm_pp_kwrejections));
    ((void)(hmm_pp_rejections));
    ((void)(hmm_pp_sptimeouts));
    ((void)(hmm_pp_ntimeouts));
    ((void)(hmm_pp_other_cases));
}

static cy_rslt_t cy_lpwwd_validate_init_params(cy_lpwwd_config_params_t *config_params)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    cy_lpwwd_log_info("Init: %d [%p %p] [%p %p %p] [%p %p %p]",
            config_params->dual_wake_word_detection,
            config_params->ml_model_binary_buf,
            config_params->ml_model_meta_buf,
            config_params->hmm_model1_keyword_binary_buf,
            config_params->hmm_model1_garbage_binary_buf,
            config_params->hmm_model1_noise_binary_buf,
            config_params->hmm_model2_keyword_binary_buf,
            config_params->hmm_model2_garbage_binary_buf,
            config_params->hmm_model2_noise_binary_buf);

#ifdef ENABLE_IFX_INFERENCE
    if((NULL == config_params->ml_model_meta_buf) ||
            (NULL == config_params->ml_model_binary_buf) )
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Invalid ml model %p,%p",
                config_params->ml_model_meta_buf,
                config_params->ml_model_binary_buf );
        return result;
    }
#else
#ifdef ENABLE_SVC_ML_MW_SUPPORT
    //Models will be passed directly using MODEL_NAME variable in makefile
#else
    if(NULL == config_params->ml_model_binary_buf)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Invalid ml model %p",
                config_params->ml_model_binary_buf );
        return result;
    }
#endif
#endif

#ifndef HMMS_CONFIG_MODEL
    if((NULL == config_params->hmm_model1_keyword_binary_buf) ||
            (NULL == config_params->hmm_model1_garbage_binary_buf) ||
            (NULL == config_params->hmm_model1_noise_binary_buf))
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Invalid hmm params %p,%p,%p",
                config_params->hmm_model1_keyword_binary_buf,
                config_params->hmm_model1_garbage_binary_buf,
                config_params->hmm_model1_noise_binary_buf);
        return result;
    }
#endif

    if(true == config_params->dual_wake_word_detection)
    {
#if 1
        result = CY_RSLT_LPWWD_NOT_SUPPORTED;
        cy_lpwwd_log_err(result, "Dual WWD mnot supported");
        return result;
#else
        if((NULL == config_params->hmm_model2_keyword_binary_buf) ||
                (NULL == config_params->hmm_model2_garbage_binary_buf) ||
                (NULL == config_params->hmm_model2_noise_binary_buf))
        {
            result = CY_RSLT_LPWWD_BAD_ARG;
            cy_lpwwd_log_err(result, "Invalid hmm params %p,%p,%p",
                    config_params->hmm_model2_keyword_binary_buf,
                    config_params->hmm_model2_garbage_binary_buf,
                    config_params->hmm_model2_noise_binary_buf);
            return result;
        }
#endif
    }

    return result;
}


cy_rslt_t cy_lpwwd_init(cy_lpwwd_config_params_t *config_params,
        cy_lpwwd_handle_t *handle)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_lpwwd_ml_ctrl_params_t ml_ctrl = { 0 };
    cy_lpwwd_ml_output_info_t ml_output_info = { 0 };
#ifdef COMPONENT_MEMORY_CHECK_UTILS
    long mem=0;
#endif


    if (NULL == config_params || NULL == handle)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Invalid params %p,%p", config_params, handle);
        return result;
    }

    *handle = NULL;

    result = cy_lpwwd_validate_init_params(config_params);
    if(CY_RSLT_SUCCESS != result)
    {
        return result;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem at start : %ld", __LINE__, __FUNCTION__, (unsigned int)mem);
    (void)mem;
#endif

    /* Allocate memory for LPWWD context */
    cy_lpwwd_context_t *lpwwd_context = NULL;
    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                (uint32_t) sizeof(cy_lpwwd_context_t), (void**) &lpwwd_context);

        if (NULL != lpwwd_context)
        {
            memset(lpwwd_context, 0, sizeof(cy_lpwwd_context_t));
        }
    }
    else
    {
    	lpwwd_context = calloc(1, sizeof(cy_lpwwd_context_t));
    }
    if (NULL == lpwwd_context)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_log_err(result, "Mem alloc fail for context");
        return result;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after lpwwd_context alloc : %ld", __LINE__, __FUNCTION__, mem);
    (void)(mem);
#endif

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    CY_MEM_UTIL_PRINT_ADDR("Alloc:lpwwd_handle", lpwwd_context,
            sizeof(cy_lpwwd_context_t));
#endif

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    /* Initialize Speech onset detection module */
    result = cy_sod_init(&config_params->sod_config,
            &lpwwd_context->sod_handle);
    if (CY_RSLT_SUCCESS == result)
    {
        cy_lpwwd_log_info("SOD Init success");
    }
    else
    {
        cy_lpwwd_log_err(result, "SOD init fail");
        goto cleanup;
    }
    lpwwd_context->sod_trigger_status = true;
#endif

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after sod_init : %ld", __LINE__, __FUNCTION__, mem);
#endif

    /* Initialize Pre wake word component default parameters */
    lpwwd_context->prewwd_config.frame_size = (FRAME_SIZE_MS * SAMPLE_RATE / 1000);
    lpwwd_context->prewwd_config.frame_shift = (FRAME_SHIFT_MS * SAMPLE_RATE / 1000);
    lpwwd_context->prewwd_config.number_of_dct_coefficients = NUM_MFCC_COEFFS;
    lpwwd_context->prewwd_config.number_of_filter_banks = NUM_FBANK_BINS;
    lpwwd_context->prewwd_config.sampling_rate = SAMPLE_RATE;
    lpwwd_context->prewwd_config.audio_input_frame_size = MONO_FRAME_SIZE;

    lpwwd_context->postwwd_config.sampling_rate = SAMPLE_RATE;
    lpwwd_context->postwwd_config.frame_rate = NN_FRAME_RATE;
    lpwwd_context->postwwd_config.stacked_frame_delay = (int16_t)(
            0.5 + NN_STACKED_DELAY_SEC * (1l << Q_LOOKBACK));
    lpwwd_context->postwwd_config.detection_threshold = HMM_PP_THD_FIXED;
    lpwwd_context->postwwd_config.lookback_buffer_length = config_params->lookback_buffer_length;

    /* Initialize pre wake word component */
    result = cy_lpwwd_prewwd_init(&lpwwd_context->prewwd_config,
            &lpwwd_context->number_of_features);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_lpwwd_log_err(result, "PreWWD init fail");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after prewwd_init : %ld", __LINE__, __FUNCTION__, mem);
#endif

#ifndef ENABLE_SVC_ML_MW_SUPPORT
#if defined(COMPONENT_CM33)
#if defined(COMPONENT_NNLITE)
    result = cy_lpwwd_ml_nnlite_init();
    if(CY_RSLT_SUCCESS != result)
	{
    	cy_lpwwd_log_err(result, "[LPWWD-ML-Kernel init fail");
        return result;
    }
#endif
#endif
#endif


    /* Initialize Machine learning inference component */
    ml_ctrl.ml_model_binary_buf = config_params->ml_model_binary_buf;
    ml_ctrl.ml_model_meta_buf = config_params->ml_model_meta_buf;
    ml_ctrl.num_features = lpwwd_context->number_of_features;
    ml_ctrl.feature_scale = config_params->feature_scale;
    ml_ctrl.feature_offset = config_params->feature_offset;

    result = cy_lpwwd_ml_init(&ml_ctrl, &ml_output_info);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_lpwwd_log_err(result, "Failed to initialize ML component");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after ml_init : %ld", __LINE__, __FUNCTION__, mem);
#endif

    /* Initialize Post processing wake word component */
    lpwwd_context->number_of_feature_frames =
            ml_output_info.number_of_feature_frames;
    lpwwd_context->number_of_tokens = ml_output_info.number_of_tokens;

    lpwwd_context->postwwd_config.number_of_tokens =
            ml_output_info.number_of_tokens;

#ifndef HMMS_CONFIG_MODEL
    ppkwmodel = (int16_t *)config_params->hmm_model1_keyword_binary_buf;
    ppgmodel = (int16_t *)config_params->hmm_model1_garbage_binary_buf;
    ppnmodel = (int16_t *)config_params->hmm_model1_noise_binary_buf;
#endif

    /* Initialize post wake-word component */
    result = cy_lpwwd_postwwd_init(&lpwwd_context->postwwd_config);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_lpwwd_log_err(result,
                "Failed to initialize post wake-word component");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after postwwd_init : %ld", __LINE__, __FUNCTION__, mem);
#endif

    /*
     * TODO: To be done for ES100.
     * */
#if 0
    if(true == config_params->dual_wake_word_detection)
    {
        if((NULL == config_params->postwwd_config.hmm_model2_keyword_binary_buf) ||
                (NULL == config_params->postwwd_config.hmm_model2_garbage_binary_buf) ||
                (NULL == config_params->postwwd_config.hmm_model2_noise_binary_buf))
        {
            cy_lpwwd_log_err(result, "Invalid params %p,%p,%p",
                    config_params->postwwd_config.hmm_model2_keyword_binary_buf,
                    config_params->postwwd_config.hmm_model2_garbage_binary_buf,
                    config_params->postwwd_config.hmm_model2_noise_binary_buf);
            goto cleanup;
        }

#ifndef HMMS_CONFIG_MODEL
            ppkwmodel = (int16_t *)config_params->postwwd_config.hmm_model2_keyword_binary_buf;
            ppgmodel = (int16_t *)config_params->postwwd_config.hmm_model2_garbage_binary_buf;
            ppnmodel = (int16_t *)config_params->postwwd_config.hmm_model2_noise_binary_buf;
#endif
        /* Initialize post wake-word component */
        result = cy_lpwwd_postwwd_init(&lpwwd_context->postwwd_config);
        if (CY_RSLT_SUCCESS != result)
        {
            cy_lpwwd_log_err(result,
                    "Failed to initialize post wake-word component");
            goto cleanup;
        }
    }
#endif

    // set number of audio frame shifts, which determines how often feature extraction is called
    lpwwd_context->num_audio_frame_shifts =
            (FRAME_SHIFT_MS * SAMPLE_RATE / 1000) / MONO_FRAME_SIZE;


#ifdef COMPONENT_MEMORY_CHECK_UTILS
    CY_MEM_UTIL_PRINT_ADDR("Alloc:lpwwd_feature_frame_buffer",
            lpwwd_context->feature_buffer.feature_frame_buffer,
            lpwwd_context->number_of_features * sizeof(LPWWD_ML_DATA_TYPE_T));
#endif

    /**
     * Allocate single feature frame buffer
     */
    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_FEATURE_BUFFER,
                (uint32_t) lpwwd_context->number_of_features
                        * sizeof(LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T),
                (void**) &lpwwd_context->feature_buffer.feature_frame_buffer);

        if (NULL != lpwwd_context->feature_buffer.feature_frame_buffer)
        {
            memset(lpwwd_context->feature_buffer.feature_frame_buffer, 0,
                    lpwwd_context->number_of_features
                            * sizeof(LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T));
        }
    }
    else
    {
    	lpwwd_context->feature_buffer.feature_frame_buffer =
            (LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T*) calloc(1,
                    lpwwd_context->number_of_features
                            * sizeof(LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T));
    }
    if (lpwwd_context->feature_buffer.feature_frame_buffer == NULL)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_log_err(result, "Fail to allocate memory");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after feature_frame_buf alloc : %ld", __LINE__, __FUNCTION__, mem);
#endif

    lpwwd_context->feature_buffer.feature_frame_buffer_size =
            lpwwd_context->number_of_features
                    * sizeof(LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T);


    /**
     * Allocate memory for Output score for ML output
     **/
    if(svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
            (uint32_t)(lpwwd_context->postwwd_config.number_of_tokens
            * sizeof(LPWWD_ML_OP_DATA_TYPE_T)),
            (void **)&lpwwd_context->output_score.output_score_buffer);

        if(NULL != lpwwd_context->output_score.output_score_buffer)
        {
            memset(lpwwd_context->output_score.output_score_buffer,0,
            lpwwd_context->postwwd_config.number_of_tokens
            * sizeof(LPWWD_ML_OP_DATA_TYPE_T));
        }
    }
    else
    {
        lpwwd_context->output_score.output_score_buffer =
            (LPWWD_ML_OP_DATA_TYPE_T*) calloc(1,
                lpwwd_context->postwwd_config.number_of_tokens
                * sizeof(LPWWD_ML_OP_DATA_TYPE_T));
    }

    if (lpwwd_context->output_score.output_score_buffer == NULL)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_log_err(result, "failed to allocate memory");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after output_score_buf alloc : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

    lpwwd_context->output_score.output_score_buffer_size =
            (lpwwd_context->postwwd_config.number_of_tokens
                    * sizeof(LPWWD_ML_OP_DATA_TYPE_T));

    /**
     * Allocate memory for Output score feed for PostWWD input
     **/
    if (svc_alloc_memory)
    {
        svc_alloc_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                (uint32_t)(
                        lpwwd_context->postwwd_config.number_of_tokens
                                * sizeof(LPWWD_POSTWWD_IP_DATA_TYPE_T)),
                (void**) &lpwwd_context->output_score_fixed);

        if (NULL != lpwwd_context->output_score_fixed)
        {
            memset(lpwwd_context->output_score_fixed, 0,
                    lpwwd_context->postwwd_config.number_of_tokens
                            * sizeof(LPWWD_POSTWWD_IP_DATA_TYPE_T));
        }
    }
    else
    {
        lpwwd_context->output_score_fixed = (LPWWD_POSTWWD_IP_DATA_TYPE_T*) calloc(1,
            lpwwd_context->postwwd_config.number_of_tokens
            * sizeof(LPWWD_POSTWWD_IP_DATA_TYPE_T));
    }
    if (lpwwd_context->output_score_fixed == NULL)
    {
        result = CY_RSLT_LPWWD_OUT_OF_MEMORY;
        cy_lpwwd_log_err(result, "failed to allocate memory");
        goto cleanup;
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after output_score_fixed alloc : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    CY_MEM_UTIL_PRINT_ADDR("Alloc:lpwwd_output_score_buffer",
            lpwwd_context->output_score.output_score_buffer,
            lpwwd_context->postwwd_config.number_of_tokens
                    * sizeof(LPWWD_ML_OP_DATA_TYPE_T));
#endif


#ifdef COMPONENT_MEMORY_CHECK_UTILS
    CY_MEM_UTIL_PRINT_ADDR("Alloc:lpwwd_output_score_fixed", lpwwd_context->output_score_fixed, lpwwd_context->postwwd_config.number_of_tokens * sizeof(LPWWD_POSTWWD_IP_DATA_TYPE_T));
#endif

    lpwwd_context->config_input_param = *config_params;
    *handle = lpwwd_context;

    cy_lpwwd_log_info(
            "LPWWD init success, Handle:%p, DataType[PreWOp:%d,MLOp:%d,PostWIp:%d]",
            lpwwd_context, sizeof(LPWWD_PREWWD_FEATURE_FRAME_DATA_TYPE_T),
            sizeof(LPWWD_ML_OP_DATA_TYPE_T),
            sizeof(LPWWD_POSTWWD_IP_DATA_TYPE_T));


#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_ML_MODEL_PRINT)
    (void) print_ml_model(
            (const char*) config_params->ml_model_binary_buf);
#endif

    return CY_RSLT_SUCCESS;
    cleanup:

    if(NULL != lpwwd_context)
    {
        (void) cy_lpwwd_deinit((cy_lpwwd_handle_t*) &lpwwd_context);
        lpwwd_context = NULL;
    }

    return result;
}

cy_rslt_t cy_lpwwd_feed(cy_lpwwd_handle_t handle, char *data,
        cy_lpwwd_wwd_detect_status_t *wwd_status)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int16_t *input_data_to_prewwd = NULL;
    cy_lpwwd_wwd_detect_status_t wake_word_status = CY_LPWWD_WAKE_WORD_INVALID;
    cy_lpwwd_context_t *lpwwd_context = NULL;

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    int n = 0;
    cy_sod_status_t sod_status = CY_SOD_STATUS_INVALID;
    int16_t *audio_data = (int16_t*) data;
#endif

    if (NULL == handle || NULL == data || NULL == wwd_status)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Invalid args: %p,%p,%p", handle, data,
                wwd_status);
        return result;
    }

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEED_PRINT)
    PRINT_ARR_AS_CHAR("InputF",data);
#endif

    lpwwd_context = (cy_lpwwd_context_t*) handle;

    /** SOD processing start */
    /**
     * Perform SOD processing and buffer the data needed for shifting before
     * feeding it to lpwwd
     */
    input_data_to_prewwd = (int16_t *)data; /** Assign buffer without shifting */
#ifdef ENABLE_LPWWD_MW_WITH_SOD
    for (n = 0; n < MONO_FRAME_SIZE; n++)
    {
        lpwwd_context->circular_buffer[lpwwd_context->write_index] =
                audio_data[n];
        lpwwd_context->write_index = circular_index(lpwwd_context->write_index,
                1, CIRCULAR_BUFFER_SIZE);
    }

#ifdef ENABLE_SOD_PROFILLING
    start_time();
#endif

    result = cy_sod_process(lpwwd_context->sod_handle,
            lpwwd_context->sod_trigger_status, (int16_t*) data, &sod_status);

#ifdef ENABLE_SOD_PROFILLING
    stop_time();
    ++lpwwd_context->frame_counter_for_sod;

    lpwwd_context->total_count_sod += get_time();

    if(lpwwd_context->frame_counter_for_sod == FRAME_COUNT)
    {
        printf("SOD MCPS(1sec data) : %d \n", lpwwd_context->total_count_sod);
    }
#endif

    if ((CY_RSLT_SUCCESS == result) && (sod_status == CY_SOD_STATUS_DETECTED))
    {
        lpwwd_context->sod_detected = true;
        lpwwd_context->sod_trigger_status = false;
        cy_lpwwd_log_info("SOD Detected, frame index:[%ld]",
                lpwwd_context->audio_frame_index);
    }

    // copy from circular buffer to lpwwd capture buffer, go back in time by extra amount to account for delays
    lpwwd_context->wwd_audio_read_idx = circular_index(
            lpwwd_context->write_index, -(LOOK_BACK_SIZE + MONO_FRAME_SIZE),
            CIRCULAR_BUFFER_SIZE);
    for (n = 0; n < MONO_FRAME_SIZE; n++)
    {
        lpwwd_context->wwd_audio_frame[n] =
                lpwwd_context->circular_buffer[lpwwd_context->wwd_audio_read_idx];
        lpwwd_context->wwd_audio_read_idx = circular_index(
                lpwwd_context->wwd_audio_read_idx, 1,
                CIRCULAR_BUFFER_SIZE);
    }
    input_data_to_prewwd = lpwwd_context->wwd_audio_frame;

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEED_PRINT_SOD_SHIFT)
    PRINT_ARR_AS_CHAR("InputFSODShift:",input_data_to_prewwd);
#endif

#endif
    /** SOD processing completed */

#ifdef COMPONENT_PROFILER
    cy_lpwwd_profile(LPWWD_PROFILE_CMD_START_OVERALL,NULL);
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

    result = cy_lpwwd_prewwd_feed((int16_t*) input_data_to_prewwd);
    if (CY_RSLT_SUCCESS != result)
    {
        cy_lpwwd_log_err(result,
                "cy_lpwwd_prewwd_feed fail");
        return result;
    }
#ifdef ENABLE_SVC_LP_CHECK_POINT
    SVC_LP_CHECK_POINT();
#endif

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    if (lpwwd_context->sod_detected)
#endif
    {
#ifdef ENABLE_SOD_PROFILLING
        lpwwd_context->frame_counter_after_sod++;

        if(lpwwd_context->first_sod_detection == false)
        {
            printf("MCPS calculation started \r\n");
            start_time();
            lpwwd_context->first_sod_detection = true;
        }
        else{
            start_time();
        }
#endif

        *wwd_status = CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS;

        // run feature extraction and inference for every feature frame shift
        lpwwd_context->audio_frame_shift_count++;

        if (lpwwd_context->audio_frame_shift_count
                == lpwwd_context->num_audio_frame_shifts)
        {

#ifdef COMPONENT_PROFILER
        	cy_lpwwd_profile(LPWWD_PROFILE_CMD_START_FE,NULL);
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
        	SVC_LP_CHECK_POINT();
#endif
            // compute feature frame
            result = cy_lpwwd_prewwd_generate_feature_frame(
                    &lpwwd_context->feature_buffer);
            if (CY_RSLT_SUCCESS != result)
            {
                cy_lpwwd_log_err(result, "Failed to generate feature frame");
                return result;
            }
#ifdef ENABLE_SVC_LP_CHECK_POINT
            SVC_LP_CHECK_POINT();
#endif

#ifdef COMPONENT_PROFILER
        	cy_lpwwd_profile(LPWWD_PROFILE_CMD_STOP_FE,NULL);
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_FRAME_PRINT)
            cy_lpwwd_print_feature_frame(lpwwd_context);
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
            SVC_LP_CHECK_POINT();
#endif
            result = cy_lpwwd_ml_feed(lpwwd_context->feature_buffer.feature_frame_buffer);
            if (CY_RSLT_SUCCESS != result)
            {
                cy_lpwwd_log_err(result,
                        "Failed to feed feature buffer to ML inference");
                return result;
            }
#ifdef ENABLE_SVC_LP_CHECK_POINT
            SVC_LP_CHECK_POINT();
#endif

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FEATURE_BUF_MATRIX_PRINT)
            printf_feature_buffer_matrix("FFMatrix",
                (const char*) cy_lpwwd_ml_get_feature_matrix_dbg(),
                lpwwd_context->number_of_features,
                lpwwd_context->number_of_feature_frames);
#endif

            lpwwd_context->feature_frame_count++;

            // wait until feature buffer becomes full
            if (lpwwd_context->feature_frame_count
                    >= lpwwd_context->number_of_feature_frames)
            {

#ifdef COMPONENT_PROFILER
            	cy_lpwwd_profile(LPWWD_PROFILE_CMD_START_ML,NULL);
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
            	SVC_LP_CHECK_POINT();
#endif
                result = cy_lpwwd_ml_generate_output_score( &lpwwd_context->output_score);
                if (CY_RSLT_SUCCESS != result)
                {
                    cy_lpwwd_log_err(result, "Failed to generate output score");
                    return result;
                }
#ifdef ENABLE_SVC_LP_CHECK_POINT
                SVC_LP_CHECK_POINT();
#endif

#ifdef COMPONENT_PROFILER
                cy_lpwwd_profile(LPWWD_PROFILE_CMD_STOP_ML,NULL);
#endif

                // Run post-processing
                for (int n = 0; n < lpwwd_context->number_of_tokens; n++)
                {
#ifdef RUN_FLOAT_FE
                    /**
                     * Float to int16_t conversion will happen
                     */
                    lpwwd_context->output_score_fixed[n] =
                            (LPWWD_POSTWWD_IP_DATA_TYPE_T) (lpwwd_context->output_score.output_score_buffer[n]
                                    * 32767 + 0.5f);
#else
                    /**
                     * int16_ to int16_t copy will happen
                     */
                    lpwwd_context->output_score_fixed[n] =
                        (LPWWD_POSTWWD_IP_DATA_TYPE_T) lpwwd_context->output_score.output_score_buffer[n];
#endif
                }

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_SCORE_PRINT)
                print_output_score(lpwwd_context);
#endif

#ifdef COMPONENT_PROFILER
                cy_lpwwd_profile(LPWWD_PROFILE_CMD_START_HMMS,NULL);
#endif

#ifdef ENABLE_SVC_LP_CHECK_POINT
            SVC_LP_CHECK_POINT();
#endif
                result = cy_lpwwd_postwwd_process(
                        lpwwd_context->output_score_fixed, &wake_word_status);
                if (CY_RSLT_SUCCESS != result)
                {
                    cy_lpwwd_log_err(result, "POSTWWD process fail");
                    return result;
                }
#ifdef ENABLE_SVC_LP_CHECK_POINT
            SVC_LP_CHECK_POINT();
#endif


#ifdef COMPONENT_PROFILER
                cy_lpwwd_profile(LPWWD_PROFILE_CMD_STOP_HMMS,NULL);
#endif

                *wwd_status = wake_word_status;
            }

            lpwwd_context->audio_frame_shift_count = 0;
        }

        lpwwd_context->audio_frame_index_after_sod++;
        lpwwd_context->audio_frame_count++;

#if (ENABLE_LPWWD_DEBUG & ENABLE_LPWWD_DBG_FORCE_ML_PROC_ALWAYS)

        /* Useful to do ML operation ignoring Post WWD process*/
        *wwd_status = CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS;

#else
        if (lpwwd_context->audio_frame_count
                >= MAX_LPWWD_FEED_FRAMES_COUNT_FOR_WW_DETECT_TIMEOUT)
        {
            *wwd_status = CY_LPWWD_TIMEOUT;
            cy_lpwwd_log_info("FORCE WWD TO, FrCount: [%d]",
                    lpwwd_context->audio_frame_count);
        }

        if (*wwd_status != CY_LPWWD_WAKE_WORD_INVALID)
        {
            if (*wwd_status != CY_LPWWD_WAKE_WORD_DETECTION_IN_PROGRESS)
            {
                print_wwd_detection_result(lpwwd_context, wwd_status);
                /* Reset LPWWD variables */
                cy_lpwwd_reset(lpwwd_context);
            }
        }
#endif
    }

#ifdef ENABLE_SOD_PROFILLING
    if(lpwwd_context->first_sod_detection == true)
    {
        stop_time();
        lpwwd_context->total_count += get_time();
    }
#endif

    lpwwd_context->audio_frame_index++;

#ifdef COMPONENT_PROFILER
    cy_lpwwd_profile(LPWWD_PROFILE_CMD_STOP_OVERALL,NULL);
#endif

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_lpwwd_reset(cy_lpwwd_handle_t handle)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_lpwwd_context_t *context = NULL;

    if (NULL == handle)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Handle is NULL [%p]", handle);
        return result;
    }

    context = (cy_lpwwd_context_t*) handle;

    context->feature_frame_count = 0;
    context->audio_frame_count = 0;

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    context->sod_detected = false;
    context->sod_trigger_status = true;
#endif

    /**
     * FE reset is not required as the SOD reset is done, once the SOD is detected
     * the FE buffer would be overwritten with new data.
     */
    // reset these in preparation for next onset
    cy_lpwwd_reset_audio_buffer( );
    cy_lpwwd_postwwd_reset( );

    return result;
}

cy_rslt_t cy_lpwwd_deinit(cy_lpwwd_handle_t *handle)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    cy_rslt_t result_priv = CY_RSLT_SUCCESS;
    cy_lpwwd_context_t *context = NULL;
#ifdef COMPONENT_MEMORY_CHECK_UTILS
    long mem = 0;
#endif

    if (NULL == handle)
    {
        result = CY_RSLT_LPWWD_BAD_ARG;
        cy_lpwwd_log_err(result, "Handle is NULL [%p]", handle);
        return result;
    }

    context = (cy_lpwwd_context_t*) *handle;

    cy_lpwwd_log_info("Deinit handle:%p",context);

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, at deinit start : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

#ifdef ENABLE_LPWWD_MW_WITH_SOD
    result_priv = cy_sod_deinit(&context->sod_handle);
    if (CY_RSLT_SUCCESS != result_priv)
    {
        cy_lpwwd_log_err(result_priv, "Failed to de-initialize SOD");
        result = result_priv;
    }
    else
    {
        cy_lpwwd_log_info("SOD deinit success");
    }
#endif

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after sod_deinit : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

    result_priv = cy_lpwwd_prewwd_deinit();
    if (CY_RSLT_SUCCESS != result_priv)
    {
        cy_lpwwd_log_err(result_priv,
                "Failed to de-initialize pre wake-word component");
        result = result_priv;
    }
    else
    {
        cy_lpwwd_log_info("PreWWD deinit success");
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after prewwd_deinit : %u \r\n", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

    result_priv = cy_lpwwd_ml_deinit( );
    if (CY_RSLT_SUCCESS != result_priv)
    {
        cy_lpwwd_log_err(result, "Failed to de-initialize ML component");
        result = result_priv;
    }
    else
    {
        cy_lpwwd_log_info("ML deinit success");
    }

#ifndef ENABLE_SVC_ML_MW_SUPPORT
#if defined(COMPONENT_CM33)
#if defined(COMPONENT_NNLITE)
    cy_lpwwd_ml_nnlite_deinit();
#endif
#endif
#endif

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after ml_deinit : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

    result_priv = cy_lpwwd_postwwd_deinit( );
    if (CY_RSLT_SUCCESS != result_priv)
    {
        cy_lpwwd_log_err(result_priv,
                "Failed to de-initialize post wake-word component");
        result = result_priv;
    }
    else
    {
        cy_lpwwd_log_info("PostWWD deinit success");
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after postwwd deinit : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif


    if (NULL != context->output_score.output_score_buffer)
    {
#ifdef COMPONENT_MEMORY_CHECK_UTILS
        CY_MEM_UTIL_PRINT_ADDR("Free:lpwwd_output_score_buffer", context->output_score.output_score_buffer, 0);
#endif

        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                    context->output_score.output_score_buffer);
        }
        else
        {
            free(context->output_score.output_score_buffer);
        }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
        mem = malloc_info_command();
        cy_lpwwd_log_dbg("%d, %s, mem after output_score_buf free : %ld", __LINE__, __FUNCTION__, mem);
        (void)mem;
#endif
    }
    else
    {
        cy_lpwwd_log_info("OP Score buf not created");
    }

    if (NULL != context->feature_buffer.feature_frame_buffer)
    {
#ifdef COMPONENT_MEMORY_CHECK_UTILS
        CY_MEM_UTIL_PRINT_ADDR("Free:lpwwd_feature_frame_buffer", context->feature_buffer.feature_frame_buffer, 0);
#endif
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_FEATURE_BUFFER,
                    context->feature_buffer.feature_frame_buffer);
        }
        else
        {
            free(context->feature_buffer.feature_frame_buffer);
        }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
        mem = malloc_info_command();
        cy_lpwwd_log_dbg("%d, %s, mem after feature_frame_buf free : %ld", __LINE__, __FUNCTION__, mem);
        (void)mem;
#endif
    }
    else
    {
        cy_lpwwd_log_info("FF buf not created");
    }

    if (NULL != context->output_score_fixed)
    {
#ifdef COMPONENT_MEMORY_CHECK_UTILS
        CY_MEM_UTIL_PRINT_ADDR("Free:lpwwd_output_score_fixed", context->output_score_fixed, 0);
#endif
        if (svc_free_memory)
        {
            svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY,
                    context->output_score_fixed);
        }
        else
        {
            free(context->output_score_fixed);
        }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
        mem = malloc_info_command();
        cy_lpwwd_log_dbg("%d, %s, mem after output_score_fixed free : %ld", __LINE__, __FUNCTION__, mem);
        (void)mem;
#endif
    }
    else
    {
        cy_lpwwd_log_info("OP score fixed buf not created");
    }

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    CY_MEM_UTIL_PRINT_ADDR("Free:lpwwd_handle", context, 0);
#endif

    if (svc_free_memory)
    {
        svc_free_memory(CY_SVC_MEM_ID_GENERIC_MEMORY, context);
    }
    else
    {
        free(context);
    }
    *handle = NULL;

#ifdef COMPONENT_MEMORY_CHECK_UTILS
    mem = malloc_info_command();
    cy_lpwwd_log_dbg("%d, %s, mem after lpwwd_context free : %ld", __LINE__, __FUNCTION__, mem);
    (void)mem;
#endif

    return result;
}

