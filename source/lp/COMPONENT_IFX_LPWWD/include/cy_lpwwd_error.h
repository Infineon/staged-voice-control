/******************************************************************************
 * File Name: cy_lpwwd_error.h
 *
 * Description: Header file for error codes.
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
#ifndef __CY_LPWWD_ERROR_H
#define __CY_LPWWD_ERROR_H

#include "cy_result.h"
#include "cy_result_mw.h"


/**
 * \defgroup low power wake word middleware results/error codes
 * @ingroup low_power_wake_word_macros
 *
 * LPWWD MW APIs return results of type cy_rslt_t and consist of three parts:
 * - module base
 * - type
 * - error code
 *
 * \par Result Format
 *
   \verbatim
              Module base                            Type     Library-specific error code
      +--------------------------------------------+--------+------------------------------+
      |CY_RSLT_MODULE_LPWWD_BASE      | 0x2        |           Error Code                  |
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

/******************************************************
 *                      Typedefs
 ******************************************************/

/** LPWWD error code base. */
#define CY_RSLT_LPWWD_ERR_BASE                         CY_RSLT_CREATE(CY_RSLT_TYPE_ERROR, CY_RSLT_MODULE_LPWWD_BASE, 0)

/** Bad arguments*/
#define CY_RSLT_LPWWD_BAD_ARG                           ( CY_RSLT_LPWWD_ERR_BASE + 1 )

/** Out of memory*/
#define CY_RSLT_LPWWD_OUT_OF_MEMORY                     ( CY_RSLT_LPWWD_ERR_BASE + 2 )

/** ML Model parse error */
#define CY_RSLT_LPWWD_ERROR_ML_MODEL_PARSE              ( CY_RSLT_LPWWD_ERR_BASE + 3 )

/** ML Model init fail */
#define CY_RSLT_LPWWD_ERROR_ML_MODEL_INIT               ( CY_RSLT_LPWWD_ERR_BASE + 4 )

/** ML Inference error */
#define CY_RSLT_LPWWD_ERROR_ML_INFERENCE                ( CY_RSLT_LPWWD_ERR_BASE + 5 )

/** Post processing fail */
#define CY_RSLT_LPWWD_ERROR_POST_PROCESS_PARSING        ( CY_RSLT_LPWWD_ERR_BASE + 6 )

/** Post processing init fail */
#define CY_RSLT_LPWWD_ERROR_POST_PROCESS_INIT           ( CY_RSLT_LPWWD_ERR_BASE + 7 )

/** Post processing generic fail */
#define CY_RSLT_LPWWD_ERROR_POST_PROCESS                ( CY_RSLT_LPWWD_ERR_BASE + 8 )

/** Post processing reset fail */
#define CY_RSLT_LPWWD_ERROR_POST_PROCESS_RESET          ( CY_RSLT_LPWWD_ERR_BASE + 9 )

/** Post processing init fail */
#define CY_RSLT_LPWWD_ERROR_PREWWD_INIT                 ( CY_RSLT_LPWWD_ERR_BASE + 10 )

/** Post processing generic error */
#define CY_RSLT_LPWWD_PREWWD_GENERIC_ERROR              ( CY_RSLT_LPWWD_ERR_BASE + 11 )

/** PreWWD not initialized */
#define CY_RSLT_LPWWD_PREWWD_NOT_INTIAILIZED            ( CY_RSLT_LPWWD_ERR_BASE + 12 )

/** PreWWD already initialized */
#define CY_RSLT_LPWWD_PREWWD_ALREADY_INTIAILIZED        ( CY_RSLT_LPWWD_ERR_BASE + 13 )

/** PostWWD not initialized */
#define CY_RSLT_LPWWD_POSTWWD_NOT_INTIAILIZED           ( CY_RSLT_LPWWD_ERR_BASE + 14 )

/** PostWWD already initialized */
#define CY_RSLT_LPWWD_POSTWWD_ALREADY_INTIAILIZED       ( CY_RSLT_LPWWD_ERR_BASE + 15 )

/** Tlite version mismatch */
#define CY_RSLT_LPWWD_ML_TFLITE_VER_MISMATCH            ( CY_RSLT_LPWWD_ERR_BASE + 16 )

/** ML Invalid model */
#define CY_RSLT_LPWWD_ML_INVALID_MODEL                  ( CY_RSLT_LPWWD_ERR_BASE + 17 )

/** ML not initialized */
#define CY_RSLT_LPWWD_ML_NOT_INTIAILIZED                ( CY_RSLT_LPWWD_ERR_BASE + 18 )

/** Feature not supported */
#define CY_RSLT_LPWWD_NOT_SUPPORTED                     ( CY_RSLT_LPWWD_ERR_BASE + 19 )

/** \} low_power_wake_word_macros */

/******************************************************
 *                      Function Prototypes
 ******************************************************/
#endif /*__CY_LPWWD_ERROR_H */

/* [] END OF FILE */
