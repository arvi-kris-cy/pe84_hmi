/******************************************************************************
* File Name:   battery_powered_music_player.c
*
* Description: This file includes music player handling and 
* acts as a bridge between I2S playback and control task
*
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
#include "cy_pdl.h"
#include "cybsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cy_retarget_io.h"
#include "app_logger.h"
#include "cyabs_rtos_internal.h"

#include "mtb_tlv320dac3100.h"

#include "battery_powered_music_player.h"
#include "asc_wav_header_parser.h"

#if SAMPLING_RATE == 16000u
  /* 16 KHz 1 ch playback files*/
#include "piano_16KHz_1ch_16bit.wav.h"
#include "drums_16KHz_1ch_16bit.wav.h"
#elif SAMPLING_RATE == 48000u
  /* 48 KHz 1 ch playback files*/
#include "guitar_48KHz_1ch_16bit.wav.h"
#include "drums_48KHz_1ch_16bit.wav.h"
#elif SAMPLING_RATE == 44100u
  /* 44.1 KHz 1 ch playback files*/
#include "guitar_44_1KHz_1ch_16bit.wav.h"
#include "drums_44_1KHz_1ch_16bit.wav.h"
#else
#error "Unsupported sampling rate"
#endif /* SAMPLING_RATE */

/******************************************************************************
* Macros
******************************************************************************/
#define WAV_HEADER_SIZE                     (44u)
#define MUSIC_PLAYER_TASK_QUEUE_LENGTH      (20u)

#define ZERO_PAD_BUFFER_SIZE_BYTES          (FRAME_SIZE * sizeof(int16_t))
#define I2S_PREFILL_FRAMES_COUNT            (3u)

/*******************************************************************************
* Global Variables
********************************************************************************/
/* Music Player task handle */
TaskHandle_t music_player_task_handle;
/* Handle of the queue holding the Music Player task commands. */
QueueHandle_t music_player_task_q;

music_player_audio_source_t music_player_audio_source = MUSIC_PLAYER_SOURCE_NOT_CONFIGURED;

/* I2S queue handle holding audio playback data */
extern QueueHandle_t i2s_playback_task_q;

/* Used to indicate Music player state */
bool music_player_active = false;
bool music_player_pause = false;
extern bool i2s_skip_frame;

/* Flash Music Player variables */
static const uint8_t *flash_music_ptr[] =
{
    hex_array,
    hex_array_next,
};

static unsigned int flash_music_sizes[] =
{
    hex_array_size,
    hex_array_next_size,
};

/* Holds PCM sample index which will be sent to i2s playback task to play */
static int32_t flash_music_file_index = 0;
/* Calculate number of music files */
static int32_t flash_music_file_index_max = sizeof(flash_music_sizes) / sizeof(flash_music_sizes[0]);

/* Common Music Player Variables */
static uint8_t *music_player_ptr = NULL;
static int32_t music_player_index = 0;
static int32_t music_player_index_max;

static uint8_t zero_pad_buffer1[ZERO_PAD_BUFFER_SIZE_BYTES] __attribute__ ((section(".cy_socmem_data"))) = { 0 };
static uint8_t zero_pad_buffer2[ZERO_PAD_BUFFER_SIZE_BYTES] __attribute__ ((section(".cy_socmem_data"))) = { 0 };

static uint8_t* active_zero_pad_buffer_ptr;
static uint8_t* inactive_zero_pad_buffer_ptr;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
static void music_player_task(void* pvParameters);
static bool process_music_player_wav_file(uint8_t *wav_ptr, uint32_t wav_size);
static bool process_music_player_relative_file_index(int32_t relative_file_index);

