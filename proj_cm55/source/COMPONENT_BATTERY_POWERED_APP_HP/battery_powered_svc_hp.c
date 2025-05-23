/******************************************************************************
* File Name : battery_powered_svc_hp.c
*
* Description :
* Staged Voice Control Application code on CM55.
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

#include "cy_pdl.h"
#include "cybsp.h"
#include "cy_log.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#include "FreeRTOS.h"
#include "task.h"

#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_hp.h"
#include "staged_voice_control_hp_private.h"
#include "staged_voice_control_hp_ipc.h"
#include "battery_powered_svc_hp.h"

#include "inferencing_interface.h"
#include "battery_powered_inferencing.h"
#include "battery_powered_afe.h"
#include "afe_app_defines.h"

#include "svc_app_common.h"
#include "audio_input_configuration.h"
#include "app_logger.h"
#include "cy_audio_front_end_error.h"

#include "intercore_ipc.h"
#ifdef AUDIO_OUT
#include "i2s_playback.h"
#include "battery_powered_music_player.h"
#endif /* AUDIO_OUT */
#include "cy_afe_configurator_settings.h"
/*******************************************************************************
* Macros
*******************************************************************************/
#define SVC_HP_APP_CMD_Q_MSG_SIZE                   (20)
#define SVC_HP_APP_CMD_Q_SIZE                       (400)
#define SVC_HP_APP_TASK_Q_STACK_SIZE                (10*1024)
#define SVC_HP_APP_TASK_PRIORITY                    (4)
/* Number of samples in an audio frame */
#define SVC_FRAME_SIZE                              (160u)
#define SAMPLE_INCREMENT_STEREO                     (SVC_FRAME_SIZE*4)
#define SAMPLE_INCREMENT_MONO                       (SVC_FRAME_SIZE*2)
#define FRAME_SIZE_MS                               (10u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
typedef struct
{
     TaskHandle_t thread;
     QueueHandle_t app_cmd_queue;
     bool init_done;
     unsigned int total_frame_received_count;

} svc_hp_app_t;

svc_hp_app_t svc_hp_app = {0};
int16_t aec_ref_buffer[MONO_AUDIO_DATA_IN_BYTES/2] __attribute__ ((section(".cy_socmem_data")));
int16_t* actual_aec_ref = aec_ref_buffer;

/*******************************************************************************
* Function Name: svc_hp_app_process_data
********************************************************************************
* Summary:
* Process data received via SVC and feed to AFE.
*
* Parameters:
*  frame_buffer - Pointer to audio frame data.
*  frame_count - Frame count.
*  buffer_info - Buffer information.
*
* Return:
*  None
*
*******************************************************************************/

static void svc_hp_app_process_data(
        uint8_t *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buffer_info)
{

    char *data = NULL;
    unsigned int counter = 0;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    int16_t* aec_reference = NULL;

    uint8_t bulk_delay = AFE_CONFIG_BULK_DELAY;
    uint8_t bulk_delay_equivalent_frames = 0;

    if(bulk_delay <= FRAME_SIZE_MS)
    {
        bulk_delay_equivalent_frames = 0u;
    }
    else
    {
        bulk_delay_equivalent_frames = (bulk_delay/FRAME_SIZE_MS) - 1;
    }

    if (1 < frame_count)
    {
        app_svc_log_hp("CurFrameCount:%d [0x%x] TotFCount:%d",
                frame_count, frame_buffer, svc_hp_app.total_frame_received_count);
    }

    if(CY_SVC_BUF_INFO_PREROLL_INSUFFICIENT_BUF & buffer_info)
    {
        app_svc_log_hp("InSufficient PreRoll frame count:%d",
                frame_count);
    }


    if(CY_SVC_BUF_INFO_ASR_FORCE_START & buffer_info)
    {
        app_svc_log_hp("ASR Force Start Buf Info");
        if (app_inferencing_get_streaming_state()==false)
        {
            switch_model_to_asr();
            switch_to_asr_inferencing(); /* For Push to Talk */
        } else
        {
            app_log_print("PTT during ASR processing \r\n");
            switch_model_to_asr();
            switch_to_asr_inferencing_streaming(); /* For Push to Talk */
        }
    }
    data = (char *)frame_buffer;
    if(NULL != data)
    {
        for (counter = 0; counter < frame_count; counter++)
        {
#ifdef ENABLE_IFX_AEC
    bool static bulk_dealy_wait_flag = false;
    if(is_music_player_active() || is_music_player_paused())
    {
        if(!bulk_dealy_wait_flag && (uxQueueMessagesWaiting(aec_ref_data_q) >= bulk_delay_equivalent_frames))
        {
            bulk_dealy_wait_flag = true;
        }

        if (bulk_dealy_wait_flag)
        {
            if (pdTRUE != xQueueReceive(aec_ref_data_q, &aec_reference, 0))
            {
                actual_aec_ref = NULL;
            }
            else
            {
                actual_aec_ref = aec_reference;
            }
        }
        else
        {
            actual_aec_ref = NULL;
        }
    }
    else
    {
        bulk_dealy_wait_flag = false;
        actual_aec_ref = NULL;
    }
#else
        actual_aec_ref = NULL;
#endif /* ENABLE_IFX_AEC */
        result = battery_powered_afe_mw_feed(data, actual_aec_ref, buffer_info);
        if(CY_RSLT_SUCCESS != result)
        {
            app_log_print("Failed to feed audio frame to AFE \r\n");
        }

#ifdef ENABLE_STEREO_INPUT_FEED
            data = data + SAMPLE_INCREMENT_STEREO;
#else
            data = data + SAMPLE_INCREMENT_MONO;
#endif /* ENABLE_STEREO_INPUT_FEED*/
        }
    }
}

