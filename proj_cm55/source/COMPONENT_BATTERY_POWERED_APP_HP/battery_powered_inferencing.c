/******************************************************************************
* File Name : battery_powered_inferencing.c
*
* Description :
* Source file for Inferencing and post processing on CM55 . (battery use-case)
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
#include "cycfg.h"
#include "cy_log.h"
#include "cyabs_rtos.h"
#include "cybsp_types.h"
#include "battery_powered_svc_hp.h"
#include "cy_staged_voice_control.h"
#include "battery_powered_control.h"
#include "battery_powered_inferencing.h"
#ifdef AUDIO_OUT
#include "i2s_playback.h"
#include "battery_powered_music_player.h"
#endif /* AUDIO_OUT */

#include "app_logger.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define MAX_SIMULATE_POST_WWD_REQ_FRAME_COUNT           (150)
#define MAX_AUD_SAMPLES_PER_CHANNEL_FOR_10MS_DATA       (160)
#define STREAMING_STATE_TIMEOUT_FRAME_COUNTER           (1000)
#define WWD_STAGE                                       (1)
#define ASR_STAGE                                       (2)

#define INFERENCING_LOG_CONTROL                         (0)

#if INFERENCING_LOG_CONTROL
#define hp_app_infer_log(format,...)                    printf(format "\r\n",##__VA_ARGS__);
#else
#define hp_app_infer_log(format,...)
#endif /* INFERENCING_LOG_CONTROL */

#define INFERENCING_TASK_PRIORITY                       (3)
#define INFER_QUEUE_SIZE                                (320)
#define INFER_QUEUE_ELEMENTS                            (30)
#define INFER_FRAME_SIZE                                (320)
#define INFERENCING_STACK_SIZE                          (1024*10)
/****************************************************************************
* Global variables
*****************************************************************************/
volatile bool ptt_flag = false;

bool enable_stage1_timeout = true;
bool enable_stage2_timeout = true;

cy_svc_stage_t g_cur_stage = CY_SVC_STAGE_INVALID;

unsigned int ui_frame_counter_on_streaming_state = 0;
bool enabled_streaming_state = false;
extern QueueHandle_t map_id_q;

TaskHandle_t rtos_infer_task;
QueueHandle_t infer_queue_handle;

extern QueueHandle_t app_buf_info_queue;

/****************************************************************************
* Functions Prototypes
*****************************************************************************/
extern void svc_hp_app_print_statistics();

/*******************************************************************************
* Function Name: app_request_post_wwd_data
********************************************************************************
* Summary:
* Request post WWD data.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_request_post_wwd_data( )
{
    cy_svc_hp_set_state_hpwwd_det_in_prog_info_t hpwwd_det_state_info = { 0 };

    hpwwd_det_state_info.action = CY_SVC_SEND_POST_WWD_BUFFER;
    hpwwd_det_state_info.post_wwd_frame_count = 50;

    app_log_print("Requesting for more data from CM33 for Wake Word inferencing\r\n");
    cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_DETECT_IN_PROGRESS,
            &hpwwd_det_state_info);

    return;
}

/*******************************************************************************
* Function Name: app_inferencing_extend_streaming_state
********************************************************************************
* Summary:
* Extend streaming state in Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_inferencing_extend_streaming_state()
{
    ui_frame_counter_on_streaming_state = 0;

    return;
}
/*******************************************************************************
* Function Name: app_inferencing_disable_streaming_state
********************************************************************************
* Summary:
* Disable streaming state in Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void app_inferencing_disable_streaming_state()
{

    ui_frame_counter_on_streaming_state = 0;
    enabled_streaming_state = false;
    enable_stage1_timeout = true;
    enable_stage2_timeout = true;
    inference_reset_wwd_feed_timeout_counter();

    return;
}

/*******************************************************************************
* Function Name: app_inferencing_enable_streaming_state
********************************************************************************
* Summary:
* Enable streaming state in Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_inferencing_enable_streaming_state()
{

    ui_frame_counter_on_streaming_state = 0;
    enabled_streaming_state = true;
    enable_stage1_timeout = false;
    enable_stage2_timeout = false;
    return;
}

/*******************************************************************************
* Function Name: app_inferencing_get_streaming_state
********************************************************************************
* Summary:
* Get the streaming state of application during ASR processing
*
* Parameters:
*  None
*
* Return:
*  true: Application streaming state
*  false: Application non-streaming state
*
*******************************************************************************/

