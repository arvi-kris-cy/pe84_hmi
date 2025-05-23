/******************************************************************************
* File Name : app_afe_tuner.c
*
* Description :
* Source file for Audio Front End tuning.
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

#ifdef CY_AFE_ENABLE_TUNING_FEATURE

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cybsp.h"
#include "cy_pdl.h"
#include "app_afe_tuner.h"
#include "cy_retarget_io.h"
#include "cy_result.h"
#include "cy_log.h"
#include "afe_app_log.h"
#include "audio_usb_send_utils.h"
#include "pdm_mic_interface.h"

/*******************************************************************************
* Macros
*******************************************************************************/
#define UART_READ_TIMEOUT_MS        (1u)

/*******************************************************************************
* Global Variables
*******************************************************************************/
int cmd_index = 0;

/*******************************************************************************
* Function Name: afe_tuner_settings_notify_cb
********************************************************************************
* Summary:
*  Tuner notification callback from AFE middleware.
*
* Parameters:
*  handle - AFE handle
*  config_setting - Configuration settings.
*  user_arg - User argument.
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

cy_rslt_t afe_tuner_settings_notify_cb(cy_afe_t handle, cy_afe_config_setting_t *config_setting, void *user_arg)
{
    static int stored_input_gain=0;
    int* data = NULL;
    if(CY_AFE_READ_CONFIG == config_setting->action)
    {
        if(CY_AFE_CONFIG_INPUT_GAIN == config_setting->config_name)
        {
            data = (int*) config_setting->value;
            app_afe_log("Read input gain");
            *data = stored_input_gain;
        }
    }
    else if (CY_AFE_UPDATE_CONFIG == config_setting->action)
    {
        if(CY_AFE_CONFIG_INPUT_GAIN == config_setting->config_name)
        {
            
            data = (int*) config_setting->value;
            stored_input_gain = *data;
            app_afe_log("Update input gain config. Input gain : %d", *data);
            stored_input_gain=stored_input_gain*2;
            if (stored_input_gain>=PDM_PCM_MIN_GAIN && stored_input_gain<=PDM_PCM_MAX_GAIN)
            {
                app_afe_log("Setting input gain to %d \r\n",stored_input_gain);
                set_pdm_pcm_gain((int16_t)stored_input_gain);
            }
            else
            {
                app_afe_log("PDM Gain out of bounds (-103 to 82) dB \r\n");
            }
        }
        else if(CY_AFE_CONFIG_STREAM == config_setting->config_name)
        {
            data = (int*) config_setting->value;
            app_afe_log("data is %d \r\n",*data);
        }
    }
    else
    {
        // Notify setting
    }

    return CY_RSLT_SUCCESS;

}

/*******************************************************************************
* Function Name: afe_tuner_read_req_cb
********************************************************************************
* Summary:
*  Callback for AFE read operation.
*
* Parameters:
*  handle - AFE handle
*  request_buffer - Buffer with read data.
*  user_arg - User argument.
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

cy_rslt_t afe_tuner_read_req_cb(cy_afe_t handle, cy_afe_tuner_buffer_t *request_buffer, void *user_arg)
{
    uint32_t read_value = Cy_SCB_UART_Get(CYBSP_DEBUG_UART_HW);
    if (CY_SCB_UART_RX_NO_DATA == read_value)
    {
        vTaskDelay(pdMS_TO_TICKS(UART_READ_TIMEOUT_MS));
        read_value = Cy_SCB_UART_Get(CYBSP_DEBUG_UART_HW);
    }

    if (CY_SCB_UART_RX_NO_DATA != read_value)
    {
        request_buffer->buffer[0] = (uint8_t)read_value;
        request_buffer->length = 1;
    }
    else
    {
        request_buffer->length = 0;
        request_buffer->buffer = NULL;
    }
    return CY_RSLT_SUCCESS;
}

/*******************************************************************************
* Function Name: afe_tuner_write_res_cb
********************************************************************************
* Summary:
*  Callback for AFE write operation.
*
* Parameters:
*  handle - AFE handle
*  response_buffer - Buffer with write data.
*  user_arg - User argument.
* Return:
*  CY_RSLT_SUCCESS
*
*******************************************************************************/

cy_rslt_t afe_tuner_write_res_cb(cy_afe_t handle, cy_afe_tuner_buffer_t *response_buffer, void *user_arg)
{
    Cy_SCB_UART_PutArrayBlocking(CYBSP_DEBUG_UART_HW, response_buffer->buffer, response_buffer->length);
    return CY_RSLT_SUCCESS;
}
#endif /* CY_AFE_ENABLE_TUNING_FEATURE */

/* [] END OF FILE */
