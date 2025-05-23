/******************************************************************************
* File Name : mains_powered_afe.c
*
* Description :
* Source file for Audio Front End in CM55 application.
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

#include <FreeRTOS.h>
#include <task.h>
#include "cyabs_rtos.h"
#include "cy_audio_front_end.h"
#include "cy_result.h"
#include "cy_log.h"
#include "cy_buffer_pool.h"
#include "cyabs_rtos_internal.h"
#include "mains_powered_afe.h"
#include "mains_powered_inferencing.h"

#include "app_logger.h"
#include "cy_afe_configurator_settings.h"
#include "afe_app_defines.h"
#include "afe_app_log.h"
#include "cy_audio_front_end_error.h"

#include "app_logger.h"


/*******************************************************************************
* Macros
*******************************************************************************/

#ifdef ENABLE_IFX_AEC
#define AFE_ALGO_PERSISTENT_MEMORY                     (160000)
#else
#define AFE_ALGO_PERSISTENT_MEMORY                     (60000)
#endif /* ENABLE_IFX_AEC */
#define AFE_ALGO_SCRATCH_MEMORY                        (25000)
#define AFE_OUTPUT_BUFFER_MEMORY                       (320)
#define AFE_APP_TEMP_MEMORY                            (2)


/*******************************************************************************
* Global Variables
*******************************************************************************/
cy_afe_t afe_handle = NULL;



uint8_t app_afe_persistent_memory[AFE_ALGO_PERSISTENT_MEMORY] __attribute__((section(".dtcm_data"), aligned(4)));
uint8_t app_afe_scratch_memory[AFE_ALGO_SCRATCH_MEMORY] __attribute__((section(".dtcm_data"), aligned(4)));
unsigned char afe_output_buffer[AFE_OUTPUT_BUFFER_MEMORY] __attribute__((section(".dtcm_data"), aligned(4)));
uint8_t app_afe_temp_mem[AFE_APP_TEMP_MEMORY] = {0};

void *dsns_mem = NULL;

extern QueueHandle_t infer_queue_handle;
/*******************************************************************************
* Function Name: license_limitation_exit
********************************************************************************
* Summary:
*  This function is blocking call after Audio Front End license timeout
*
*******************************************************************************/
void license_limitation_exit()
{
    CY_ASSERT(0);
}

/*******************************************************************************
* Function Name: afe_output_callback
********************************************************************************
* Summary:
* Output callback for AFE from MW.
*
* Parameters:
*  handle - AFE handle.
*  output_buffer_info - Output buffer from MW.
*  user_arg - User argument (unused).
* 
* Return:
*  None
*
*******************************************************************************/

static cy_rslt_t afe_output_callback(cy_afe_t handle, cy_afe_buffer_info_t *output_buffer_info, void *user_arg)
{
    int16_t *output_buffer = (int16_t*)output_buffer_info->output_buf;
    cy_rslt_t result = CY_RSLT_SUCCESS;
    BaseType_t ret = pdTRUE;
    
    if (infer_queue_handle !=NULL)
    {
        ret = xQueueSend(infer_queue_handle, (void*)output_buffer, 0);
    }
    if (pdTRUE != ret)
    {
        app_log_print(">>> Send failed to inferencing task - Queue full \r\n");
    }

    return result;
}

/*******************************************************************************
* Function Name: afe_get_output_buffer_cb
********************************************************************************
* Summary:
* Output callback for AFE from MW.
*
* Parameters:
*  context - Context (unused)
*  output_buffer - Get a free buffer.
*  user_arg - User argument (unused).
* 
* Return:
*  Result of get queue operation.
*
*******************************************************************************/

static cy_rslt_t afe_get_output_buffer_cb(cy_afe_t context, uint32_t **output_buffer, void *user_args)
{
    *output_buffer = (uint32_t*)afe_output_buffer;
    return CY_RSLT_SUCCESS;

}
/*******************************************************************************
* Function Name: app_afe_alloc_memory
********************************************************************************
* Summary:
* Callback from middle-ware to allocate memory for AFE.
*
* Parameters:
*  None
*
* Return:
*  Result of memory allocation.
*
*******************************************************************************/

