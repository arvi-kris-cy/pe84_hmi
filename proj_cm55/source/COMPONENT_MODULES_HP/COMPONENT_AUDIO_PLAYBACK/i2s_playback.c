/******************************************************************************
* File Name : i2s_playback.c
*
* Description :
* Source file for Audio Playback via I2S.
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
#include "cybsp.h"
#include "cy_pdl.h"

#include "i2s_playback.h"
#include "mtb_tlv320dac3100.h"
#include "IFX_asrc.h"

#include "app_logger.h"

#ifdef BATTERY_POWERED_LOCAL_VOICE
#include "battery_powered_music_player.h"
#endif /* BATTERY_POWERED_LOCAL_VOICE */

#ifdef MAINS_POWERED_LOCAL_VOICE
#include "mains_powered_music_player.h"
#endif /* MAINS_POWERED_LOCAL_VOICE */

#ifdef DEBUG_TUNE_MAINS_APP
#include "tune_mains_powered_music_player.h"
#endif /* DEBUG_TUNE_MAINS_APP */

#include "app_i2s.h"
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Handle of the queue holding the aec reference data at 16KHz sampling rate. */
QueueHandle_t aec_ref_data_q;

/* AEC reference control variables */
static int16_t i2s_stereo_playback_buffer1[PLAYBACK_DATA_FRAME_SIZE] __attribute__ ((section(".cy_socmem_data"))) = { 0 };
static int16_t i2s_stereo_playback_buffer2[PLAYBACK_DATA_FRAME_SIZE] __attribute__ ((section(".cy_socmem_data"))) = { 0 };

static int16_t* active_i2s_playback_buffer_ptr = NULL;
static int16_t* inactive_i2s_playback_buffer_ptr = NULL;

/* Music Player task handle */
TaskHandle_t i2s_task_handle;
QueueHandle_t i2s_playback_task_q;

/* Circular buffer variables */
circular_buffer_t aec_ref_cbuf;
uint8_t aec_ref_16khz_data[AEC_REF_CBUF_SIZE] __attribute__ ((section(".cy_socmem_data"))) = { 0 };

/* Start playback by writing first frame received over queue 
   then onwards i2s isr trigger requests frames from music player task */
bool first_frame = true;
bool i2s_deinit_flag = false;
bool i2s_skip_frame = false;

static volatile bool i2s_data_ready_flag = false;

/* I2S playback default volume */
uint8_t i2s_playback_volume = DEFAULT_VOLUME; 

/* ASRC variables for down-sampling audio to 16000Hz */
IFX_ASRC_STRUCT_t asrc_mem_down_sampling;
#if SAMPLING_RATE_44_1kHz == SAMPLING_RATE 
/* ASRC variables for up-sampling 44100Hz to 48000Hz */
IFX_ASRC_STRUCT_t asrc_mem_up_sampling;
#endif /* SAMPLING_RATE_44_1kHz */

static uint32_t input_sampling_freq = 0;
static uint32_t output_sampling_freq = 0;
static uint16_t asrc_out_len = 0;

/* Definition of buffers for ASRC, AEC and I2S */
static int32_t asrc_in_i2s_playback_buffer_32bit[FRAME_SIZE]           __attribute__ ((section(".cy_socmem_data"))) = { 0 };
static int32_t asrc_out_aec_ref_buffer_32bit[ASRC_OUTPUT_BUFFER_SIZE]  __attribute__ ((section(".cy_socmem_data"))) = { 0 };
static int16_t asrc_out_aec_ref_buffer_16bit[AEC_REF_FRAME_SIZE]       __attribute__ ((section(".cy_socmem_data"))) = { 0 };
#if SAMPLING_RATE_44_1kHz == SAMPLING_RATE 
static int16_t asrc_out_up_sampled_buffer_16bit[PLAYBACK_DATA_FRAME_SIZE/2]  __attribute__ ((section(".cy_socmem_data"))) = { 0 };
#endif /* SAMPLING_RATE_44_1kHz */
static int16_t i2s_stereo_zero_buffer[PLAYBACK_DATA_FRAME_SIZE]        __attribute__ ((section(".cy_socmem_data"))) = { 0 };
static int16_t aec_ref_zero_buffer[AEC_REF_FRAME_SIZE]                 __attribute__ ((section(".cy_socmem_data"))) = { 0 };
int16_t* i2s_playback_ptr = i2s_stereo_zero_buffer;

