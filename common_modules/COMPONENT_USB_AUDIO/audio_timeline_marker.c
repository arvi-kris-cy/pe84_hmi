/******************************************************************************
* File Name : audio_timeline_marker.c
*
* Description :
* USB audio class based audio timeline marker that captures the state transitions
* in audio pipeline when wakeword or ASR is detected.
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

#include "audio_receive_task.h"
#include "audio_send_task.h"
#include "cy_pdl.h"
#include "cybsp.h"
#include "cycfg.h"
#include "rtos.h"
#include "cy_retarget_io.h"
#include "audio_timeline_marker.h"

#include "audio_usb_send_utils.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define MONO_10MS_SAMPLE                    (160)
/*******************************************************************************
* Global Variables
*******************************************************************************/

short g_timeline_markers[CY_AUDIO_TIMELINE_MAX] =
{
        0,   //  CY_AUDIO_TIMELINE_INVALID,
        0,   //  CY_AUDIO_TIMELINE_UPDATE_DUMMY,
        1000, // CY_AUDIO_TIMELINE_AAD_DETECTED,
        2500,   // CY_AUDIO_TIMELINE_SOD_DETECTED,
        -2500,   //  CY_AUDIO_TIMELINE_SOD_NOT_DETECTED,
        5000,   //CY_AUDIO_TIMELINE_LPWWD_DETECTED,
        -5000,   //CY_AUDIO_TIMELINE_LPWWD_NOT_DETECTED,
        10000,   //CY_AUDIO_TIMELINE_HPWWD_DETECTED,
        -10000,   // CY_AUDIO_TIMELINE_HPWWD_NOT_DETECTED,
        15000,  // CY_AUDIO_TIMELINE_ASR_DETECTED,
        -15000,  //CY_AUDIO_TIMELINE_ASR_NOT_DETECTED,
        20000,  //CY_AUDIO_TIMELINE_PROCESS_COMPLETED,
        -1,   // CY_AUDIO_TIMELINE_CUSTOM_MARKER,
        30000  //CY_AUDIO_TIMELINE_ASR_FORCE_STAGE
};

short audio_timleine_mono_10ms_sample[MONO_10MS_SAMPLE] = {0};

/*******************************************************************************
* Function Name: cy_audio_timeline_marker_update_status
********************************************************************************
* Summary:
* Depending on the marker, send the data via USB to PC.
*
* Parameters:
*  timeline - timeline marker
*  mono_10_ms_data_marker - The data to be sent over USB.
* Return:
*  None
*
*******************************************************************************/

void cy_audio_timeline_marker_update_status(cy_audio_timeline_marker_t timeline,
                                            short *mono_10_ms_data_marker)
{
    if (CY_AUDIO_TIMELINE_CUSTOM_MARKER == timeline)
    {
        memcpy(audio_timleine_mono_10ms_sample, mono_10_ms_data_marker,
                sizeof(audio_timleine_mono_10ms_sample));
    }
    else
    {
        for (int i = 0; i < MONO_10MS_SAMPLE; i++) 
        {
            audio_timleine_mono_10ms_sample[i] = g_timeline_markers[timeline];
        }
    }

    usb_send_out_dbg_put(USB_CHANNEL_2, audio_timleine_mono_10ms_sample);

    return;
}

