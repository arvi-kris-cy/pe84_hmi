/******************************************************************************
* File Name : mains_powered_control.c
*
* Description :
* Source file for mains powered local voice control task.
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
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "cybsp.h"
#include "cybsp_types.h"

#include "mains_powered_control.h"
#include "mains_powered_inferencing.h"

#include "mains_powered_music_player.h"

#ifdef AUDIO_OUT
#include "i2s_playback.h"
#endif /* AUDIO_OUT */
#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define MAP_ID_QUEUE_LENGTH                                 (2u)
#define MAINS_POWERED_CONTROL_TASK_PRIORITY                 (3u)
#define MAINS_POWERED_CONTROL_TASK_STACK_SIZE               (256u)
#define ASR_LED_GLOW_DELAY                                  (1000)


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void mains_powered_control_task(void* pvParameters);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Handle of the queue holding the map_id. */
QueueHandle_t map_id_q;
/* Control task handle */
TaskHandle_t control_task_handle;
/* Indicates the inferencing stage */
int8_t asr_flag=0;

/*******************************************************************************
* Function Name: mains_powered_control_task_init
********************************************************************************
* Summary:
* Initialize the control task for doing post processing.
*
* Parameters:
* None
*
* Return:
*  None
*
*******************************************************************************/
void mains_powered_control_task_init(void)
{
    BaseType_t status;
    status = xTaskCreate(mains_powered_control_task, "app_control_task",
                    MAINS_POWERED_CONTROL_TASK_STACK_SIZE, NULL, 
                    MAINS_POWERED_CONTROL_TASK_PRIORITY,
                    &control_task_handle);
    if (pdPASS != status)
    {
        app_log_print("Error in battery powered local voice control task \r\n");
    }
}
 /*******************************************************************************
 * Function Name: mains_powered_control_task
 ********************************************************************************
 * Summary:
 *
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *******************************************************************************/
void mains_powered_control_task(void *arg)
{
    int32_t map_id;

    /* Create a message queue to communicate map_id between tasks. */
    map_id_q = xQueueCreate(MAP_ID_QUEUE_LENGTH, sizeof(int32_t));

#ifdef AUDIO_OUT
    music_player_q_data_t music_player_q_data;
#endif /* AUDIO_OUT */

    while (1)
    {
        if (pdTRUE == xQueueReceive(map_id_q, &map_id, portMAX_DELAY))
        {
            int8_t i2s_control_flag = I2S_PLAYBACK_DEFAULT_STATE;
            switch (map_id)
            {
                case OK_INFINEON_CMD_ID:
                {
                    asr_flag=1;
#ifdef AUDIO_OUT
            i2s_control_flag=I2S_PLAYBACK_WWD;

            music_player_q_data.data = NULL;
            music_player_q_data.data_len = 0;
            music_player_q_data.cmd = i2s_control_flag;

            if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, 0))
            {
                app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
            }
#endif /* AUDIO_OUT */
                    app_log_print("Wake Word detected on High Performance Core CM55 \r\n");
                    Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
                    Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_ON);

                    app_log_print("Waiting for ASR command \r\n");
                    break;
                }
                case PLAY_MUSIC_CMD_ID:
                case STOP_MUSIC_CMD_ID:
                case NEXT_TRACK_CMD_ID:
                case PREVIOUS_TRACK_CMD_ID:
                case INCREASE_VOL_CMD_ID:
                case DECREASE_VOL_CMD_ID:
                {
                    asr_flag=2;
                    app_log_print("ASR command detected \r\n");
                    if (map_id==PLAY_MUSIC_CMD_ID)
                    {
                        app_log_print("Play Music \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_PLAY_MUSIC;
                        asr_flag=3;
#endif /* AUDIO_OUT */
                    }
                    if (map_id==STOP_MUSIC_CMD_ID)
                    {
                        app_log_print("Stop Music \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_STOP_MUSIC;
#endif /* AUDIO_OUT */
                    }
                    if (map_id==NEXT_TRACK_CMD_ID)
                    {
                        app_log_print("Next Track \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_NEXT_TRACK;
                        asr_flag=3;
#endif /* AUDIO_OUT */
                    }
                    if (map_id==PREVIOUS_TRACK_CMD_ID)
                    {
                        app_log_print("Previous Track \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_PREV_TRACK;
                        asr_flag=3;
#endif /* AUDIO_OUT */
                    }
                    if (map_id==INCREASE_VOL_CMD_ID)
                    {
                        app_log_print("Increase Volume \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_INC_VOL;
#endif /* AUDIO_OUT */
                    }
                    if (map_id==DECREASE_VOL_CMD_ID)
                    {
                        app_log_print("Decrease Volume \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag=I2S_PLAYBACK_DEC_VOL;
#endif /* AUDIO_OUT */
                    }


                    Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_ON);



#ifdef AUDIO_OUT
                    music_player_q_data.data = NULL;
                    music_player_q_data.data_len = 0;
                    music_player_q_data.cmd = i2s_control_flag;

                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, 0))
                    {
                        app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
                    }

                    if (!is_music_player_active() && !is_music_player_paused())
                    {
                        vTaskDelay(ASR_LED_GLOW_DELAY);
                    }
                    if (i2s_control_flag!=I2S_PLAYBACK_PLAY_MUSIC)
                    {
                        if (!is_music_player_active())
                        {
                            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                            app_log_print("ASR processing done \r\n");
                        }
                    }
#endif /* AUDIO_OUT */
                    app_log_print("Waiting for Wake Word \r\n");
                    break;
                }

                case INFERENCING_TIMEOUT:
                {
#ifdef AUDIO_OUT
                    if (is_music_player_active() || is_music_player_paused())
                    {
                        i2s_control_flag=I2S_PLAYBACK_RESUME_MUSIC;

                        music_player_q_data.data = NULL;
                        music_player_q_data.data_len = 0;
                        music_player_q_data.cmd = i2s_control_flag;

                        if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, 0))
                        {
                            app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
                        }
                    }
                    else
                    {
                        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
                    }
#else
                        Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
                        Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
#endif /* AUDIO_OUT */
                    break;
                }

                default:
                {
                    app_log_print("Unknown command: %d",map_id);
                    break;
                }
            }
        }
    }
}
/* [] END OF FILE */
