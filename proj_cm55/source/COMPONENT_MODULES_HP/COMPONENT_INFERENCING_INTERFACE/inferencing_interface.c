/******************************************************************************
* File Name : inferencing_interface.c
*
* Description :
* Source file for Inference processing on CM55.
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
#include "inferencing_interface.h"
#ifdef ITSI_INFERENCING
#include "ifx_sp_common.h"
#include "ifx_itsi.h"
#include "cy_result.h"
#include "itsi_inferencing.h"
#include <stdio.h>
#endif /* ITSI_INFERENCING */
#include "app_logger.h"

#ifdef SENSORY_INFERENCING
#include "sensory_inferencing.h"
/* RTC dependencies */
#include "cy_time.h"
#include "mtb_hal.h"
#include "cybsp.h"
#endif /* SENSORY_INFERENCING */


/*******************************************************************************
* Macros
*******************************************************************************/
#ifdef ITSI_INFERENCING
#ifdef BATTERY_POWERED_LOCAL_VOICE
#define ITSI_DIGITAL_GAIN                 (2)
#else
#define ITSI_DIGITAL_GAIN                 (1)
#endif /* BATTERY_POWERED_LOCAL_VOICE */
#define ITSI_AUDIO_SAMPLES                (160)
#endif /* ITSI_INFERENCING */

#ifdef SENSORY_INFERENCING
#define SNSR_DIGITAL_GAIN                 (1)
#define SNSR_AUDIO_SAMPLES                (160)

#define RTC_ACCESS_RETRY_COUNT            (500U)
#define RTC_RETRY_DELAY_MS                (5U)

#endif /* SENSORY_INFERENCING */

#define WAKE_WORD_INFER_STAGE             (1)
#define ASR_INFER_STAGE                   (2)
/*******************************************************************************
* Global Variables
*******************************************************************************/
#ifdef ITSI_INFERENCING
short boosted_data[ITSI_AUDIO_SAMPLES] __attribute__((section(".dtcm_data"), aligned(4))) = {0};
#else
short boosted_data[SNSR_AUDIO_SAMPLES] __attribute__((section(".dtcm_data"), aligned(4))) = {0};
#endif /*ITSI_INFERENCING*/

#ifdef ITSI_INFERENCING
short inferencing_stage = 0;
int inferencing_frame_count_ww = 0;
int inferencing_frame_count_asr = 0;
inferencing_callback g_callback_function;
extern void app_inferencing_callback(const char *function, char *message, char *parameter);
#endif /* ITSI_INFERENCING */


#ifdef SENSORY_INFERENCING
SnsrSession session = NULL;

/*******************************************************************************
* Function Name: rtc_init
********************************************************************************
* Summary:
*    RTC for Sensory Inferencing Engine.
*
*******************************************************************************/

cy_en_rtc_status_t rtc_init(void)
{
    /* Variable used to store return status of RTC API */
    cy_en_rtc_status_t rtc_status;
    uint32_t rtc_access_retry = RTC_ACCESS_RETRY_COUNT;

    static mtb_hal_rtc_t mtb_timer_rtc;
    struct tm *loc_time;
    time_t seconds, new_time_seconds;

    /* RTC block doesn't allow to access, when synchronizing the user registers
    * and the internal actual RTC registers. It will return RTC_BUSY value, if
    * it is not available to update the configuration values. Needs to retry,
    * if it doesn't return CY_RTC_SUCCESS. */

    do
    {
        rtc_status = Cy_RTC_Init(&CYBSP_RTC_config);
        rtc_access_retry--;
        Cy_SysLib_Delay(RTC_RETRY_DELAY_MS);
    } while((rtc_status != CY_RTC_SUCCESS) && (rtc_access_retry != 0));

    Cy_RTC_SetDateAndTimeDirect(00, 00, 00,01,01,25);

    new_time_seconds = 1614231319;

    /* Convert time_t to tm format */
    loc_time = localtime(&new_time_seconds);

    /* Write new time to RTC */
    mtb_hal_rtc_write(&mtb_timer_rtc, loc_time);
    mtb_clib_support_init(&mtb_timer_rtc);


    return rtc_status;
}
#endif /*SENSORY_INFERENCING */

