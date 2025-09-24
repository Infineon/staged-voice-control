/******************************************************************************
* File Name: cy_lpwwd_defines.h
*
* Description: Header file for LPWWD defines
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
* Macros
******************************************************************************/
/* Sample rate */
#define SAMPLE_RATE   (16000)

/* Frame size in ms for feature extraction */
#define FRAME_SIZE_MS   32

/* Frame shift in ms for feature extraction */
#define FRAME_SHIFT_MS  20

/* Number of feature banks */
#define NUM_FBANK_BINS  40

/* Number of MFCC coefficients. can be less than NUM_FBANK_BINS in general */
#define NUM_MFCC_COEFFS 20

/* Audio frame size in m. cannot be larger than and must be multiples of FRAME_SHIFT_MS */
#define AUDIO_FRAME_SIZE_MS 10

/* Audio capture buffer size in bytes for 10ms */
#define WWD_CAPTURE_BUFFER_SIZE (160)

#define APPLY_PP 1
#define LOOK_BACK_SIZE (8000) // 500 msec @ 16 kHz
#if APPLY_PP
#define AUDIO_CAPTURE_BUFFER_SIZE (64000) // 4 sec @ 16 kHz
#else
#define AUDIO_CAPTURE_BUFFER_SIZE (35200) // use 35200 (2.2 sec @ 16 kHz) for "Ok Infineon", 142 sec for "Alexa"
#endif
#define CIRCULAR_BUFFER_END_MARGIN (1600) // 100 msec, extra space added at end of circular buffer
#define CIRCULAR_BUFFER_SIZE (LOOK_BACK_SIZE + AUDIO_CAPTURE_BUFFER_SIZE + CIRCULAR_BUFFER_END_MARGIN)

//#define STEREO_FRAME_SIZE           (320)
#define MONO_FRAME_SIZE             (160)

/* Define how many samples in a frame */
#define FRAME_SIZE                  (320)

#define HMM_PP_THD (3.0) // can be changed by user
#define HMM_PP_THD_FIXED (0) // can be changed by user
#define NN_FRAME_RATE (1000 / FRAME_SHIFT_MS) // number of stacked frames per second
#define NN_STACKED_DELAY_SEC (0.36) // 360ms @ 16 kHz
#define NAIVE_PP_THD (0.5) // can be changed by user, 0.5 for real, 0.3 for synthetic