/*******************************************************************************
* Function Name: create_music_player_task
********************************************************************************
* Summary:
*  Function that creates the music player task.
*
* Parameters:
*  None
*
* Return:
*  CY_RSLT_SUCCESS upon successful creation of the task, else a non-zero value
*   that indicates the error.
*
*******************************************************************************/
cy_rslt_t create_music_player_task(void)
{
    BaseType_t status;

    status = xTaskCreate(music_player_task, "Music Player Task", MUSIC_PLAYER_TASK_STACK_SIZE,
                         NULL, MUSIC_PLAYER_TASK_PRIORITY, &music_player_task_handle);

    return (status == pdPASS) ? CY_RSLT_SUCCESS : (cy_rslt_t) status;
}

/*******************************************************************************
* Function Name: music_player_task
********************************************************************************
* Summary:
*  Music player task implementation which handles:
*      - Sending data to I2S playback based on WW/ASR command
*      - Initialize music player source (Flash)
*
* Parameters:
*  void *pvParameters : Task parameter defined during task creation (unused)
*
* Return:
*  None
*
*******************************************************************************/
static void music_player_task(void* pvParameters)
{
    /* Remove warning for unused parameter */
    (void)pvParameters;

    /* Status variable to indicate the result of various operations */
    music_player_q_data_t music_player_q_data;
    i2s_playback_q_data_t i2s_playback_q_data;
    uint32_t i2s_q_count = 0;

    /* Create a Receive message queue (to i2s control) between control tasks, music player task and i2s playback task. */
    music_player_task_q = xQueueCreate(MUSIC_PLAYER_TASK_QUEUE_LENGTH, sizeof(music_player_q_data_t));

    active_zero_pad_buffer_ptr = zero_pad_buffer1;
    inactive_zero_pad_buffer_ptr = zero_pad_buffer2;

    for (;;)
    {
        if (pdTRUE == xQueueReceive(music_player_task_q, &music_player_q_data, portMAX_DELAY))
        {
            switch(music_player_q_data.cmd)
            {
                case I2S_PLAYBACK_PLAY_MUSIC:
                    if (!is_music_player_active())
                    {
                        music_player_active = !music_player_active;

                        if (music_player_active && !music_player_pause)
                        {
                            flash_music_file_index = 0;
                            enable_music_player();
                        }

                        if(music_player_pause)
                        {
                            music_player_pause = false;
                            xTaskNotify(i2s_task_handle, 0, eNoAction);
                        }

                        i2s_q_count = I2S_PREFILL_FRAMES_COUNT;
                    }

                case I2S_PLAYBACK_RESUME_MUSIC:
                    if (!is_music_player_active())
                    {
                        music_player_active = !music_player_active;
                        if(music_player_pause)
                        {
                            music_player_pause = false;
                            xTaskNotify(i2s_task_handle, 0, eNoAction);
                        }
                        i2s_q_count = 1;
                    }
                case MUSIC_PLAYER_I2S_TRIGGER:
                    /* Request for single frame from I2S playback task */
                    if (i2s_q_count == 0)
                    {
                        i2s_q_count = 1;
                    }

                case MUSIC_PLAYER_PRELOAD_DATA:
                    if (i2s_q_count == 0)
                    {
                        i2s_q_count = I2S_PREFILL_FRAMES_COUNT;
                    }

                    if(!music_player_active)
                    {
                        break;
                    }
                    for (uint32_t i = 0; i < i2s_q_count; i++)
                    {
                        /* Data size = number of bytes per frame. Here size is considered for mono data as stereo data is already down mixed to mono */
                        i2s_playback_q_data.data_len = FRAME_SIZE * sizeof(int16_t);
                        i2s_playback_q_data.data = (int16_t*) &(music_player_ptr[music_player_index]);

                        if(NULL == i2s_playback_q_data.data)
                        {
                            break;
                        }

                        if ( (music_player_index + i2s_playback_q_data.data_len) >= music_player_index_max )
                        {
                            /* Next audio file as this file is done... */
                            process_music_player_relative_file_index(0);
                            i2s_playback_q_data.data = (int16_t*) &(music_player_ptr[music_player_index]);
                        }

                        music_player_index += (i2s_playback_q_data.data_len);

                        if (pdTRUE != xQueueSend(i2s_playback_task_q, &i2s_playback_q_data, portMAX_DELAY))
                        {
                            app_log_print(">>> Error: I2S playback Queue Send failed*******************\r\n");
                        }
                    }

                    i2s_q_count = 0;

                    break;
                case I2S_PLAYBACK_WWD:
                {
                    if(music_player_active)
                    {
                        music_player_pause = true;
                    }
                    music_player_active = false;
                    break;
                }

                case I2S_PLAYBACK_INC_VOL:
                {
                    if(MUSIC_PLAYER_SOURCE_NOT_CONFIGURED == music_player_audio_source)
                    {
                        i2s_playback_volume_control(INCREASE_VOLUME);
                    }
                    else
                    {
                        if (!is_music_player_active())
                        {
                            music_player_active = !music_player_active;
                            if(music_player_pause)
                            {
                                music_player_pause = false;
                                xTaskNotify(i2s_task_handle, 0, eNoAction);
                            }
                        }

                        i2s_playback_volume_control(INCREASE_VOLUME);
                    }
                    break;
                }
                case I2S_PLAYBACK_DEC_VOL:
                {
                    if(MUSIC_PLAYER_SOURCE_NOT_CONFIGURED == music_player_audio_source)
                    {
                        i2s_playback_volume_control(REDUCE_VOLUME);
                    }
                    else
                    {
                        if (!is_music_player_active())
                        {
                            music_player_active = !music_player_active;
                            if(music_player_pause)
                            {
                                music_player_pause = false;
                                xTaskNotify(i2s_task_handle, 0, eNoAction);
                            }
                        }

                        i2s_playback_volume_control(REDUCE_VOLUME);
                    }
                    break;
                }

                case MUSIC_PLAYER_FLUSH_DATA:
                {
                    break;
                }

                case I2S_PLAYBACK_PAUSE_MUSIC:
                {
                    if(music_player_active)
                    {
                        music_player_pause = true;
                    }
                    music_player_active = false;
                    break;
                }

                case I2S_PLAYBACK_STOP_MUSIC:
                {
                    xQueueReset(music_player_task_q);
                    xQueueReset(i2s_playback_task_q);

                    if (!is_music_player_active())
                    {
                        if(music_player_pause)
                        {
                            music_player_pause = false;
                            xTaskNotify(i2s_task_handle, 0, eNoAction);
                        }
                    }

                    music_player_active = false;
                    music_player_pause = false;
                    xQueueReset(i2s_playback_task_q);
                    i2s_deinit();
                    music_player_audio_source = MUSIC_PLAYER_SOURCE_NOT_CONFIGURED;
                    break;
                }

                case I2S_PLAYBACK_NEXT_TRACK:
                {
                    xQueueReset(music_player_task_q);
                    xQueueReset(i2s_playback_task_q);

                    if (!is_music_player_active())
                    {
                        music_player_active = !music_player_active;
                        if(music_player_pause)
                        {
                            music_player_pause = false;
                            xTaskNotify(i2s_task_handle, 0, eNoAction);
                        }
                    }
                    else if (is_music_player_paused())
                    {
                        i2s_skip_frame = true;
                    }

                    if(MUSIC_PLAYER_SOURCE_NOT_CONFIGURED == music_player_audio_source)
                    {
                        flash_music_file_index = 1;
                        enable_music_player();
                    }
                    else
                    {
                        process_music_player_relative_file_index(1);
                    }

                    music_player_q_data.cmd = MUSIC_PLAYER_PRELOAD_DATA;
                    music_player_q_data.data_len = 0;
                    music_player_q_data.data = NULL;
                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                    {
                        app_log_print(">>> Error: Next track - Music Player Task Queue Send failed*******************\r\n");
                        break;
                    }
                    break;
                }

                case I2S_PLAYBACK_PREV_TRACK:
                {
                    xQueueReset(music_player_task_q);
                    xQueueReset(i2s_playback_task_q);

                    if (!is_music_player_active())
                    {
                        music_player_active = !music_player_active;
                        if(music_player_pause)
                        {
                            music_player_pause = false;
                            xTaskNotify(i2s_task_handle, 0, eNoAction);
                        }
                    }
                    else if (is_music_player_paused())
                    {
                        i2s_skip_frame = true;
                    }

                    if(MUSIC_PLAYER_SOURCE_NOT_CONFIGURED == music_player_audio_source)
                    {
                        flash_music_file_index = (sizeof(flash_music_sizes) / sizeof(flash_music_sizes[0])) - 1;
                        enable_music_player();
                    }
                    else
                    {
                        process_music_player_relative_file_index(-1);
                    }
                    music_player_q_data.cmd = MUSIC_PLAYER_PRELOAD_DATA;
                    music_player_q_data.data_len = 0;
                    music_player_q_data.data = NULL;
                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                    {
                        app_log_print(">>> Error: Prev track - Music Player Task Queue Send failed*******************\r\n");
                        break;
                    }
                    break;
                }

                case MUSIC_PLAYER_SOURCE_FLASH_INIT:
                {
                    music_player_audio_source = MUSIC_PLAYER_SOURCE_FLASH;

                    flash_music_file_index_max = sizeof(flash_music_sizes) / sizeof(flash_music_sizes[0]);

                    if (!process_music_player_relative_file_index(0))
                    {
                        app_log_print(">>> Error: process_music_player_relative_file_index(0) failed*******************\r\n");
                        break;
                    }

                    music_player_q_data.cmd = MUSIC_PLAYER_PRELOAD_DATA;
                    music_player_q_data.data_len = 0;
                    music_player_q_data.data = NULL;
                    if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data, portMAX_DELAY))
                    {
                        app_log_print(">>> Error: Flash Init - Music Player Task Queue Send failed*******************\r\n");
                        break;
                    }
                    break;
                }

                case MUSIC_PLAYER_SOURCE_FLASH_DEINIT:
                {
                    music_player_audio_source = MUSIC_PLAYER_SOURCE_NOT_CONFIGURED;
                    break;
                }

                case I2S_PLAYBACK_DEFAULT_STATE:
                {
                    break;
                }
            }
        }
    }
}

