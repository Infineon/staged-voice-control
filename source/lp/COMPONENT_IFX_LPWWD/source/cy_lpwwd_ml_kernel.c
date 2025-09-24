/******************************************************************************
 * File Name: cy_lpwwd_ml_kernel.c
 *
 * Description: Implements functions required for kernel init for nnlite
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

/******************************************************
 *                      Typedefs
 ******************************************************/

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                      Variables
 ******************************************************/


/******************************************************
 *                      Function Prototypes
 ******************************************************/
#ifndef ENABLE_SVC_ML_MW_SUPPORT
#if defined(COMPONENT_CM33)
#if defined(COMPONENT_NNLITE)
#include "cy_nnlite.h"
#include "cy_nn_kernel.h"
#include "cyabs_rtos.h"

cy_mutex_t mw_nnliteMutex;
cy_semaphore_t mw_nnliteSem;
cy_kernel_config_t mw_cy_kernel_config = {0};

extern uint32_t Cy_NNLite_Mutex_Create(void *mutex);
extern uint32_t Cy_NNLite_Mutex_Lock(void *mutex);
extern uint32_t Cy_NNLite_Mutex_Unlock(void *mutex);
extern uint32_t Cy_NNLite_Mutex_Delete(void *mutex);
extern uint32_t Cy_NNLite_Sem_Create(void *sem);
extern uint32_t Cy_NNLite_Sem_Wait(void *sem);
extern uint32_t Cy_NNLite_Sem_Give(void *sem);
extern uint32_t Cy_NNLite_Sem_Delete(void *sem);

cy_rslt_t cy_lpwwd_ml_nnlite_init( )
{
    /* initialize with proper mutex/sema */
    mw_cy_kernel_config.nnliteMutex        = &mw_nnliteMutex;
    mw_cy_kernel_config.nnliteSem          = &mw_nnliteSem;

    mw_cy_kernel_config.cbArg              = NULL;
    mw_cy_kernel_config.completionCbFunc   = NULL;
    mw_cy_kernel_config.mutexCreateFunc    = Cy_NNLite_Mutex_Create; /* rtos_mutex_init */
    mw_cy_kernel_config.mutexDeleteFunc    = Cy_NNLite_Mutex_Delete;
    mw_cy_kernel_config.mutexLockFunc      = Cy_NNLite_Mutex_Lock;
    mw_cy_kernel_config.mutexUnlockFunc    = Cy_NNLite_Mutex_Unlock;
    mw_cy_kernel_config.SemCreateFunc      = Cy_NNLite_Sem_Create; /* rtos_sem_init */
    mw_cy_kernel_config.SemDeleteFunc      = Cy_NNLite_Sem_Delete;
    mw_cy_kernel_config.SemWaitFunc        = Cy_NNLite_Sem_Wait;
    mw_cy_kernel_config.SemGiveFunc        = Cy_NNLite_Sem_Give;
    if (Cy_NNLite_KernelInit(&mw_cy_kernel_config) != CY_NNLITE_SUCCESS)
    {
        return -1;
    }
    return 0;
}

cy_rslt_t cy_lpwwd_ml_nnlite_deinit( )
{
	cy_rslt_t result = CY_RSLT_SUCCESS;
	result = Cy_NNLite_KernelDeInit();
	return result;
}
#endif
#endif
#endif


/* [] END OF FILE */