/*******************************************************************************
* Function Name: svc_hp_app_data_callback
********************************************************************************
* Summary:
* Callback for audio data on CM55.
*
* Parameters:
*  frame_buffer - Pointer to SVC data.
*  frame_count - Frame counter.
*  buf_info - Buffer information.
*  callback_user_argument - User argument (unused)
*
* Return:
*  Result of put queue operation.
*
*******************************************************************************/

cy_rslt_t svc_hp_app_data_callback(
        CY_SVC_DATA_T *frame_buffer,
        uint32_t frame_count,
        cy_svc_buffer_info_t buf_info,
        void *callback_user_arg)
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    svc_hp_app_cmd_q_data_t data_buf_to_app = {0};
    svc_hp_app.total_frame_received_count += frame_count;

    if (NULL != frame_buffer)
    {
        data_buf_to_app.buffer_info = buf_info;

        for (int i = 0; i < frame_count; i++)
        {
            data_buf_to_app.cmd_id = (char) SVC_CMD_HEADER;
            data_buf_to_app.data_pointer = (uint8_t*) frame_buffer;
            data_buf_to_app.frame_count = 1;

            if (pdTRUE != xQueueSendToBack(svc_hp_app.app_cmd_queue, (void*)&data_buf_to_app, 0))
            {
                app_log_print("SVC queue full with frame count = one \r\n");
            }

#ifdef ENABLE_STEREO_INPUT_FEED
            frame_buffer = frame_buffer + SAMPLE_INCREMENT_MONO;
#else
            frame_buffer = frame_buffer + SVC_FRAME_SIZE;
#endif /* ENABLE_STEREO_INPUT_FEED */
            data_buf_to_app.buffer_info = 0;
        }
    }
    else
    {
        data_buf_to_app.buffer_info = buf_info;
        data_buf_to_app.cmd_id = (char) SVC_CMD_HEADER;
        data_buf_to_app.data_pointer = (uint8_t*) frame_buffer;
        data_buf_to_app.frame_count = frame_count;

        if (pdTRUE != xQueueSendToBack(svc_hp_app.app_cmd_queue, (void*)&data_buf_to_app, 0))
        {
            app_log_print("SVC buffer full with frame buffer == NULL \r\n");
        }
    }

    return ret_val;
}

