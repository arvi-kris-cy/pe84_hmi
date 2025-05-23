/******************************************************************************
* File Name : afe_app_defines.h
*
* Description :
* Header file for AFE MW configuration.
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

#ifndef INCLUDED_AFE_APP_DEFINES_H
#define INCLUDED_AFE_APP_DEFINES_H

#include "cy_afe_configurator_settings.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NO_OF_BUFFERS                           (5)

#define AFE_FEED_DELAY                          (1)

#define FRAME_DATA_SIZE                         (64)

#define GENERATE_DUMMY_DATA                     (0)

/* Approx wave header length.
 * TODO : Currently it is approximately set for current file. Need to calculate header length */
#define WAV_HEADER_LENGTH                       (100)

#define NO_OF_CHANNELS_RECEIVED                 (AFE_INPUT_NUMBER_CHANNELS)
#define NO_OF_BYTES_PER_SAMPLE                  (2)
#define MONO_AUDIO_DATA_IN_BYTES                (320)
#define STEREO_AUDIO_DATA_IN_BYTES              (640)

#ifdef __cplusplus
} 
#endif /* __cplusplus */
#endif /* INCLUDED_AFE_APP_DEFINES_H */
