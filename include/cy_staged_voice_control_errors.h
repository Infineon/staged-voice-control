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

/** @file cy_staged_voice_control_errors.h
 *
 * @brief This file is the header file for Staged voice control (SVC) library
 * error codes defines
 *
 * Abbreviations used in the header file
 *
 * SVC      -   Stage Voice Control
 */

#ifndef __CY_STAGED_VOICE_CONTROL_ERRORS_H__
#define __CY_STAGED_VOICE_CONTROL_ERRORS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cy_result.h"
#include "cy_result_mw.h"


/**
 * \defgroup staged_voice_control_results Staged voice control (SVC) results/error codes
 * @ingroup group_staged_voice_control_macros
 *
 * staged voice middleware APIs return results of type cy_rslt_t and consist of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base                            Type     Library-specific error code
      +--------------------------------------------+--------+------------------------------+
      |CY_RSLT_MODULE_CY_SVC_BASE     | 0x2 |           Error Code         |
      +--------------------------------------------+--------+------------------------------+
                14 bits               2 bits            16 bits

   See the macro section of this document for library-specific error codes.
   \endverbatim
 *
 * The data structure cy_rslt_t is part of cy_result.h located in <core_lib/include>
 *
 * Module base: This base is derived from CY_RSLT_MODULE_MIDDLEWARE_BASE (defined in cy_result.h) and is an offset of the CY_RSLT_MODULE_MIDDLEWARE_BASE.
 *              The details of the offset and the middleware base are defined in cy_result_mw.h, which is part of the [GitHub connectivity-utilities] (https://github.com/Infineon/connectivity-utilities) repo.
 *              For example, the buffer pool library uses CY_RSLT_MODULE_CY_SVC_BASE as the module base.
 *
 * Type: This type is defined in cy_result.h and can be one of CY_RSLT_TYPE_FATAL, CY_RSLT_TYPE_ERROR, CY_RSLT_TYPE_WARNING, or CY_RSLT_TYPE_INFO. AWS library error codes are of type CY_RSLT_TYPE_ERROR.
 *
 * Library-specific error code: These error codes are library-specific and defined in the macro section.
 *
 * Helper macros used for creating the library-specific result are provided as part of cy_result.h.
 * \{
 */

/** SVC error code base. */
#define CY_RSLT_SVC_ERR_BASE                         CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_STAGED_VOICE_CONTROL_BASE, 0)

/** Out of memory */
#define CY_RSLT_SVC_OUT_OF_MEMORY                   ( CY_RSLT_SVC_ERR_BASE + 1 )

/** Generic Error */
#define CY_RSLT_SVC_GENERIC_ERROR                   ( CY_RSLT_SVC_ERR_BASE + 2 )

/** Bad arguments */
#define CY_RSLT_SVC_BAD_ARG                         ( CY_RSLT_SVC_ERR_BASE + 3 )

/** SVC not initialized */
#define CY_RSLT_SVC_NOT_INITIALIZED                 ( CY_RSLT_SVC_ERR_BASE + 4 )

/** SVC MW already initialized */
#define CY_RSLT_SVC_ALREADY_INITIALIZED             ( CY_RSLT_SVC_ERR_BASE + 5 )

/** SVC MW max instance count*/
#define CY_RSLT_SVC_INVALID_MAX_INSTANCE_COUNT      ( CY_RSLT_SVC_ERR_BASE + 6 )

/** SVC MW instance not found */
#define CY_RSLT_SVC_INSTANCE_NOT_FOUND              ( CY_RSLT_SVC_ERR_BASE + 7 )

/** SVC MW Free instance not available */
#define CY_RSLT_SVC_INSTANCE_FREE_NOT_AVAILABLE     ( CY_RSLT_SVC_ERR_BASE + 8 )

/** SVC MW max instance reached */
#define CY_RSLT_SVC_INSTANCE_MAX_INSTANCE_REACHED   ( CY_RSLT_SVC_ERR_BASE + 9 )