/* ****************************************************************************
 * Function Name: configure_music_player_audio_source
 ******************************************************************************
 * Summary: 
 *   This function configures music player source (FLASH). 
 *
 * Parameters:
 *  audio_source: Audio source is Flash      
 *
 * Return:
 *  None
 *
 * ***************************************************************************/
void configure_music_player_audio_source(music_player_audio_source_t audio_source)
{
    music_player_q_data_t music_player_q_data;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    music_player_q_data.cmd = 0;
    music_player_q_data.data_len = 0;
    music_player_q_data.data = NULL;

    if (music_player_audio_source == audio_source)
    {
        return;
    }

    if (music_player_audio_source == MUSIC_PLAYER_SOURCE_FLASH)
    {
        music_player_q_data.cmd = MUSIC_PLAYER_SOURCE_FLASH_DEINIT;
    }

    if (music_player_q_data.cmd > 0)
    {
        if (is_in_isr())
        {
            if (pdTRUE != xQueueSendToFrontFromISR(music_player_task_q, &music_player_q_data, &xHigherPriorityTaskWoken))
            {
                app_log_print(">>> Error: Music Player Task Queue Send from ISR failed*******************\r\n");
            }
            else
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            if (pdTRUE != xQueueSendToFront(music_player_task_q, &music_player_q_data, portMAX_DELAY))
            {
                app_log_print(">>> Error: Music Player Task Queue Send failed*******************\r\n");
            }
        }
    }

    /* Init required music player source */
    music_player_q_data.cmd = 0;
    music_player_q_data.data_len = 0;
    music_player_q_data.data = NULL;

    if (audio_source == MUSIC_PLAYER_SOURCE_FLASH)
    {
        music_player_q_data.cmd = MUSIC_PLAYER_SOURCE_FLASH_INIT;
    }

    if (music_player_q_data.cmd > 0)
    {
        if (is_in_isr())
        {
            if (pdTRUE != xQueueSendToFrontFromISR(music_player_task_q, &music_player_q_data, &xHigherPriorityTaskWoken))
            {
                app_log_print(">>> Error: Music Player Task Queue Send from ISR failed*******************\r\n");
            }
            else
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
        }
        else
        {
            if (pdTRUE != xQueueSendToFront(music_player_task_q, &music_player_q_data, portMAX_DELAY))
            {
                app_log_print(">>> Error: Music Player Task Queue Send failed*******************\r\n");
            }
        }
    }

}

