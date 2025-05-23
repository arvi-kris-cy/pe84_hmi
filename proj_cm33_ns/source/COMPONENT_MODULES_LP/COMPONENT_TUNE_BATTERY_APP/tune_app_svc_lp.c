
/******************************************************************************
* File Name : app_svc_lp.c
*
* Description :
* Code for CM33 non secure application - staged voice control.
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

#include "tune_app_svc_lp.h"

#include "audio_conv_utils.h"
#include "user_interaction.h"
#include "cy_staged_voice_control.h"
#include "cy_staged_voice_control_common.h"
#include "cycfg_pins.h"
#include "cybsp_types.h"
#include "audio_timeline_marker.h"
#include "audio_usb_send_utils.h"

#include "app_logger.h"
#include "audio_input_configuration.h"
#include "intercore_ipc.h"


/*******************************************************************************
* Macros
*******************************************************************************/

#define MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES     (160*2*2)
#define MONO_10MS_AUDIO_BYTES                           (160)
#define MIC_EXEC_START_DELAY_MS                         (50)
#define MIC_EXEC_START_DELAY_COUNTER                    (MIC_EXEC_START_DELAY_MS / (10))

#ifdef ENABLE_STEREO_INPUT_FEED
#define APP_INTER_CORE_BUF_SIZE                         (256000)
#else
#define APP_INTER_CORE_BUF_SIZE                         (128000)
#endif /* ENABLE_STEREO_INPUT_FEED */
#define APP_CFG_MAX_SHARED_BUF_SIZE                     (APP_INTER_CORE_BUF_SIZE)

#define LPWWD_DETECT_FRAME                              (200u)
#define SOD_HIT_DELAY                                   (500u)
#define SOD_SENSITIVITY                                 (16384u)
/*******************************************************************************
* Global Variables
*******************************************************************************/
int8_t noninterleave_audio[MAX_SIZE_FOR_NON_INTERLEAVE_STEREO_IN_BYTES] = { 0 };

static int delay_counter = 0;
int inf_count=0;
unsigned char app_data_shared_socmem[APP_CFG_MAX_SHARED_BUF_SIZE] __attribute__((section(".cy_shared_socmem")));

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

extern cy_rslt_t cy_svc_lp_feed( CY_SVC_DATA_T *data);
extern const char* svc_lp_events_to_printable_string(cy_svc_event_t event);
#ifdef INTERCORE_PIPE
extern void app_ipc_init_cm33(void);
#endif /* INTERCORE_PIPE */

/*******************************************************************************
* Function Name: audio_mic_data_feed_cm33
********************************************************************************
* Summary:
* API that receives audio data from mic (PDM) and sends it to SVC.
*
* Parameters:
*  audio_data - Audio data
*
* Return:
*  None
*
*******************************************************************************/

void audio_mic_data_feed_cm33(int16_t *audio_data)
{

    if(delay_counter < MIC_EXEC_START_DELAY_COUNTER)
    {
        delay_counter++;
        return;
    }
#ifdef ENABLE_STEREO_INPUT_FEED
/* Convert stereo audio to mono*/
    conv_stereo_to_non_interleaved((uint16_t *)audio_data,
            (uint16_t *)noninterleave_audio);

    cy_svc_lp_feed((CY_SVC_DATA_T *)noninterleave_audio);
    usb_send_out_dbg_put(USB_CHANNEL_1,(int16_t *)noninterleave_audio);
#else
    cy_svc_lp_feed((CY_SVC_DATA_T *)audio_data);
    usb_send_out_dbg_put(USB_CHANNEL_1,(int16_t *)audio_data);
#endif /* ENABLE_STEREO_INPUT_FEED */
}


/*******************************************************************************
* Function Name: usb_mic_data_feed
********************************************************************************
* Summary:
* API that receives audio data from mic (USB) and sends it to SVC.
*
* Parameters:
*  audio_data - Audio data
*
* Return:
*  None
*
*******************************************************************************/