cy_rslt_t app_afe_alloc_memory(cy_afe_mem_id_t mem_id, uint32_t size, void **buffer)
{
      cy_rslt_t  ret_val = CY_RSLT_SUCCESS;

      if(NULL == buffer)
      {
          return ret_val;
      }
      *buffer = NULL;
      app_log_print("app afe alloc size %ld \r\n",(long) size);
      if (size == 0)
      {
          app_log_print("No allocation as size is zero \r\n");
          *buffer = &app_afe_temp_mem[0];
          return ret_val;
      }
      switch(mem_id)
      {

          case CY_AFE_MEM_ID_ALGORITHM_NS_MEMORY:
          {
              app_log_print("DSNS Memory requires %ld bytes \r\n",(long)size);
              dsns_mem = calloc(size+15,1);
              if (dsns_mem == NULL)
              {
                 app_log_print("DSNS memory allocation failed \r\n");
                 ret_val = -1;
              }
              else
              {
                  *buffer = (void *) (((uintptr_t)dsns_mem+15) & ~ (uintptr_t)0x0F);
              }
              break;
          }
          case CY_AFE_MEM_ID_ALGORITHM_PERSISTENT_MEMORY:
          {
              app_log_print("Persistent Memory requires %ld bytes \r\n",(long)size);
              if(size > AFE_ALGO_PERSISTENT_MEMORY)
              {
                  app_log_print("Defaulting to heap allocation for persistent memory \r\n");
                  *buffer = (void *)calloc(size,1);
                  if (*buffer == NULL)
                  {
                      app_log_print("Persistent memory allocation failed \r\n");
                      ret_val = -1;
                  }
              }
              else
              {
                  *buffer = &app_afe_persistent_memory[0];
                  memset(*buffer, 0 , size);
              }
              break;
          }
          case CY_AFE_MEM_ID_ALGORITHM_SCRATCH_MEMORY:
          {
              app_log_print("Scratch Memory requires %ld bytes \r\n",(long)size);
              if(size > AFE_ALGO_SCRATCH_MEMORY)
              {
                  app_log_print("Defaulting to heap allocation for scratch memory \r\n");
                  *buffer = (void *)calloc(size,1);
                  if (*buffer == NULL)
                  {
                      app_log_print("Scratch memory allocation failed \r\n");
                      ret_val = -1;
                  }
              }
              else
              {
                  *buffer = &app_afe_scratch_memory[0];
                  memset(*buffer, 0 , size);
              }
              break;
          }
          default:
          {
              *buffer = (void *)calloc(size,1);
              if (*buffer == NULL)
              {
                  app_log_print("AFE memory allocation failed \r\n");
                  ret_val = -1;
              }
              break;
          }
      }

      return ret_val;

}

/*******************************************************************************
* Function Name: app_afe_free_memory
********************************************************************************
* Summary:
* Callback from middle-ware to free memory for AFE.
*
* Parameters:
*  None
*
* Return:
*  Result of freeing memory.
*
*******************************************************************************/

cy_rslt_t app_afe_free_memory(cy_afe_mem_id_t mem_id, void *buffer)
{
    cy_rslt_t  ret_val = CY_RSLT_SUCCESS;

    if (buffer != NULL)
    {
          free(buffer);
          buffer=NULL;
    }
    return ret_val;

}

/*******************************************************************************
* Function Name: mains_powered_afe_mw_init
********************************************************************************
* Summary:
* Initialize AFE MW for mains powered application.
*
* Parameters:
*  None
* 
* Return:
*  Result of AFE initialization.
*
*******************************************************************************/

int mains_powered_afe_mw_init()
{
    cy_afe_config_t afe_config = {0};
    cy_rslt_t result = CY_RSLT_SUCCESS;

    afe_config.filter_settings = AFE_FILTER_SETTINGS;
    afe_config.mw_settings = NULL;
    afe_config.afe_get_buffer_callback = afe_get_output_buffer_cb;
    afe_config.afe_output_callback = afe_output_callback;
    afe_config.user_arg_callbacks = NULL;

#if AFE_MW_SETTINGS_SIZE
    afe_config.mw_settings = AFE_MW_SETTINGS;
    afe_config.mw_settings_length = AFE_MW_SETTINGS_SIZE;
#else
    afe_config.mw_settings = NULL;
    afe_config.mw_settings_length = 0;
#endif /* AFE_MW_SETTINGS_SIZE */

    afe_config.alloc_memory = app_afe_alloc_memory;
    afe_config.free_memory = app_afe_free_memory;
    /* Create AFE instance (AFE Handle) */
    result = cy_afe_create(&afe_config, &afe_handle);
    if(CY_RSLT_SUCCESS != result)
    {
        app_afe_log("Failed to initialize AFE instance");
        CY_ASSERT(0);
    }

    app_log_print("AFE initialized \r\n");

    return result;
}

/*******************************************************************************
* Function Name: mains_powered_afe_mw_feed
********************************************************************************
* Summary:
*  Feed mic audio data and AEC reference to AFE middleware.
*
*******************************************************************************/

int mains_powered_afe_mw_feed(void* audio_input, void* aec_buffer)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;
    if (is_inference_license_valid() == -1) 
    {
        license_limitation_exit();
    }

    result = cy_afe_feed(afe_handle, (int16_t*)audio_input, (int16_t*)aec_buffer);

    if (CY_RSLT_AFE_FUNCTIONALITY_RESTRICTED == result)
    {
        app_log_print("CPU Halt: Audio Front End Restricted License Timeout - Reset the board \r\n");
        license_limitation_exit();
        return result;
    }
    if(CY_RSLT_SUCCESS != result)
    {
        app_log_print("Failed to feed data to AFE \r\n");
        return result;
    }

    return result;
}

/* [] END OF FILE */