/*******************************************************************************
* Function Name: is_music_player_active
********************************************************************************
* Summary:
* Returns music player state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
bool is_music_player_active(void)
{
    return music_player_active;
}

/*******************************************************************************
* Function Name: is_music_player_paused
********************************************************************************
* Summary:
* Returns music player state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
bool is_music_player_paused(void)
{
    return music_player_pause;
}

/* ****************************************************************************
 * Function Name: process_music_player_wav_file
 ******************************************************************************
 * Summary: Parses audio data array for wav header and audio data
 *
 * Parameters:        
 *  wav_ptr: audio data array pointer
 *  wav_size: audio data size in bytes
 * Return:
 *  None
 *
 * ***************************************************************************/
static bool process_music_player_wav_file(uint8_t *wav_ptr, uint32_t wav_size)
{
    uint32_t        n_channels;
    uint32_t        i_channel_mask;
    uint32_t        sample_rate;
    uint32_t        pcm_sz;
    int32_t         length;
    unsigned int    asc_stream_read_counter;

    if (wav_size < WAV_HEADER_SIZE)
    {
        return false;
    }

    if( 0 != cy_wav_header_decode(&n_channels,
                                  &i_channel_mask,
                                  &sample_rate,
                                  &pcm_sz,
                                  &length,
                                  wav_ptr,
                                  &asc_stream_read_counter,
                                  wav_size ) )
    {
        app_log_print("Error WAV header parsing failed\r\n");
        return false;
    }
    music_player_ptr = wav_ptr;
    music_player_index = asc_stream_read_counter;
    music_player_index_max = wav_size;

    return true;
}