/** SVC MW instance already exists */
#define CY_RSLT_SVC_INSTANCE_ALREADY_EXISTS         ( CY_RSLT_SVC_ERR_BASE + 10 )

/** SVC MW feed result in buffer overwrite*/
#define CY_RSLT_SVC_FEED_DETECTED_INPUT_BUFFER_OVERRIDE ( CY_RSLT_SVC_ERR_BASE + 11 )

/** SVC MW internal error on buffer write offset invalid */
#define CY_RSLT_SVC_CIRCULAR_BUFFER_INVALID_WR_OFFSET   ( CY_RSLT_SVC_ERR_BASE + 12 )

/** Invalid stage trigger */
#define CY_RSLT_SVC_INVALID_STAGE_TRIGGER            ( CY_RSLT_SVC_ERR_BASE + 13 )

/** Invalid IPC Size mismatch */
#define CY_RSLT_SVC_IPC_SIZE_MISMATCH                (CY_RSLT_SVC_ERR_BASE + 14 )

/** IPC Send fail */
#define CY_RSLT_SVC_IPC_SEND_FAIL                    (CY_RSLT_SVC_ERR_BASE + 15 )

/** Invalid state */
#define CY_RSLT_SVC_INVALID_STATE                    (CY_RSLT_SVC_ERR_BASE + 16 )

/** Data corruption detected in the buffer */
#define CY_RSLT_SVC_DATA_CORRUPTION                  (CY_RSLT_SVC_ERR_BASE + 17 )

/** Invalid stage */
#define CY_RSLT_SVC_INVALID_STAGE                    (CY_RSLT_SVC_ERR_BASE + 18 )

/**
 * Debug purpose: Circular buffer is corrupted by external software.
 * The Circular buffer start and end is guarded by Magic header. if any
 * external application attempts to tamper the circular buffer area,
 * SVC MW will detect the corruption and perform exit by logging an error code.
 */
#define CY_RSLT_SVC_CBUF_EXT_CORRUPTION              (CY_RSLT_SVC_ERR_BASE + 19 )

/** IPC create resource fail */
#define CY_RSLT_SVC_IPC_CREATE_RSRC_FAIL             (CY_RSLT_SVC_ERR_BASE + 20 )

/**
 * Debug purpose: IPC MSG counter mismatch. This error code will logged in the
 * SVC MW in case if there is issue in the buffer sharing IPC mechanism.
 * This error code is not returned to application.
 */
#define CY_RSLT_SVC_IPC_COUNTER_MISMATCH             ( CY_RSLT_SVC_ERR_BASE + 21)

/**
 * Debug purpose: (not required for real time application).This error will returned only
 * when the SVC-LP and SVC-HP compiled with the define "ENABLE_SVC_LP_HP_BUFFER_SYNC" is
 * enabled. Enabling "ENABLE_SVC_LP_HP_BUFFER_SYNC" provides support for synchronize
 * buffer share/access mechanism between two cores. This is needed only when the CM55
 * does slower process, i.e., detect buffer overwrite by Core1 due to application feed
 * to Core2.
 * This error code will be returned in cy_svc_lp_feed(), application can retry feeding the
 * same data on detection this error.
 */
#define CY_RSLT_SVC_DATA_OVERWRITE_WARNING           ( CY_RSLT_SVC_ERR_BASE + 22)

/** SVC init after deinit is not supported */
#define CY_RSLT_SVC_DEINIT_INIT_NOT_SUPPORTED        ( CY_RSLT_SVC_ERR_BASE + 23 )

/**
 * SVC Q is full
 */
#define CY_RSLT_SVC_Q_FULL                           (CY_RSLT_SVC_ERR_BASE + 24 )

/** SVC license expired */
#define CY_RSLT_SVC_LICENSE_EXPIRED                  (CY_RSLT_SVC_ERR_BASE + 25 )

/** \} group_staged_voice_control_macros */
#ifdef __cplusplus
} /*extern "C" */
#endif
#endif /* ifndef __CY_STAGED_VOICE_CONTROL_ERRORS_H__ */