void usb_mic_data_feed(int16_t *audio_data)
{

    cy_svc_lp_feed((CY_SVC_DATA_T *)audio_data);
    usb_send_out_dbg_put(USB_CHANNEL_1,(int16_t *)audio_data);
}

/*******************************************************************************
* Function Name: svc_lp_usb_marker
********************************************************************************
* Summary:
* Perform USB timeline marker updates based on staged voice control events.
*
* Parameters:
*  event - Events from SVC (staged voice control) middleware.
*
* Return:
*  None
*
*******************************************************************************/
void svc_lp_usb_marker(cy_svc_event_t event)
{
    switch(event)
    {

        case CY_SVC_EVENT_ACOUSTIC_ACTIVITY_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_AAD_DETECTED,NULL);
            break;
        }

        case CY_SVC_EVENT_SPEECH_ONSET_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_SOD_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_LPWWD_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_LPWWD_NOT_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_HPWWD_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_HPWWD_NOT_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_ASR_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_ASR_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_ASR_NOT_DETECTED:
        {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_ASR_NOT_DETECTED,NULL);
            break;
        }
        case CY_SVC_EVENT_ASR_PROCESSING_COMPLETED: {
            cy_audio_timeline_marker_update_status(CY_AUDIO_TIMELINE_ASR_PROCESS_COMPLETED,NULL);
            break;
        }
        default:
        {
            break;
        }
    }
    return;
}

/*******************************************************************************
* Function Name: svc_lp_perform_led
********************************************************************************
* Summary:
* Perform LED transitions based on staged voice control events.
*
* Parameters:
*  event - Events from SVC (staged voice control) middleware.
*
* Return:
*  None
*
*******************************************************************************/

void svc_lp_perform_led(cy_svc_event_t event)
{
    switch(event)
    {
        app_log_print("Event received is %d \r\n",event);
        case CY_SVC_EVENT_SPEECH_ONSET_DETECTED:
        {
            app_log_print("\r\nSpeech Onset Detection detected speech \r\n");
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED:
        {
            app_log_print("Wake Word detected in Low Power Core CM33 \r\n");
            app_log_print("Inference count %d \r\n",++inf_count);
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_ON);
#ifndef HP_INFERENCING_ENABLE
            app_log_print("Important: Inferencing Code on CM55 unavailable - Enable it for CE to work with all functionality. \r\n");
#endif /* HP_INFERENCING_ENABLE */
            break;
        }
        case CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_NOT_DETECTED:
        {
            app_log_print("Wake Word not detected in Low Power Core CM33 \r\n");
            app_log_print("Waiting for Wake Word \r\n");
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_ON);
            break;
        }
        case CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_NOT_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            app_log_print("Waiting for Wake Word \r\n");
            break;
        }
        case CY_SVC_EVENT_ASR_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_ON);
            break;
        }
        case CY_SVC_EVENT_ASR_NOT_DETECTED:
        {
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
            app_log_print("Waiting for Wake Word \r\n");
            break;
        }
        case CY_SVC_EVENT_ASR_PROCESSING_COMPLETED:
        {
            Cy_GPIO_Write(CYBSP_LED_RED_PORT, CYBSP_LED_RED_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_GREEN_PORT, CYBSP_LED_GREEN_PIN, CYBSP_LED_STATE_OFF);
            Cy_GPIO_Write(CYBSP_LED_BLUE_PORT, CYBSP_LED_BLUE_PIN, CYBSP_LED_STATE_OFF);
            app_log_print("Waiting for Wake Word \r\n");
            break;
        }
        default:
        {
            break;
        }
    }

    return;
}

