/******************************************************************************
* File Name : battery_powered_control.c
*
* Description :
* Source file for battery powered local voice control task.
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

#include "cy_staged_voice_control.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_control.h"
#include "inferencing_interface.h"
#include "app_logger.h"
#ifdef AUDIO_OUT
#include "battery_powered_music_player.h"
#include "i2s_playback.h"
#endif /* AUDIO_OUT */
/*******************************************************************************
* Macros
*******************************************************************************/
#define MAP_ID_QUEUE_LENGTH                         (2u)
#define BATTERY_POWERED_CONTROL_TASK_PRIORITY       (3u)
#define BATTERY_POWERED_CONTROL_TASK_STACK_SIZE     (256u)

#define ASR_LED_GLOW_DELAY                          (1000u)

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void battery_powered_control_task(void* pvParameters);

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Handle of the queue holding the map_id. */
QueueHandle_t map_id_q;
/* Control task handle */
TaskHandle_t control_task_handle;

/* Streaming state request in case of playback */
cy_svc_hp_set_state_asr_detected_info_t asr_detected_state_info = {0};

extern bool enabled_streaming_state;
extern volatile bool ptt_flag;
extern int8_t play_flag;

/*******************************************************************************
* Function Name: battery_powered_control_task_init
********************************************************************************
* Summary:
* Initialize the control task to do post processing and send notifications
*
* Parameters:
* None
*
* Return:
* None
*
*******************************************************************************/
void battery_powered_control_task_init(void)
{
    BaseType_t status;
    status = xTaskCreate(battery_powered_control_task, "app_control_task",
                    BATTERY_POWERED_CONTROL_TASK_STACK_SIZE, NULL, 
                    BATTERY_POWERED_CONTROL_TASK_PRIORITY,
                    &control_task_handle);
    if (pdPASS != status)
    {
        app_log_print("Error in battery powered local voice control task \r\n");
    }
}
 /*******************************************************************************
 * Function Name: battery_powered_control_task
 ********************************************************************************
 * Summary:
 * Control task to do post processing and send notifications
 *
 * Parameters:
 *  None
 *
 * Return:
 *  None
 *******************************************************************************/
