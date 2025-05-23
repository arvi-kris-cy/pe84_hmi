/******************************************************************************
* File Name : mains_powered_app_hp.c
*
* Description :
* Audio application for CM55 core (mains powered use-case).
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
#include "cycfg_pins.h"
#include "cybsp_types.h"
#include "mains_powered_inferencing.h"
#include "mains_powered_afe.h"
#include "mains_powered_app_hp.h"
#include "pdm_mic_interface.h"
#include "mains_powered_control.h"
#include "mains_powered_music_player.h"

#ifdef AUDIO_OUT
#include "i2s_playback.h"
#endif /* AUDIO_OUT */

/*******************************************************************************
* Function Name: mains_powered_app_hp
********************************************************************************
* Summary:
* Initialize CM55 audio application for mains powered use-case.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void mains_powered_app_hp()
{
    cy_rslt_t result;

    app_core2_boot_log();
    mains_powered_inference_engine_init();
    mains_powered_afe_mw_init();
    mains_powered_control_task_init();

#ifdef AUDIO_OUT
    create_music_player_task();
    create_i2s_task();
#endif /* AUDIO_OUT */

#ifdef ITSI_INFERENCING
    app_push_to_talk_cm55_init(app_user_action_cb);
#endif /* ITSI_INFERENCING */
    app_log_print("\x1b[2J\x1b[;H");

    app_log_print("****************** \
    Mains Powered Local Voice Code Example \
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

#if !defined(ITSI_INFERENCING) && !defined(SENSORY_INFERENCING)
    app_log_print("Important: Inferencing Code unavailable - Enable it for CE to work with all functionality. \r\n");
#else
    app_log_print("Waiting for Wake Word \r\n");
#endif /* ITSI_INFERENCING */

/* PDM mic initialization.
 * PDM mic data arrives via ISR. Refer to mic_pdm_isr() API for the data and flow.
 */
    result = pdm_mic_interface_init();
    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("PDM initialization failed - Reset the board \r\n");
        CY_ASSERT(0);
    }

}

/*******************************************************************************
* Function Name: app_push_to_talk_cm55_init
********************************************************************************
* Summary:
* Initialize user button and pass the callback.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_push_to_talk_cm55_init(cb_user_action user_action_cb)
{
    user_interaction_init(USER_INTERACTION_BUTTON, user_action_cb);
}

/*******************************************************************************
* Function Name: app_user_action_cb
********************************************************************************
* Summary:
* Callback for user button - Push to Talk - Goes to ASR detection state.
*
* Parameters:
*  action - User button action.
*
* Return:
*  None
*
*******************************************************************************/

void app_user_action_cb(void)
{
    app_log_print("Push-To-Talk : User button pressed - Waiting for ASR command \r\n");
    ptt_flag = true;
}
/* [] END OF FILE */