/*******************************************************************************
* Function Name: svc_lp_app_rt_event_handler
********************************************************************************
* Summary:
* Event handler for SVC events.
*
* Parameters:
*  event - Events from SVC (staged voice control) middleware.
* callback_user_arg - NULL
*
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

cy_rslt_t svc_lp_app_rt_event_handler(
        cy_svc_event_t event,
        void *callback_user_arg)
{

    svc_lp_perform_led(event);
    svc_lp_usb_marker(event);
    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: app_svc_user_action_cb
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

void app_svc_user_action_cb(void)
{
    cy_svc_stage_t stage = CY_SVC_STAGE_UNKNOWN;
    cy_svc_lp_get_current_stage(&stage);

    app_log_print("Push-To-Talk : User button pressed - Waiting for ASR command \r\n");
    cy_svc_lp_set_stage(CY_SVC_STAGE_WAITING_FOR_ASR_REQUEST_DETECT);
    svc_lp_perform_led(CY_SVC_EVENT_LOW_POWER_WAKEUP_WORD_DETECTED);
    svc_lp_perform_led(CY_SVC_EVENT_HIGH_PERFORMANCE_WAKEUP_WORD_DETECTED);

}

/*******************************************************************************
* Function Name: app_push_to_talk_cm33_init
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

void app_push_to_talk_cm33_init(cb_user_action user_action_cb)
{
    user_interaction_init(USER_INTERACTION_BUTTON,user_action_cb);
}

/*******************************************************************************
* Function Name: app_svc_alloc_memory
********************************************************************************
* Summary:
* Callback to allocate memory for SVC MW.
*
* Parameters:
*  None
*
*******************************************************************************/


cy_rslt_t app_svc_alloc_memory(cy_svc_lp_mem_id_t mem_id,
        uint32_t size, void **buffer)
{
      cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;

      if(NULL == buffer)
      {
          return ret_val;
      }
      *buffer = NULL;

      switch(mem_id)
      {
          case CY_SVC_MEM_ID_INTERCORE_SHARED_BUFFER:
          {
              if(APP_CFG_MAX_SHARED_BUF_SIZE != size)
              {
                  app_log_print("Inter Core Shared Buffer mismatch %d: %ld",
                          (int)APP_CFG_MAX_SHARED_BUF_SIZE, (long)size);
                  break;
              }
              *buffer = &app_data_shared_socmem[0];
              memset(*buffer, 0 , size);
              ret_val = CY_RSLT_SUCCESS;
              break;
          }
          default:
          {
              *buffer = (void *)calloc(size,1);
               ret_val = CY_RSLT_SUCCESS;
              break;
          }
      }

      app_log_print("Alloc: MEMID:%d, Addr:%p, Size:%ld", (int)mem_id, *buffer,(long)size);

      return ret_val;
}


/*******************************************************************************
* Function Name: app_svc_free_memory
********************************************************************************
* Summary:
* Callback to free memory for SVC MW.
*
* Parameters:
*  None
*
*******************************************************************************/

cy_rslt_t app_svc_free_memory(cy_svc_lp_mem_id_t mem_id,
        void *buffer)
{
      cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;

      if(NULL == buffer)
      {
          return ret_val;
      }

      switch(mem_id)
      {
          case CY_SVC_MEM_ID_INTERCORE_SHARED_BUFFER:
          {
              break;
          }
          default:
          {
              free(buffer);
              ret_val = CY_RSLT_SUCCESS;
              break;
          }
      }
      buffer = NULL;
      app_log_print("Free: MEMID:%d, Addr:%p",mem_id, buffer);

      return ret_val;
}

/*******************************************************************************
* Function Name: app_init_svc
********************************************************************************
* Summary:
* Initialize the staged voice control middleware.
*
* Parameters:
*  None
*
* Return:
*  Status of SVC initialization.
*
*******************************************************************************/