/*******************************************************************************
* Function Name: svc_hp_app_init
********************************************************************************
* Summary:
* Initialization function for staged voice application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void svc_hp_app_init(void)
{
    cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_svc_hp_config_t init_svc = {0};

    init_svc.callback_user_arg = &svc_hp_app;
    init_svc.data_callback = svc_hp_app_data_callback;
    init_svc.ipc_communication_sync_sempahore = 0;

    init_svc.ipc_pipe_config.lp_pipe_end_point_address = APP_LP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.lp_pipe_end_point_client_id = APP_LP_PIPE_END_POINT_CLIENT_ID_1;
    init_svc.ipc_pipe_config.lp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.lp_pipe_end_point_client_id);

    init_svc.ipc_pipe_config.hp_pipe_end_point_address = APP_HP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.hp_pipe_end_point_client_id = APP_HP_PIPE_END_POINT_CLIENT_ID_2;
    init_svc.ipc_pipe_config.hp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.hp_pipe_end_point_client_id);


    ret_val = cy_svc_hp_init(&init_svc);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_log_print("SVC initialization failed in CM55 \r\n");
    }
    else
    {
        svc_hp_app.init_done = true;
        app_log_print("SVC initialization success \r\n");
    }
}
/*******************************************************************************
* Function Name: svc_hp_app_deinit
********************************************************************************
* Summary:
* Deinitialize Staged Voice Control application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void svc_hp_app_deinit(void)
{
    (void) cy_svc_hp_deinit();
}

/*******************************************************************************
* Function Name: svc_hp_app_print_statistics
********************************************************************************
* Summary:
* Print statistics of SVC application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void svc_hp_app_print_statistics()
{
    size_t num_waiting = 0;

    if (NULL != svc_hp_app.app_cmd_queue)
    {
        num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
        if(0 != num_waiting)
            app_svc_log_hp("HP APP Pending msg:%d", num_waiting);
    }
    return;
}

/*******************************************************************************
* Function Name: svc_hp_app_process_app_cmd_q
********************************************************************************
* Summary:
* Process command queue.
*
* Parameters:
*  None
*
* Return:
*  Result of get queue operation.
*
*******************************************************************************/


static cy_rslt_t svc_hp_app_process_app_cmd_q()
{
    cy_rslt_t ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    uint8_t data[SVC_HP_APP_CMD_Q_MSG_SIZE] = {0};

    if(pdTRUE != xQueueReceive(svc_hp_app.app_cmd_queue, (void*)&data[0], portMAX_DELAY))
    {
        app_svc_log_hp("Get queue failed for SVC application");
        return ret_val;
    }
    if (SVC_CMD_HEADER == data[0])
    {
        svc_hp_app_cmd_q_data_t *data_buf_to_app = (svc_hp_app_cmd_q_data_t*) data;
        (void) svc_hp_app_process_data(data_buf_to_app->data_pointer,  data_buf_to_app->frame_count,
                data_buf_to_app->buffer_info);

    }
    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: app_svc_hp_clear_queue
********************************************************************************
* Summary:
* Clear the SVC queue.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_svc_hp_clear_queue(void)
{
    size_t num_waiting = 0;
    int index =0;
    uint8_t data[SVC_HP_APP_CMD_Q_MSG_SIZE] = {0};

    if (NULL != svc_hp_app.app_cmd_queue)
    {
        num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
        app_svc_log_hp("Number of elements in the queue %d",num_waiting);
        for (index=0; index < num_waiting; index++)
        {
            if(pdTRUE != xQueueReceive(svc_hp_app.app_cmd_queue, (void*)&data[0], 0))
            {
                app_svc_log_hp("Queue cleared");
                break;
            }
        }
    }
    num_waiting = uxQueueMessagesWaiting(svc_hp_app.app_cmd_queue);
    app_svc_log_hp("Number of elements in the queue %d",num_waiting);
}

/*******************************************************************************
* Function Name: svc_hp_app_thread
********************************************************************************
* Summary:
* Task for handling commands sent via SVC.
*
* Parameters:
*  thread_input - Input to task (Unused)
*
* Return:
*  None
*
*******************************************************************************/

static void svc_hp_app_thread()
{

    for (;;)
    {
        if (true == svc_hp_app.init_done)
        {
            svc_hp_app_process_app_cmd_q();
        }
    }

}

/*******************************************************************************
* Function Name: app_svc_hp_app_init
********************************************************************************
* Summary:
* Initialize queues and Task for SVC application on CM55.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_svc_hp_app_init(void)
{
    BaseType_t rtos_task_status;

    svc_hp_app_init();

    svc_hp_app.app_cmd_queue = xQueueCreate(SVC_HP_APP_CMD_Q_SIZE, SVC_HP_APP_CMD_Q_MSG_SIZE);
    if (svc_hp_app.app_cmd_queue == NULL)
    {
        app_log_print("Unable to create queue for SVC application \r\n");
        CY_ASSERT(0);
    }

    /* Start SVC data process thread */

    rtos_task_status = xTaskCreate(svc_hp_app_thread, "svc_hp_app_thread",
                        SVC_HP_APP_TASK_Q_STACK_SIZE, NULL, SVC_HP_APP_TASK_PRIORITY,
                        &svc_hp_app.thread);

    if (pdPASS != rtos_task_status)
    {
        app_log_print("Unable to create SVC thread \r\n");
        CY_ASSERT(0);
    }
    return;
}

/* [] END OF FILE */

