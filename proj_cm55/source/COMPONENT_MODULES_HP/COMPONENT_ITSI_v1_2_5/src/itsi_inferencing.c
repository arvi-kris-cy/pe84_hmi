/******************************************************************************
* File Name : itsi_inferencing.c
*
* Description :
* Source file for ITSI post processing.
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
#include "itsi_inferencing.h"
#include "ifx_sp_common.h"
#include <string.h>
#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define COMMAND_LENGTH             (250)
#define NUM_INDICES                (2)
#define NUMBER_LENGTH              (20)

/*******************************************************************************
* Global Variables
*******************************************************************************/
extern void* dfcmd_obj;
char wwd_detected_flag = 0;
char cmd_not_detected_flag = 0;
char ptt_status_flag =0;

/*******************************************************************************
* Function Name: itsi_process_results
********************************************************************************
* Summary:
*    Process ITSI results
*
*******************************************************************************/

void itsi_process_results(int wwd_result, int cmd_result)
{
    char commandtext[COMMAND_LENGTH] = {0};
    int indices[NUM_INDICES], Nindices, Nnumbers;
    char number[3][NUMBER_LENGTH];
    char units[3][NUMBER_LENGTH];
    int status = 0;

#ifdef ENABLE_PUSH_TO_TALK
    if (ptt_flag && ptt_status_flag==0)
    {
        ptt_status_flag=1;
        app_log_print("Speech detected for Push-To-Talk \r\n");
        itsi_command_to_id(INF_WAKE_WORD);
    }
#endif /* ENABLE_PUSH_TO_TALK */
    if (wwd_result == 1)
    {
        wwd_detected_flag = 1;
        cmd_not_detected_flag=1;
#ifndef BATTERY_POWERED_LOCAL_VOICE
        if (ptt_flag==true)
        {
            app_log_print("Speech detected for Push-To-Talk \r\n");
        }
        ptt_flag=false;
        ptt_status_flag=0;
#endif /* BATTERY_POWERED_LOCAL_VOICE */
#ifndef ENABLE_PUSH_TO_TALK
        itsi_command_to_id(INF_WAKE_WORD);
#endif /* ENABLE_PUSH_TO_TALK */
        return;
    }

    if (wwd_result == 0 && cmd_result == -2)
    {
        if (wwd_detected_flag && cmd_not_detected_flag)
        {
            wwd_detected_flag = 0;
#ifndef BATTERY_POWERED_LOCAL_VOICE
            app_log_print("ASR command not detected on High Performance Core CM55 \r\n");
            ptt_flag=false;
            ptt_status_flag=0;
#ifndef ENABLE_PUSH_TO_TALK
            app_log_print("Waiting for Wake Word \r\n");
#endif /* ENABLE_PUSH_TO_TALK */

#endif /* BATTERY_POWERED_LOCAL_VOICE */
            itsi_command_to_id(INF_TIMEOUT);
        }
    }
    if (cmd_result == 1)
    {
        status = ifx_get_command(dfcmd_obj, commandtext);
        if (status)
        {
            app_log_print("Error! ITSI get command Failed!! Error code=%x, Component index=%d, Line number=%d, exiting ITSI!\r\n",
                IFX_SP_ENH_ERR_CODE(status), IFX_SP_ENH_ERR_COMPONENT_INDEX(status), IFX_SP_ENH_ERR_LINE_NUMBER(status));
            return;
        }
        cmd_not_detected_flag=0;
#ifndef BATTERY_POWERED_LOCAL_VOICE
        ptt_flag=false;
        ptt_status_flag=0;
#endif /* BATTERY_POWERED_LOCAL_VOICE */
        itsi_command_to_id(commandtext);
        status = ifx_get_command_indices_and_numbers(dfcmd_obj, indices, &Nindices, number,&Nnumbers, units, &Nnumbers);

        if (status)
        {
            app_log_print("Error! ITSI get command indices Failed!! Error code=%x, Component index=%d, Line number=%d, exiting ITSI!\r\n",
                IFX_SP_ENH_ERR_CODE(status), IFX_SP_ENH_ERR_COMPONENT_INDEX(status), IFX_SP_ENH_ERR_LINE_NUMBER(status));
            return;
        }
    }

}
/*******************************************************************************
* Function Name: itsi_command_to_id
********************************************************************************
* Summary:
*    Converts command to map id
*
*******************************************************************************/
void itsi_command_to_id (char *command)
{
    int map_id=0;

    if (!strcmp(command,INF_WAKE_WORD))
    {
        map_id = WAKEWORD_CMD_ID;
    }
    else if (!strcmp(command,INF_PLAY_MUSIC))
    {
        map_id = PLAY_MUSIC_CMD_ID;
    }
    else if (!strcmp(command,INF_STOP_MUSIC))
    {
        map_id = STOP_MUSIC_CMD_ID;
    }
    else if (!strcmp(command,INF_INCREASE_VOLUME))
    {
        map_id = INCREASE_VOL_CMD_ID;
    }
    else if (!strcmp(command,INF_DECREASE_VOLUME))
    {
        map_id = DECREASE_VOL_CMD_ID;
    }
    else if (!strcmp(command,INF_NEXT_TRACK))
    {
        map_id = NEXT_TRACK_CMD_ID;
    }
    else if (!strcmp(command,INF_PREVIOUS_TRACK))
    {
        map_id = PREVIOUS_TRACK_CMD_ID;
    }
    
    else if (!strcmp(command, INF_TIMEOUT))
    {
        map_id = INFERENCING_TIMEOUT;
    }

    
#ifdef BATTERY_POWERED_LOCAL_VOICE
    battery_powered_post_processing(map_id);
#elif defined (MAINS_POWERED_LOCAL_VOICE) || defined (DEBUG_TUNE_MAINS_APP)
    mains_powered_post_processing(map_id);
#endif /* BATTERY_POWERED_LOCAL_VOICE/MAINS_POWERED_LOCAL_VOICE */

}


