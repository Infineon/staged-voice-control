/******************************************************************************
 * File Name:   svc_model.c
 *
 * Description: SVC Models file. This contains MTB ML model + HMM model required
 * for wakeword detection.
 *
 *******************************************************************************/

#include "cy_svc_model.h"
#ifndef ENABLE_SVC_ML_MW_SUPPORT
#include "ALEXA_INT8_tflm_model_int8x8.h"

const bool SVC_MODEL_DUAL_WAKEWORD_ENABLED = false;
const char *SVC_MODEL_BINARY_BUFFER = (const char*) ALEXA_INT8_model_bin;
const char *SVC_MODEL_PARAMS_BUFFER = (const char*) NULL;
const unsigned int SVC_MODEL_ARENA_BUFFER_SIZE = ALEXA_INT8_ARENA_SIZE;
#else
const bool SVC_MODEL_DUAL_WAKEWORD_ENABLED = false;
const char *SVC_MODEL_BINARY_BUFFER = (const char*) NULL;
const char *SVC_MODEL_PARAMS_BUFFER = (const char*) NULL;
#endif

#ifdef ENABLE_IFX_LPWWD_HMMS
	const int16_t ppkwmodel1_input[] = {
	#include "AL_EXA_2023-10-14_pp_kw_hmm_int8_nn.h"
			};
	const int16_t ppgmodel1_input[] = {
	#include "AL_EXA_2023-10-14_pp_g_hmm_int8_nn.h"
			};
	const int16_t ppnmodel1_input[] = {
	#include "AL_EXA_2023-10-14_pp_n_hmm_int8_nn.h"
			};

	const char *SVC_MODEL_HMM_KEYWORD_1 = (const char*) ppkwmodel1_input;
	const char *SVC_MODEL_HMM_GARBAGE_1 = (const char*) ppgmodel1_input;
	const char *SVC_MODEL_HMM_NOISE_1 = (const char*) ppnmodel1_input;

	const char *SVC_MODEL_HMM_KEYWORD_2 = NULL;
	const char *SVC_MODEL_HMM_GARBAGE_2 = NULL;
	const char *SVC_MODEL_HMM_NOISE_2 = NULL;
#else
	const int32_t lpwwd_pp_config_params[] = {
		#include "ALEXA_LPWWD_PP_parms.h"
		};

	const int32_t* SVC_MODEL_LPWWD_PP_CONFIG_PARAMS = (const int32_t*)lpwwd_pp_config_params;
#endif /* ENABLE_IFX_LPWWD_HMMS */