/* Pointer declarations for AEC Reference ASRC implementation */
static int16_t* frame_in_buffer;
static int16_t* frame_out_buffer;
static int16_t* aec_ref_cb_ptr = NULL;

uint8_t i2s_32_samples_frame_count = 0;
/*******************************************************************************
* Function Name: create_i2s_task
********************************************************************************
* Summary:
* Creates i2s task 
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void create_i2s_task(void)
{
    BaseType_t status;
    status = xTaskCreate(i2s_playback_task, "I2S Out Task",
                    I2S_PLAYBACK_TASK_STACK_SIZE, NULL, I2S_PLAYBACK_TASK_PRIORITY,
                    &i2s_task_handle);
    if (pdPASS != status)
    {
        app_log_print("Error in I2S playback task \r\n");
    }
    app_log_print("Ready for I2S output \r\n");
}

/*******************************************************************************
 * Function Name: i2s_playback_task
 ********************************************************************************
 * Summary: i2s playback task which handles
 *          - Receiving audio data from music player task
 *          - Do ASRC and enqueue aec reference into circular buffer (cbuf)
 *          - Send rreceived audio frame to I2S 
 *          - Request for new frame from music player task
 *
 *  Parameters:
 *  arg:
 *
 * Return:
 *  None
 *******************************************************************************/
