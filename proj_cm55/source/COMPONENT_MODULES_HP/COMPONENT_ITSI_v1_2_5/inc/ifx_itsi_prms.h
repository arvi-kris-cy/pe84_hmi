/******************************************************************************
* File Name: ifx_itsi_prms.h
*
* Description: This file contains ITSI prameter constants for Infineon ITSI header
*
*
*******************************************************************************
* (c) 2024, Infineon Technologies Company. All rights reserved.
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
* Include guard
*******************************************************************************/
#ifndef __IFX_ITSI_PRMS_H
#define __IFX_ITSI_PRMS_H

#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

//Please do not change these defined values
#define FRAME_SIZE_16K 160 // 10ms

#define MAX_WW_MODEL_SIZE   10000
#define MAX_CMD_MODEL_SIZE  100000
#define MAX_NMB_MODEL_SIZE 100000

#define AM_FRAME_SET ((int)(AM_FRAME_SIZE/FRAME_SIZE_16K))

#define FEATURE_BUF_SZ 38
#define FRAMES_HOP 4
#define N_PHONEMES 40
#define N_SEQ (23)

#define ITSI_SAMPLE_RATE (16000)

#ifndef SOD_LOOKBACK_S
#define SOD_LOOKBACK_S  (0.2f)
#endif
#ifndef SOD_LOOKBACK
#define SOD_LOOKBACK    ((int)((SOD_LOOKBACK_S*16000)/FRAME_SIZE_16K))
#endif

#define AM_LOOKBACK ((int)(SOD_LOOKBACK/ FRAMES_HOP))

#define AM_OUT_SZ_FOR_BUFFER (256)

#endif /* __IFX_ITSI_PRMS_H */