/*******************************************************************************
* Function Name: inferencing_engine_init
********************************************************************************
* Summary:
*    Initializes the inferencing engine.
*
*******************************************************************************/
int inferencing_engine_init(inferencing_callback callback_function)
{

#ifdef ITSI_INFERENCING
    uint32_t status=0;

#ifdef WWD_ONLY
    status = ifx_init_itsi_wwd(WWmodeldata);
#else
    status = ifx_init_itsi(WWmodeldata, CMDmodeldata, NMBmodeldata);
#endif
    app_log_print("ITSI init returned %d \r\n",status);
    if (status != 0)
    {
        printf("Error! ITSI init Failed!! Error code=%x, Component index=%d, Line number=%d, exiting ITSI!\r\n",
            IFX_SP_ENH_ERR_CODE(status), IFX_SP_ENH_ERR_COMPONENT_INDEX(status), IFX_SP_ENH_ERR_LINE_NUMBER(status));
        return -1;
    }
    g_callback_function=callback_function;

#endif /* ITSI_INFERENCING */

#ifdef SENSORY_INFERENCING
    /* RTC must be initialized before Sensory library initialization */
    rtc_init();

    session=sensory_init();
    if (session == NULL)
    {
        app_log_print("Sensory Init failed \r\n");
    }
#endif
    return 1;

}

/*******************************************************************************
* Function Name: inferencing_engine_deinit
********************************************************************************
* Summary:
*    De-initializes the inferencing engine.
*
*******************************************************************************/
void inferencing_engine_deinit()
{
#ifdef ITSI_INFERENCING
#ifdef WWD_ONLY
    ifx_reset_itsi_wwd();
    ifx_free_itsi_wwd();
#else
    ifx_reset_itsi();
    ifx_free_itsi();
#endif
    return;
#else
    return;
#endif /*ITSI_INFERENCING */

}


/*******************************************************************************
* Function Name: inference_processing
********************************************************************************
* Summary:
*    Perform inferencing on the data.
*
*******************************************************************************/
void inference_processing(short *data, int num_samples)
{

#ifdef ITSI_INFERENCING

    int index = 0;
    uint32_t status = 0;
    int wwd_result, cmd_result;

    wwd_result = cmd_result = 0;

    for (index = 0; index < ITSI_AUDIO_SAMPLES; index++)
    {
        boosted_data[index] = data[index]*ITSI_DIGITAL_GAIN;
    }
    inferencing_frame_count_ww++;
    inferencing_frame_count_asr++;
#ifdef WWD_ONLY
    status = ifx_run_itsi_wwd(boosted_data, &wwd_result);
#else
    status = ifx_run_itsi(boosted_data, false, &wwd_result, &cmd_result);
#endif

    if (status)
    {
        app_log_print("Error! ITSI run Failed!! Error code=%x, Component index=%d, Line number=%d, exiting ITSI!\r\n",
            IFX_SP_ENH_ERR_CODE(status), IFX_SP_ENH_ERR_COMPONENT_INDEX(status), IFX_SP_ENH_ERR_LINE_NUMBER(status));
        return;
    }
    itsi_process_results(wwd_result,cmd_result);
#endif /* ITSI_INFERENCING */

#ifdef SENSORY_INFERENCING
    int index = 0;
    for (index = 0; index < SNSR_AUDIO_SAMPLES; index++)
    {
        boosted_data[index] = data[index]*SNSR_DIGITAL_GAIN;
    }
    sensory_processing(session, boosted_data, num_samples*2);
#endif /* SENSORY_INFERENCING */
    return;

}

/*******************************************************************************
* Function Name: is_inference_license_valid
********************************************************************************
* Summary:
*    Check license of inference library.
*
*******************************************************************************/

int is_inference_license_valid()
{
    return 1;
}

#ifdef BATTERY_POWERED_LOCAL_VOICE
/*******************************************************************************
* Function Name: get_current_inference_stage
********************************************************************************
* Summary:
*    Get the current inferencing stage (WWD or ASR).
*
*******************************************************************************/
int get_current_inference_stage()
{

#ifdef ITSI_INFERENCING
    return inferencing_stage;
#endif /*ITSI_INFERENCING */


}

