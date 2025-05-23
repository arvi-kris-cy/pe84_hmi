/******************************************************************************
* File Name : battery_powered_app_lp.c
*
* Description :
* CM33 Non Secure application for staged voice control enabled audio pipeline.
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
#include "battery_powered_app_lp.h"

#include "app_logger.h"
#include "pdm_mic_interface.h"
#include "battery_powered_svc_lp.h"
#include "cy_retarget_io.h"
#include "cycfg_pins.h"
#include "cybsp_types.h"


/*******************************************************************************
* Function Name: battery_powered_app_lp
********************************************************************************
* Summary:
*  Initialize PDM mic, staged voice control, logging for battery powered application.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_app_lp()
{
    
    cy_rslt_t result;
/* Prints CM33 non-secure boot-time logs */
    app_core1_boot_log();

    /* Initialize Staged Voice Control */
    app_init_svc();

/*
 * If PDM mic is configured as the input for audio data, PDM mic is initialized here.
 * PDM mic data arrives via ISR. Refer to mic_pdm_isr() API for the data and flow.
 */
    result = pdm_mic_interface_init();
    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("PDM initialization failed - Reset the board \r\n");
        CY_ASSERT(0);
    }

    return;
}

/* [] END OF FILE */
