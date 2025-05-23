/******************************************************************************
* File Name : battery_powered_app_hp.c
*
* Description :
* Battery Powered Audio application for CM55 core.
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

#include "app_logger.h"
#include "FreeRTOS.h"
#include "cy_syslib.h"
#include "task.h"
#include "timers.h"
#include "battery_powered_svc_hp.h"
#ifdef AUDIO_OUT
#include "i2s_playback.h"
#include "battery_powered_music_player.h"
#endif /* AUDIO_OUT */
#include "battery_powered_app_hp.h"
#include "battery_powered_control.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_afe.h"
#include "intercore_ipc.h"




/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

#ifdef INTERCORE_PIPE
extern void app_ipc_init_cm55(void);
#endif /* INTERCORE_PIPE */


/*******************************************************************************
* Function Name: battery_powered_app_hp
********************************************************************************
* Summary:
* Initialize CM55 audio application for battery powered use-case.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/


void battery_powered_app_hp()
{
    app_core2_boot_log();

#ifdef INTERCORE_PIPE
    app_ipc_init_cm55();
#endif /* INTERCORE_PIPE */

    battery_powered_inference_engine_init();

    battery_powered_afe_mw_init();
#if defined(ITSI_INFERENCING) || defined(SENSORY_INFERENCING)
    app_svc_hp_app_init();
#endif /* ITSI_INFERENCING */
    battery_powered_control_task_init();
#ifdef AUDIO_OUT
    create_music_player_task();
    create_i2s_task();
#endif /* AUDIO_OUT */   
    app_log_print("\x1b[2J\x1b[;H");

    app_log_print("****************** \
    Battery Powered Local Voice Code Example with ITSI \
    ****************** \r\n\n");

    app_log_print("Music Player Application \r\n");
    app_log_print("Wake word: Okay Infineon\r\n"
           "Commands:\r\n"
           "    1. Play music\r\n"
           "    2. Next track\r\n"
           "    3. Previous track\r\n"
           "    4. Stop music\r\n"
           "    5. Increase Volume\r\n"
           "    6. Decrease Volume\r\n");

#if !defined(ITSI_INFERENCING)
    app_log_print("Important: ITSI Inferencing Code unavailable - Enable it for CE to work with all functionality. \r\n");
#else
    app_log_print("Waiting for Wake Word \r\n");
#endif /* ITSI_INFERENCING */


}


/* [] END OF FILE */
