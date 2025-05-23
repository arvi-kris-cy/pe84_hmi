/******************************************************************************
* File Name : tune_battery_powered_audio.c
*
* Description :
* Tune battery powered audio application.
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

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "tune_battery_powered_audio.h"

#include "app_logger.h"
#include "pdm_mic_interface.h"
#include "usb_audio_interface.h"
#include "tune_app_svc_lp.h"
#include "cycfg_pins.h"
#include "cybsp_types.h"
#include "audio_usb_send_utils.h"

#include "app_logger.h"

/*******************************************************************************
* Function Name: tune_battery_powered_audio
********************************************************************************
* Summary:
*  Initialize PDM mic, staged voice control, logging and USB interface
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

int tune_battery_powered_audio()
{
    
    app_core1_boot_log();

    app_log_print("Initializing USB interface \r\n");
    usb_audio_interface_init();
    usb_send_out_dbg_init_channels();

    /* Initialize Staged Voice Control */
    app_init_svc();
#ifdef ENABLE_MIC_INPUT_FEED
/*
 * If PDM mic is configured as the input for audio data, PDM mic is initialized here.
 * PDM mic data arrives via ISR. Refer to mic_pdm_isr() API for the data and flow.
 */
    cy_rslt_t result;
    result = pdm_mic_interface_init();
    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("PDM initialization failed - Reset the board \r\n");
        CY_ASSERT(0);
    }
    app_log_print("PDM Mic initialized \r\n");
#endif /* ENABLE_MIC_INPUT_FEED */
    return CY_RSLT_SUCCESS;
}
/* [] END OF FILE */