int app_inferencing_get_streaming_state()
{
    return enabled_streaming_state;
}
/*******************************************************************************
* Function Name: app_inferencing_streaming_state_timeout
********************************************************************************
* Summary:
* Timeout for streaming state. Changes the state of Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_inferencing_streaming_state_timeout()
{
    cy_svc_hp_set_state_asr_detected_info_t asr_state_for_ptt = {0};

    if (true == enabled_streaming_state)
    {
        ui_frame_counter_on_streaming_state++;
    }

    if (ui_frame_counter_on_streaming_state >= STREAMING_STATE_TIMEOUT_FRAME_COUNTER)
    {
        if  ((!is_music_player_active() && !is_music_player_paused()) && (!enabled_streaming_state))
        {
            app_log_print("ASR not detected - timeout \r\n");
            cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_PROCESSING_COMPLETED, NULL);
            app_inferencing_disable_streaming_state();
            switch_model_to_wwd();
            switch_to_wwd_inferencing();
        } 
        else 
        {
            if (ASR_STAGE == get_current_inference_stage())
            {
                app_log_print("ASR not detected - timeout \r\n");
                app_log_print("Waiting for Wake Word \r\n");
                switch_model_to_wwd();
                switch_to_wwd_inferencing_streaming();
                if (ptt_flag == true)
                {
                    asr_state_for_ptt.action = CY_SVC_STREAM_DATA_REQUEST;
                    cy_svc_hp_set_state(CY_SVC_SET_STATE_ASR_DETECTED, &asr_state_for_ptt);
                    ptt_flag=false;
                }

                int8_t i2s_control_flag = I2S_PLAYBACK_RESUME_MUSIC;
                music_player_q_data_t music_player_q_data;
                music_player_q_data.data = NULL;
                music_player_q_data.data_len = 0;
                music_player_q_data.cmd = i2s_control_flag;
                if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                {
                    app_log_print(">>> Error: I2S Control Queue Send failed*******************\r\n");
                }
            }
            ui_frame_counter_on_streaming_state=0;
        }
    }

    return;
}

/*******************************************************************************
* Function Name: app_inferencing_callback
********************************************************************************
* Summary:
* Callback function for inferencing library.
* This function is used to print outputs from inferencing library API.
*
* Parameters:
*  function - Function from which callback is called.
*  message - Message to print.
*  parameter - Parameter passed in callback from inferencing library API.
*
* Return:
*  None
*
*******************************************************************************/

void app_inferencing_callback(const char *function, char *message, char *parameter)
{
    if (!strcmp(message,"Timeout"))
    {

        if (is_music_player_active() || is_music_player_paused())
        {
            app_log_print("Ignoring timeout in play-state \r\n");
        }
        else
        {
            app_log_print("Wake Word not detected in High Performance Core CM55-Time out \r\n");
            cy_svc_hp_set_state(CY_SVC_SET_STATE_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED, NULL);
        }

    }
    return;
}

/*******************************************************************************
* Function Name: battery_powered_post_processing
********************************************************************************
* Summary:
* Post Processing API for Inference outputs.
* Based on the inferred map id, the necessary action will be taken.
*
* Parameters:
*  map_id - Inferred Map ID.
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_post_processing(int map_id)
 {
    BaseType_t ret = pdTRUE;
/* Switch the model groups before continuing the inferencing */
    switch (map_id)
    {
        case OK_INFINEON_CMD_ID:
        {
            switch_model_to_asr();
            break;
        }
        case PLAY_MUSIC_CMD_ID:
        case STOP_MUSIC_CMD_ID:
        case NEXT_TRACK_CMD_ID:
        case PREVIOUS_TRACK_CMD_ID:
        case INCREASE_VOL_CMD_ID:
        case DECREASE_VOL_CMD_ID:
        {
            switch_model_to_wwd();
            break;
        }
        case INFERENCING_TIMEOUT:
        {
            if (enabled_streaming_state==true)
            {
                ui_frame_counter_on_streaming_state=STREAMING_STATE_TIMEOUT_FRAME_COUNTER;
                return;
            }
            else
            {
                switch_model_to_wwd();
            }
            break;
        }
        default:
        {
            app_log_print("Unknown command: %d",map_id);
            break;
        }
    }
    ret = xQueueSend(map_id_q, &map_id, 0);
    if (pdTRUE != ret)
    {
        app_log_print(">>> Send failed to control task - Queue full \r\n");
    }
}