void i2s_playback_task(void *arg)
{
    i2s_playback_q_data_t i2s_playback_q_data;
    circular_buffer_init(&aec_ref_cbuf, aec_ref_16khz_data, AEC_REF_CBUF_SIZE);
    music_player_q_data_t music_player_q_data;
    int16_t* temp_buffer_ptr;
    uint32_t ulNotificationValue = 0;

    int32_t* asrc_out_ptr = NULL;

    /* Create a send message queue to communicate audio data to i2s playback task */
    i2s_playback_task_q = xQueueCreate(I2S_PLAYBACK_TASK_QUEUE_LENGTH,
                                       sizeof(i2s_playback_q_data_t));

    /* Create a send message queue to communicate audio data to i2s playback task */
    aec_ref_data_q = xQueueCreate(AEC_REF_DATA_QUEUE_LENGTH, sizeof(int16_t*));

    active_i2s_playback_buffer_ptr = i2s_stereo_playback_buffer1;
    inactive_i2s_playback_buffer_ptr = i2s_stereo_playback_buffer2;

    while (1)
    {
        if (pdTRUE != xQueueReceive(i2s_playback_task_q, &i2s_playback_q_data,
                                    portMAX_DELAY))
        {
            app_log_print(">>> Error: I2S Task Queue Receive failed !\r\n");
        }

        /* Wait for notification in case of paused state.
         * This occurs when the I2S playback is paused due to WWD in the
         * middle of audio playback.
         */
        if (!is_music_player_active() && is_music_player_paused())
        {
            xTaskNotifyWait(0, 0, &ulNotificationValue, portMAX_DELAY);
        }

        /* Clear the flags, queues and notifications in case of I2S de-init.
         * This occurs when the "Stop Music" command is given.
         */
        if(i2s_deinit_flag)
        {
            i2s_deinit_flag = false;
            first_frame = true;
            xQueueReset(i2s_playback_task_q);
            xQueueReset(aec_ref_data_q);
            ulTaskNotifyValueClear(i2s_task_handle, UINT32_MAX_VALUE);
            continue;
        }

        /* Communicate to Music Player task to add one more frame to I2S
         * playback queue.
         */
        music_player_q_data.cmd = MUSIC_PLAYER_I2S_TRIGGER;
        music_player_q_data.data = NULL;
        music_player_q_data.data_len = 0;
        if (pdTRUE != xQueueSend(music_player_task_q, &music_player_q_data,
                                 portMAX_DELAY))
        {
            app_log_print(">>> Error: Music Player Queue Send failed!\r\n");
        }
            if(first_frame)
            {
                /* i2s init */
#ifdef AUDIO_OUT
                i2s_init();
#endif /* AUDIO_OUT */
            }
#if SAMPLING_RATE_44_1kHz == SAMPLING_RATE
            /* The 'first_frame' flag is set only during the 1st frame of
            * playback for a new file.
            */
            if(first_frame)
            {

                /* Initialize ASRC for up sampling 44.1KHz to 48kHz playback data */
                input_sampling_freq = SAMPLING_RATE_44_1kHz;
                output_sampling_freq = SAMPLING_RATE_48kHz;

                init_IFX_asrc(&asrc_mem_up_sampling, input_sampling_freq, output_sampling_freq);
                IFX_SetClockDrift(&asrc_mem_up_sampling, 0);
            }

            frame_in_buffer = (int16_t *) i2s_playback_q_data.data;
            for (uint32_t i = 0; i < FRAME_SIZE; i++)
            {
                asrc_in_i2s_playback_buffer_32bit[i] = (int32_t)
                                                 (*(int16_t *)frame_in_buffer);
                frame_in_buffer ++;
            }
            asrc_out_len = ASRC_OUTPUT_BUFFER_SIZE;
            asrc_out_ptr = asrc_out_aec_ref_buffer_32bit;
            frame_out_buffer = asrc_out_up_sampled_buffer_16bit;
            /* IFX_asrc() called 2 times to pass 441 samples, 240 and 201 in each call.*/
            for(uint32_t i = 0; i < ASRC_NUM_ITERATIONS_PER_FRAME; i++)
            {
                if((ASRC_NUM_ITERATIONS_PER_FRAME - 1) != i)
                {
                    IFX_asrc(asrc_in_i2s_playback_buffer_32bit +
                            ASRC_INPUT_SAMPLES * i, ASRC_INPUT_SAMPLES,
                            asrc_out_ptr, &asrc_out_len, &asrc_mem_up_sampling);
                }
                else
                {
                    IFX_asrc(asrc_in_i2s_playback_buffer_32bit +
                            ASRC_INPUT_SAMPLES * i, 201,
                            asrc_out_ptr, &asrc_out_len, &asrc_mem_up_sampling);
                }
                /* Convert int32_t to int16_t */
                for(uint32_t j = 0; j < asrc_out_len; j++)
                {
                    *(frame_out_buffer++) = (int16_t)(*(asrc_out_ptr + j));
                }
                asrc_out_ptr += asrc_out_len;
            }
#endif

        /* Convert the input mono channel audio to stereo by appropriate zero
            * padding for playback over I2S.
            */
#if SAMPLING_RATE_44_1kHz == SAMPLING_RATE                         
            convert_mono_to_stereo((int16_t *)asrc_out_up_sampled_buffer_16bit,
                                    (PLAYBACK_DATA_FRAME_SIZE / 2),
                                    inactive_i2s_playback_buffer_ptr);
#else
            convert_mono_to_stereo((int16_t *)i2s_playback_q_data.data,
                                    (PLAYBACK_DATA_FRAME_SIZE / 2),
                                    inactive_i2s_playback_buffer_ptr);
#endif

        temp_buffer_ptr = active_i2s_playback_buffer_ptr;
        active_i2s_playback_buffer_ptr = inactive_i2s_playback_buffer_ptr;
        inactive_i2s_playback_buffer_ptr = temp_buffer_ptr;
        i2s_data_ready_flag = true;

        /* The 'first_frame' flag is set only during the 1st frame of
         * playback for a new file.
         */
        if(first_frame)
        {
#if SAMPLING_RATE_16kHz != SAMPLING_RATE
            /* Initialize ASRC for 16kHz AEC reference */
            input_sampling_freq = SAMPLING_RATE;
            output_sampling_freq = AEC_REF_SAMPLING_RATE;

            /* ASRC for 16KHz aec ref from 44.1KHz/48KHz playback data */
            init_IFX_asrc(&asrc_mem_down_sampling, input_sampling_freq, output_sampling_freq);
            IFX_SetClockDrift(&asrc_mem_down_sampling, 0);
#endif
            /* Only for the first frame, the I2S write happens from this task. */
            if(!i2s_deinit_flag)
            {
                app_i2s_enable();
                i2s_write_32_samples();
                app_i2s_activate();
            }
            i2s_data_ready_flag = false;
        }

        /* Perform ASRC for sampling rates apart from 16kHz and enqueue the
         * AEC reference data in a circular buffer.
         */
#if SAMPLING_RATE_16kHz != SAMPLING_RATE
            frame_in_buffer = (int16_t *) i2s_playback_q_data.data;
            for (uint32_t i = 0; i < FRAME_SIZE; i++)
            {
                asrc_in_i2s_playback_buffer_32bit[i] = (int32_t)
                                                 (*(int16_t *)frame_in_buffer);
                frame_in_buffer ++;
            }

            asrc_out_len = ASRC_OUTPUT_BUFFER_SIZE;
            asrc_out_ptr = asrc_out_aec_ref_buffer_32bit;
            frame_out_buffer = asrc_out_aec_ref_buffer_16bit;
            /* IFX_asrc() called 2 times to pass the samples */
            for(uint32_t i = 0; i < ASRC_NUM_ITERATIONS_PER_FRAME; i++)
            {
                if(SAMPLING_RATE_48kHz == SAMPLING_RATE)
                {
                    IFX_asrc(asrc_in_i2s_playback_buffer_32bit +
                             ASRC_INPUT_SAMPLES * i, ASRC_INPUT_SAMPLES,
                             asrc_out_ptr, &asrc_out_len, &asrc_mem_down_sampling);
                }
                if(SAMPLING_RATE_44_1kHz == SAMPLING_RATE)
                {
                    if((ASRC_NUM_ITERATIONS_PER_FRAME - 1) != i)
                    {
                        IFX_asrc(asrc_in_i2s_playback_buffer_32bit +
                                ASRC_INPUT_SAMPLES * i, ASRC_INPUT_SAMPLES,
                                asrc_out_ptr, &asrc_out_len, &asrc_mem_down_sampling);
                    }
                    else
                    {
                        IFX_asrc(asrc_in_i2s_playback_buffer_32bit +
                                ASRC_INPUT_SAMPLES * i, (FRAME_SIZE - ASRC_INPUT_SAMPLES),
                                asrc_out_ptr, &asrc_out_len, &asrc_mem_down_sampling);
                    }
                }
                /* Convert int32_t to int16_t */
                for(uint32_t j = 0; j < asrc_out_len; j++)
                {
                    *(frame_out_buffer++) = (int16_t)(*(asrc_out_ptr + j));
                }
                asrc_out_ptr += asrc_out_len;
            }

            /* Push the frame into aec ref circular buffer */
            aec_ref_cb_ptr = (int16_t*)circular_buffer_enqueue(&aec_ref_cbuf,
                                       (uint8_t*)asrc_out_aec_ref_buffer_16bit);
#else
            /* Push the frame into aec ref circular buffer */
            aec_ref_cb_ptr = (int16_t*)circular_buffer_enqueue(&aec_ref_cbuf,
                                       (uint8_t*)i2s_playback_q_data.data);
#endif

        /* Only for the first frame, push the AEC reference data from the task. */
#ifdef ENABLE_IFX_AEC
        if(!i2s_deinit_flag && first_frame)
        {
            /* Send the AEC reference data pointer to the queue. */
            if (pdTRUE != xQueueSend(aec_ref_data_q, &aec_ref_cb_ptr, portMAX_DELAY))
            {
                app_log_print(">>> Error: I2S Task AEC Ref Queue Send failed!\r\n");
            }
        }
#endif /* ENABLE_IFX_AEC */

        /* Wait for notification from I2S ISR for completion of previous
         * audio frame playback.
         */
        if (!first_frame && !i2s_deinit_flag)
        {
            xTaskNotifyWait(0, 0, &ulNotificationValue, portMAX_DELAY);
        }

        /* Reset first frame flag. */
        first_frame = false;
    }
}