void battery_powered_control_task(void *arg)
{
    int32_t map_id;

    cy_svc_stage_t svc_stage = CY_SVC_STAGE_INVALID;
    unsigned int current_feed_counter_stage1 = 0;
    unsigned int current_feed_counter_stage2 = 0;
    asr_detected_state_info.action = 0;

#ifdef AUDIO_OUT
    int8_t i2s_control_flag = I2S_PLAYBACK_DEFAULT_STATE;
    /* Send the ASR commands or timeout over q to music player task */
    music_player_q_data_t music_player_q_data;
#endif /* AUDIO_OUT */

    /* Create a message queue to communicate map_id between tasks. */
    map_id_q = xQueueCreate(MAP_ID_QUEUE_LENGTH, sizeof(int32_t));

    while (1)
    {
        if (pdTRUE == xQueueReceive(map_id_q, &map_id, portMAX_DELAY))
        {
            get_inferencing_frame_counters(&current_feed_counter_stage1,&current_feed_counter_stage2);
            switch (map_id)
            {
                case OK_INFINEON_CMD_ID:
                {
#ifdef AUDIO_OUT
                    music_player_q_data.data = NULL;
                    music_player_q_data.data_len = 0;
                    i2s_control_flag=I2S_PLAYBACK_WWD;
                    music_player_q_data.cmd = i2s_control_flag;
                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                    {
                        app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
                    }
#endif /* AUDIO_OUT */
                    cy_svc_hp_get_current_stage(&svc_stage);

                    if (svc_stage == CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT && ptt_flag == false)
                    {
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
                        switch_to_wwd_inferencing();
                    }
                    else if (svc_stage == CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT && ptt_flag == true)
                    {
                        app_log_print("Speech detected on High Performance Core CM55 for Push-To-Talk\r\n");

                    }
                    if(false == enabled_streaming_state && ptt_flag == false)
                    {
                        app_log_print("Wake Word detected in High Performance Core CM55 \r\n");
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
                        app_inferencing_enable_streaming_state();
                    }
                    else if (false == enabled_streaming_state && ptt_flag == true)
                    {
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED, NULL);
                        app_inferencing_enable_streaming_state();
                    }
                    else
                    {
                        app_log_print("Wake Word detected during audio playback \r\n");
                        app_inferencing_extend_streaming_state();
                    }
#ifdef AUDIO_OUT
                    if (is_music_player_active() || is_music_player_paused())
                    {
                        switch_to_asr_inferencing_streaming();
                    }
                    else
                    {
                        switch_to_asr_inferencing();
                    }
#else
                    switch_to_asr_inferencing();
#endif

                    break;
                }
                case PLAY_MUSIC_CMD_ID:
                case STOP_MUSIC_CMD_ID:
                case NEXT_TRACK_CMD_ID:
                case PREVIOUS_TRACK_CMD_ID:
                case INCREASE_VOL_CMD_ID:
                case DECREASE_VOL_CMD_ID:
                {
                    app_log_print("ASR command detected \r\n");
                    if (map_id == PLAY_MUSIC_CMD_ID)
                    {
                        app_log_print("Play Music \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_PLAY_MUSIC;
#endif /* AUDIO_OUT */
                    }
                    if (map_id == STOP_MUSIC_CMD_ID)
                    {
                        app_log_print("Stop Music \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_STOP_MUSIC;
#endif /* AUDIO_OUT */
                    }
                    if (map_id == NEXT_TRACK_CMD_ID)
                    {
                        app_log_print("Next Track \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_NEXT_TRACK;
#endif /* AUDIO_OUT */
                    }
                    if (map_id == PREVIOUS_TRACK_CMD_ID)
                    {
                        app_log_print("Previous Track \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_PREV_TRACK;
#endif /* AUDIO_OUT */
                    }
                    if (map_id == INCREASE_VOL_CMD_ID)
                    {
                        app_log_print("Increase Volume \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_INC_VOL;
#endif /* AUDIO_OUT */
                    }
                    if (map_id == DECREASE_VOL_CMD_ID)
                    {
                        app_log_print("Decrease Volume \r\n");
#ifdef AUDIO_OUT
                        i2s_control_flag = I2S_PLAYBACK_DEC_VOL;
#endif /* AUDIO_OUT */
                    }
#ifdef AUDIO_OUT
                    music_player_q_data.cmd = i2s_control_flag;
                    music_player_q_data.data = NULL;
                    music_player_q_data.data_len = 0;

                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                    {
                        app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
                    }
#endif /* AUDIO_OUT */


                    if(false == enabled_streaming_state || ptt_flag == true )
                    {
                        /**
                         * Streaming is enabled first time.
                         */
                        asr_detected_state_info.action = CY_SVC_STREAM_DATA_REQUEST;
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_DETECTED, &asr_detected_state_info);
                    }



#ifdef AUDIO_OUT
                    if ((!is_music_player_active() && !is_music_player_paused()) || i2s_control_flag == I2S_PLAYBACK_STOP_MUSIC)
                    {
                        vTaskDelay(ASR_LED_GLOW_DELAY);
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
                        if (ptt_flag == true)
                        {
                            ptt_flag = false;
                        }
                        switch_to_wwd_inferencing();
                    }
                    else if (is_music_player_active() || is_music_player_paused() || i2s_control_flag == I2S_PLAYBACK_PLAY_MUSIC )
                    {
                        if (ptt_flag == false)
                        {
                            app_log_print("Waiting for Wake Word \r\n");
                            switch_to_wwd_inferencing_streaming();
                        } else
                        if (ptt_flag == true)
                        {
                            app_log_print("Waiting for Wake Word \r\n");
                            switch_to_wwd_inferencing_streaming();
                            ptt_flag = false;
                        }
                        else
                        {
                        //    cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
                        //    switch_to_wwd_inferencing();
                        }
                    }
#endif /* AUDIO_OUT */
                    break;
                }
                case INFERENCING_TIMEOUT:
                {
                    cy_svc_hp_get_current_stage(&svc_stage);
                    if(CY_SVC_STAGE_WAITING_FOR_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTION == svc_stage)
                    {
                        app_log_print("Wake Word not detected on High Performance Core CM55 - Timeout \r\n");
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED, NULL);
                    }
                    else if (CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT == svc_stage)
                    {
                        app_log_print("ASR command not detected on High Performance Core CM55 \r\n");
                        //if (ptt_flag==false)
                        {
                            cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
                        }
                    }
                    else
                    {
                        app_log_print("Invalid Error State: %d!!", svc_stage);
                        cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_NOT_DETECTED, NULL);
                    }
                    if (ptt_flag == true)
                    {
                        ptt_flag = false;
                    }
                    switch_to_wwd_inferencing();

                    break;
                }
                default:
                {
                    app_log_print("Unknown command: %ld",(long)map_id);

                    /*PTT*/
                    cy_svc_hp_get_current_stage(&svc_stage);
                    app_log_print("Unknown command at stage %d \r\n",svc_stage);
                    break;
                }
            }
        }
    }
} 
/* [] END OF FILE */