/*******************************************************************************
* Function Name: get_cur_svc_stage_and_force_model_stages
********************************************************************************
* Summary:
* Get current stage in SVC and change.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void get_cur_svc_stage_and_force_model_stages(void)
{
    cy_svc_stage_t svc_stage = CY_SVC_STAGE_INVALID;

    cy_svc_hp_get_current_stage(&svc_stage);

    if(g_cur_stage != svc_stage)
    {
        g_cur_stage = svc_stage;
    }


    if (CY_SVC_STAGE_WAITING_FOR_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTION == svc_stage)
    {
        if (WWD_STAGE != get_current_inference_stage())
        {
            switch_model_to_wwd();
            switch_to_wwd_inferencing();
        }
    }
    else if (CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT == svc_stage)
    {
        if (ASR_STAGE != get_current_inference_stage() && ptt_flag ==false)
        {
            switch_model_to_asr();
            switch_to_asr_inferencing();
        }
    }

    return;
}

/*******************************************************************************
* Function Name: battery_powered_inference_feed
********************************************************************************
* Summary:
* Feeds audio frames from audio pipeline to inferencing engine.
*
* Parameters:
*  stereo - Audio data.
*  buffer_info - Buffer information.
* 
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_feed(short *mono, cy_svc_buffer_info_t buffer_info)
{
    if(0 != buffer_info)
    {
        inference_reset_wwd_feed_timeout_counter();
    }
    if(CY_SVC_BUF_INFO_ASR_FORCE_START & buffer_info)
    {
        ptt_flag = true;
    }
    get_cur_svc_stage_and_force_model_stages();
    inference_processing((short *)mono, INFER_FRAME_SIZE/2);
    if (true == enable_stage1_timeout && true == enable_stage2_timeout)
    {
        inference_timeout_check();
    }

    app_inferencing_streaming_state_timeout();

    return;
}


/*******************************************************************************
* Function Name: battery_powered_inference_engine_init
********************************************************************************
* Summary:
* Initializes inferencing library.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_engine_init()
{
    
    BaseType_t rtos_task_status;

    infer_queue_handle = xQueueCreate(INFER_QUEUE_ELEMENTS, INFER_QUEUE_SIZE);
    if (infer_queue_handle == NULL)
    {
         app_log_print("Init queue for inferencing queue failed \r\n");
         CY_ASSERT(0);
    }

    /* Initialize ASR  */
    if (!inferencing_engine_init(app_inferencing_callback)) {
        app_log_print("Error in Inference engine initialization \r\n");
    }

    hp_app_infer_log("Inference engine initialization completed. ");

/* Defunct -code - No queuing for Battery powered case as it supports only ITSI */
    /*rtos_task_status = xTaskCreate(battery_powered_infer_task, "inferencing_task",
                       INFERENCING_STACK_SIZE, NULL, INFERENCING_TASK_PRIORITY,
                        &rtos_infer_task);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("Inferencing task create failed \r\n");
        CY_ASSERT(0);
    }*/

    return;
}

/*******************************************************************************
* Function Name: battery_powered_infer_task [defunct]
********************************************************************************
* Summary:
* Inferencing task.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_infer_task(void *arg)
{

   char infer_buffer[INFER_FRAME_SIZE]={0};
   cy_svc_buffer_info_t buffer_info = 0;
    while(1)
    {
        if (pdTRUE == xQueueReceive(infer_queue_handle, (void*)infer_buffer, portMAX_DELAY))
        {
            buffer_info = 0;
            if(pdTRUE == xQueueReceive(app_buf_info_queue, &buffer_info, portMAX_DELAY))
            {
                battery_powered_inference_feed((short*)infer_buffer, buffer_info);
            }
            else
            {
                app_log_print("Failed to get buffer info from SVC queue \r\n");
            }
        }
    }
}
/*******************************************************************************
* Function Name: battery_powered_inference_engine_deinit
********************************************************************************
* Summary:
* De-initializes inferencing library.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void battery_powered_inference_engine_deinit()
{
    inferencing_engine_deinit();
    return;
}

/* [] END OF FILE */