/*******************************************************************************
* Function Name: i2s_write_32_samples
********************************************************************************
* Summary: Writes 32 samples for each channel.
* 
*
* Parameters:
* None
*
* Return:
*  None
*
*******************************************************************************/
void i2s_write_32_samples(void)
{
    i2s_32_samples_frame_count = 1;
    for(int i=0; i < HW_FIFO_SIZE; i++)
    {
        /* Write same data for L,R channels(dual mono) in FIFO */
        Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) *((i2s_playback_ptr + (HW_FIFO_SIZE * (i2s_32_samples_frame_count - 1))) + i));
        i++;
        Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) *((i2s_playback_ptr + (HW_FIFO_SIZE * (i2s_32_samples_frame_count - 1))) + i));
    }
    i2s_32_samples_frame_count = 2;
}

/*******************************************************************************
* Function Name: convert_mono_to_stereo
********************************************************************************
* Summary:
* Converts mono audio data into stereo by duplicating left channel for righ channel.
*
* Parameters:
* None
*
* Return:
*  None
*
*******************************************************************************/
void convert_mono_to_stereo(int16_t *mono_data, uint32_t mono_data_num_samples,
                            int16_t *stereo_data)
{
    for (uint32_t i = 0; i < mono_data_num_samples; i++)
    {
        *stereo_data++ = *mono_data;  // Copy the mono sample to the left channel
        *stereo_data++ = *mono_data;  // Copy the same mono sample to the right channel
        mono_data++;  // Move to the next mono sample
    }
}