/*******************************************************************************
* Function Name: set_current_inference_stage
********************************************************************************
* Summary:
*    Set the current inferencing stage (WWD or ASR).
*
*******************************************************************************/
int set_current_inference_stage(int stage)
{

#ifdef ITSI_INFERENCING
    inferencing_stage = stage;
    return 0;
#endif /*ITSI_INFERENCING */

}

/*******************************************************************************
* Function Name: switch_model_to_wwd
********************************************************************************
* Summary:
*    Switch the model to perform WWD.
*
*******************************************************************************/

int switch_model_to_wwd()
{
    return 0;
}

/*******************************************************************************
* Function Name: switch_model_to_asr
********************************************************************************
* Summary:
*    Switch the model to perform ASR.
*
*******************************************************************************/

int switch_model_to_asr()
{
    return 0;
}


/*******************************************************************************
* Function Name: switch_to_wwd_inferencing
********************************************************************************
* Summary:
*    Switch the counters for WWD inferencing.
*
*******************************************************************************/
int switch_to_wwd_inferencing()
{

#ifdef ITSI_INFERENCING
    inferencing_frame_count_ww=0;
    app_inferencing_disable_streaming_state();
    set_current_inference_stage(WAKE_WORD_INFER_STAGE);
#endif /*ITSI_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_asr_inferencing
********************************************************************************
* Summary:
*    Switch the counters for ASR inferencing.
*
*******************************************************************************/
int switch_to_asr_inferencing()
{

#ifdef ITSI_INFERENCING
    inferencing_frame_count_asr=0;
    app_inferencing_disable_streaming_state();
    set_current_inference_stage(ASR_INFER_STAGE);
#endif /*ITSI_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_wwd_inferencing_streaming
********************************************************************************
* Summary:
*    Switch the counters for WWD in streaming state.
*
*******************************************************************************/
int switch_to_wwd_inferencing_streaming()
{

#ifdef ITSI_INFERENCING
    inferencing_frame_count_ww=0;
    app_inferencing_enable_streaming_state();
    set_current_inference_stage(WAKE_WORD_INFER_STAGE);
#endif /*ITSI_INFERENCING */
    return 0;

}

/*******************************************************************************
* Function Name: switch_to_asr_inferencing_streaming
********************************************************************************
* Summary:
*    Switch the counters for ASR in streaming state.
*
*******************************************************************************/
int switch_to_asr_inferencing_streaming()
{

#ifdef ITSI_INFERENCING
    inferencing_frame_count_asr=0;
    app_inferencing_enable_streaming_state();
    set_current_inference_stage(ASR_INFER_STAGE);
#endif /*ITSI_INFERENCING */
    return 0;
}

/*******************************************************************************
* Function Name: get_inferencing_frame_counters
********************************************************************************
* Summary:
*    Get the frame count that is being sent for inferencing.
*
*******************************************************************************/

void get_inferencing_frame_counters(unsigned int *current_feed_counter_stage1,unsigned int *current_feed_counter_stage2 )
{

#ifdef ITSI_INFERENCING
    *current_feed_counter_stage1=inferencing_frame_count_ww;
    *current_feed_counter_stage2=inferencing_frame_count_asr;
    return;
#endif /*ITSI_INFERENCING */

}

/*******************************************************************************
* Function Name: inference_reset_wwd_feed_timeout_counter
********************************************************************************
* Summary:
*    Reset frame counter.
*
*******************************************************************************/
void inference_reset_wwd_feed_timeout_counter()
{

#ifdef ITSI_INFERENCING
    inferencing_frame_count_ww=0;
    inferencing_frame_count_asr=0;
    return;
#endif /*ITSI_INFERENCING */

}

/*******************************************************************************
* Function Name: inference_timeout_check
********************************************************************************
* Summary:
*    Check for timeout.
*
*******************************************************************************/
void inference_timeout_check()
{

#ifdef ITSI_INFERENCING
    if (get_current_inference_stage()==WAKE_WORD_INFER_STAGE)
    {
        if (inferencing_frame_count_ww==200)
        {
            app_request_post_wwd_data();
        }
        else if (inferencing_frame_count_ww==250)
        {
            app_inferencing_callback(__func__,"Timeout",NULL);
        }
    }
    return;
#endif /*ITSI_INFERENCING */

}
#endif /* BATTERY_POWERED_LOCAL_VOICE */
/* [] END OF FILE */