/* ****************************************************************************
 * Function Name: process_music_player_relative_file_index
 ******************************************************************************
 * Summary:
 *  Process music file and file indexes. File index indicate next sample played over I2S.
 *
 * Parameters:
 *   relative_file_index: +1 for next song, -1 for prev song, 0 for start current song 
 *   from start
 *
 * Return:
 *  None
 *
 * ***************************************************************************/
static bool process_music_player_relative_file_index(int32_t relative_file_index)
{
    const uint8_t **music_ptr;
    unsigned int *music_size;
    int32_t *music_file_index;
    int32_t music_file_index_max;
    int32_t file_index;

    int32_t parser_counter = 0;

    if (MUSIC_PLAYER_SOURCE_FLASH == music_player_audio_source)
    {
        music_ptr = flash_music_ptr;
        music_size = flash_music_sizes;
        music_file_index = &flash_music_file_index;
        music_file_index_max = flash_music_file_index_max;
    }
    else
    {
        return false;
    }

    file_index = (*music_file_index + relative_file_index) % music_file_index_max;

    if (file_index < 0)
    {
        file_index += music_file_index_max;
    }

    do
    {
        if (process_music_player_wav_file((uint8_t*)music_ptr[file_index], music_size[file_index]))
        {
            break;
        }
        app_log_print("Error parsing file at index %d...\n", (int)file_index);

        file_index ++;
        if (file_index >= music_file_index_max)
        {
            file_index = 0;
        }

        parser_counter ++;
    }
    while(parser_counter < music_file_index_max);

    if (parser_counter == music_file_index_max)
    {
        app_log_print(">> Music Player file index processing failed for all file indices in %d mode...\n", music_player_audio_source);
        return false;
    }

    *music_file_index = file_index;

    return true;
}
/* [] END OF FILE */