cy_rslt_t app_init_svc(void)
{
    cy_rslt_t  ret_val = CY_RSLT_SVC_GENERIC_ERROR;
    cy_svc_lp_config_t init_svc = {0};

#ifdef INTERCORE_PIPE
    app_ipc_init_cm33();
#endif /* INTERCORE_PIPE */

    init_svc.is_lpwwd_external = false;

    init_svc.lpwwd_external_data_callback = NULL;
    init_svc.pre_roll_frame_count_from_lpwwd_detect_frame = LPWWD_DETECT_FRAME;
    init_svc.single_frame_time_ms = CY_SVC_SUPPORTED_FRAME_TIME_MS;

#ifdef ENABLE_STEREO_INPUT_FEED
    init_svc.audio_input_type = CY_SVC_AUDIO_INPUT_TYPE_STEREO;
#else
    init_svc.audio_input_type = CY_SVC_AUDIO_INPUT_TYPE_MONO;
#endif /* ENABLE_STEREO_INPUT_FEED */

    init_svc.event_callback = svc_lp_app_rt_event_handler;
    init_svc.callback_user_arg = NULL;
    init_svc.sample_rate = CY_SVC_SAMPLE_RATE_16KHZ;

    init_svc.sod_onset_detect_max_late_hit_delay_ms = SOD_HIT_DELAY;
    init_svc.sod_onset_gap_setting_ms = CY_SOD_ONSET_GAP_SETTING_400_MS;
    init_svc.sod_sensitivity = SOD_SENSITIVITY;
    init_svc.ipc_communication_sync_sempahore = 0;

    init_svc.stage_config_list = CY_SVC_ENABLE_SOD ;

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    app_log_print("HPF enabled at LP side \r\n");
    init_svc.stage_config_list |= CY_SVC_ENABLE_HPF;
#endif /* ENABLE_IFX_PRE_PROCESS_HPF */



#ifndef LPWWD_OVERRIDE
    init_svc.stage_config_list |= CY_SVC_ENABLE_LPWWD;
#endif /* LPWWD_OVERRIDE */

#ifdef HP_INFERENCING_ENABLE
    init_svc.stage_config_list |= CY_SVC_ENABLE_HPWWD_STATE_TRANSITIONS;
    
    init_svc.stage_config_list |= CY_SVC_ENABLE_ASR_STATE_TRANSITIONS;
#endif /*HP_INFERENCING_ENABLE */

    init_svc.ipc_pipe_config.lp_pipe_end_point_address = APP_LP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.lp_pipe_end_point_client_id = APP_LP_PIPE_END_POINT_CLIENT_ID_1;
    init_svc.ipc_pipe_config.lp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.lp_pipe_end_point_client_id);

    init_svc.ipc_pipe_config.hp_pipe_end_point_address = APP_HP_PIPE_END_POINT_ADDRESS;
    init_svc.ipc_pipe_config.hp_pipe_end_point_client_id = APP_HP_PIPE_END_POINT_CLIENT_ID_2;
    init_svc.ipc_pipe_config.hp_pipe_end_point_intr_mask =
            CY_IPC_INTR_MASK(init_svc.ipc_pipe_config.hp_pipe_end_point_client_id);

    init_svc.alloc_memory = app_svc_alloc_memory;
    init_svc.free_memory = app_svc_free_memory;
    init_svc.inter_core_shared_buffer_size = APP_INTER_CORE_BUF_SIZE;
    ret_val = cy_svc_lp_init(&init_svc);
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_log_print("Staged Voice Initialization failed");
        CY_ASSERT(0);
    }
    else
    {
        app_log_print("Staged Voice Initialization success");
    }
#ifdef ENABLE_MIC_INPUT_FEED
#ifdef ITSI_INFERENCING
    app_push_to_talk_cm33_init(app_svc_user_action_cb);
#endif /* Enable PTT only for ITSI */
#endif /* ENABLE_MIC_INPUT_FEED */
    return ret_val;
}

/*******************************************************************************
* Function Name: app_deinit_svc
********************************************************************************
* Summary:
* Deinitialize the staged voice control middleware.
*
* Parameters:
*  None
*
* Return:
*  Status of SVC deinitialization.
*
*******************************************************************************/

cy_rslt_t app_deinit_svc(void)
{
    cy_rslt_t ret_val = CY_RSLT_SUCCESS;

    ret_val = cy_svc_lp_deinit();
    if (ret_val != CY_RSLT_SUCCESS)
    {
        app_svc_log("Staged Voice Control deinit failed");
    }
    else
    {
        app_svc_log("Staged Voice control deinit success");
    }

    return ret_val;
}

/* [] END OF FILE */
