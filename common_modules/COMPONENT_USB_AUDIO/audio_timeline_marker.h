/******************************************************************************
* File Name : audio_timeline_marker.h
*
* Description :
* Header file for USB based timeline marker.
********************************************************************************
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
*******************************************************************************/

#ifndef AUDIO_TIMELINE_MARKER_H_
#define AUDIO_TIMELINE_MARKER_H_


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/*******************************************************************************
 * Type definitions
 ******************************************************************************/
typedef enum
{
    CY_AUDIO_TIMELINE_INVALID,

    CY_AUDIO_TIMELINE_UPDATE_DUMMY,

    CY_AUDIO_TIMELINE_AAD_DETECTED,

    CY_AUDIO_TIMELINE_SOD_DETECTED,

    CY_AUDIO_TIMELINE_SOD_NOT_DETECTED,

    CY_AUDIO_TIMELINE_LPWWD_DETECTED,

    CY_AUDIO_TIMELINE_LPWWD_NOT_DETECTED,

    CY_AUDIO_TIMELINE_HPWWD_DETECTED,

    CY_AUDIO_TIMELINE_HPWWD_NOT_DETECTED,

    CY_AUDIO_TIMELINE_ASR_DETECTED,

    CY_AUDIO_TIMELINE_ASR_NOT_DETECTED,

    CY_AUDIO_TIMELINE_ASR_PROCESS_COMPLETED,

    CY_AUDIO_TIMELINE_CUSTOM_MARKER,

    CY_AUDIO_TIMELINE_ASR_FORCE_STAGE,

    CY_AUDIO_TIMELINE_MAX

} cy_audio_timeline_marker_t;

void cy_audio_timeline_marker_update_status(cy_audio_timeline_marker_t timeline,
                                            short *mono_10_ms_data_marker);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUDIO_TIMELINE_MARKER_H_ */