/*******************************************************************************
* Function Name: i2s_init
********************************************************************************
* Summary:
* Initialize I2S block and Hardware codec.
*
* Parameters:
* None
*
* Return:
*  None
*
*******************************************************************************/
void i2s_init(void)
{
    /* TLV codec initiailization */
    app_tlv_codec_init();

    /* I2S initialization */
    app_i2s_init();
}

/*******************************************************************************
* Function Name: i2s_deinit()
********************************************************************************
* Summary:
* Set a I2S deinitialize flag and clear the queues and task notifcations.
*
* Parameters:
* None
*
* Return:
*  None
*
*******************************************************************************/
void i2s_deinit()
{
    i2s_deinit_flag = true;

    xQueueReset(i2s_playback_task_q);
    xQueueReset(aec_ref_data_q);
    xTaskNotify(i2s_task_handle, 0, eNoAction);

    if(!first_frame)
    {
        Cy_AudioTDM_DeActivateTx(TDM_STRUCT0_TX);
        Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);

        first_frame = true;
    }
}

/*******************************************************************************
* Function Name: i2s_isr_handler
********************************************************************************
* Summary:
*  I2S ISR handler. Notifies i2s_control_task.
*
* Parameters:
*  arg: not used
*  event: event that occurred
*
* Return:
*  None
*******************************************************************************/
void i2s_tx_interrupt_handler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Get interrupt status and check for tigger interrupt and errors */
    uint32_t intr_status = Cy_AudioTDM_GetTxInterruptStatusMasked(TDM_STRUCT0_TX);

    if(CY_TDM_INTR_TX_FIFO_TRIGGER & intr_status)
    {
        if(((PLAYBACK_DATA_FRAME_SIZE / HW_FIFO_SIZE) + 1) == i2s_32_samples_frame_count)
        {
            /* When 5 frames of the data is written into I2S call 10msec frame handlling() */
            {
                music_player_q_data_t music_player_q_data;

                /* Initially, set the I2S playback and AEC reference to zero buffers. */
                int16_t* aec_ref_temp_ptr = aec_ref_cb_ptr;
                aec_ref_cb_ptr = aec_ref_zero_buffer;

                /* Set the I2S playback and AEC reference pointers to actual data in case
                * the data is ready and audio playback is active.
                */
                if(!i2s_deinit_flag && i2s_data_ready_flag &&
                ((is_music_player_active() && !is_music_player_paused()) || first_frame))
                {
                    i2s_data_ready_flag = false;
                    i2s_playback_ptr = active_i2s_playback_buffer_ptr;
                    aec_ref_cb_ptr = aec_ref_temp_ptr;
                }

                /* Unless deinitialized, play the data over I2S and send the corresponding
                * AEC reference pointer to the AEC reference queue.
                */
                if(!i2s_deinit_flag)
                {
                    i2s_write_32_samples();
#ifdef ENABLE_IFX_AEC
                    /* Stop the I2S and Music player when the AEC Reference queue is
                    * almost full.
                    */
                    if (uxQueueMessagesWaiting(aec_ref_data_q) >
                        AEC_REF_DATA_QUEUE_CHECK_THRESHOLD)
                    {
                        app_log_print(">>> Error: Music Player Queue is almost full!\r\n");
                        music_player_q_data.cmd = I2S_PLAYBACK_STOP_MUSIC;
                        music_player_q_data.data = NULL;
                        music_player_q_data.data_len = 0;
                        if (pdTRUE != xQueueSendToFrontFromISR(music_player_task_q,
                                                            &music_player_q_data,
                                                            &xHigherPriorityTaskWoken))
                        {
                            app_log_print(">>> Error: Music Player Queue Send failed!\r\n");
                        }
                        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    else if (pdTRUE != xQueueSendFromISR(aec_ref_data_q, &aec_ref_cb_ptr,
                                                    &xHigherPriorityTaskWoken))
                    {
//                        app_log_print(">>> Error: I2S ISR AEC Ref Queue Send failed!\r\n");
                    }
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif /* ENABLE_IFX_AEC */
                }

                /* If the non-zero data is played, notify the I2S task for processing of
                * playback data of next frames.
                */
                if ((i2s_task_handle != NULL) &&
                    (i2s_playback_ptr == active_i2s_playback_buffer_ptr))
                {
                    xTaskNotifyFromISR(i2s_task_handle, 0, eNoAction,
                                    &xHigherPriorityTaskWoken);
                    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                }
            }
        }
        else 
        {
            if(is_music_player_paused())
            {
                if (i2s_skip_frame)
                {
                    if (i2s_task_handle != NULL)
                    {
                        xTaskNotifyFromISR(i2s_task_handle, 0, eNoAction,
                                           &xHigherPriorityTaskWoken);
                        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    }
                    i2s_skip_frame = false;
                }

                for(int i=0; i < HW_FIFO_SIZE; i++)
                {
                    /* Write same data for L,R channels(dual mono) in FIFO */
                    Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) 0);
                    i++;
                    Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) 0);
                }
            }
            else 
            {
                for(int i=0; i < HW_FIFO_SIZE; i++)
                {
                    /* Write same data for L,R channels(dual mono) in FIFO */
                    Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) *((i2s_playback_ptr + (HW_FIFO_SIZE * (i2s_32_samples_frame_count - 1))) + i));
                    i++;
                    Cy_AudioTDM_WriteTxData(TDM_STRUCT0_TX, (uint32_t) *((i2s_playback_ptr + (HW_FIFO_SIZE * (i2s_32_samples_frame_count - 1))) + i));
                }
                i2s_32_samples_frame_count++;
            }

        }
    }
    else if(CY_TDM_INTR_TX_FIFO_UNDERFLOW & intr_status)
    {
        app_log_print("Error: I2S transmit underflowed");
    }

    /* Clear all Tx I2S Interrupt */
    Cy_AudioTDM_ClearTxInterrupt(TDM_STRUCT0_TX, CY_TDM_INTR_TX_MASK);
}

/*******************************************************************************
* Function Name: i2s_playback_volume_control
********************************************************************************
* Summary:
*  Controls i2s playback volume
*
* Parameters:
*  volume: value between 0 to 127
*
* Return:
*  None
*******************************************************************************/
void i2s_playback_volume_control(uint8_t volume)
{
    if(is_music_player_active())
    {
        if(!first_frame)
        {
            mtb_tlv320dac3100_adjust_speaker_output_volume(volume);
        }
    }
    else
    {
#ifdef AUDIO_OUT
    i2s_init();
#endif /* AUDIO_OUT */
if(!first_frame)
    {
        mtb_tlv320dac3100_adjust_speaker_output_volume(volume);
    }
#ifdef AUDIO_OUT
        Cy_AudioTDM_DeActivateTx(TDM_STRUCT0_TX);
        Cy_AudioI2S_DisableTx(TDM_STRUCT0_TX);
#endif /* AUDIO_OUT */
    }
    i2s_playback_volume = volume;

}



/* [] END OF FILE */
