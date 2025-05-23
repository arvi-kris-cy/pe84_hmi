/******************************************************************************
* File Name : main.c
*
* Description :
*  main function for CM33 Non Secure application for Audio&Voice pipeline.
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
#include "mtb_hal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#include "app_logger.h"

#include "FreeRTOS.h"
#include "task.h"
#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
#include "battery_powered_app_lp.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#endif /* BATTERY_POWERED_LOCAL_VOICE */

#ifdef DEBUG_TUNE_BATTERY_APP
#include "tune_battery_powered_audio.h"
#include "cyabs_rtos.h"
#include "cyabs_rtos_impl.h"
#endif /* DEBUG_TUNE_BATTERY_APP */

/*******************************************************************************
* Macros
*******************************************************************************/
/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC          (10U)

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
/* Enabling or disabling a MCWDT requires a wait time of upto 2 CLK_LF cycles  
 * to come into effect. This wait time value will depend on the actual CLK_LF  
 * frequency set by the BSP.
 */
#define LPTIMER_0_WAIT_TIME_USEC            (62U)

/* Define the LPTimer interrupt priority number. '1' implies highest priority. 
 */
#define APP_LPTIMER_INTERRUPT_PRIORITY      (1U)
#endif /* BATTERY_POWERED_LOCAL_VOICE */
/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void app_retarget_io_init(void);

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
static void handle_error(void);
static void lptimer_interrupt_handler(void);
static void setup_tickless_idle_timer(void);
#endif /* BATTERY_POWERED_LOCAL_VOICE */
/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Debug UART context */
cy_stc_scb_uart_context_t  DEBUG_UART_context;
/* Debug UART HAL object  */
mtb_hal_uart_t             DEBUG_UART_hal_obj;

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
/* LPTimer HAL object */
static mtb_hal_lptimer_t lptimer_obj;
#endif /* BATTERY_POWERED_LOCAL_VOICE */
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function of the CM33 non-secure application.
*
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/

int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
    /* Setup the LPTimer instance for CM33 CPU. */
    setup_tickless_idle_timer();
#endif /* BATTERY_POWERED_LOCAL_VOICE */

    /* Initialize retarget-io */
    app_retarget_io_init();

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
    /* Initialize CM33 application with staged voice control if enabled in the audio pipeline configuration*/
    battery_powered_app_lp();
#endif /* BATTERY_POWERED_LOCAL_VOICE */

#ifdef DEBUG_TUNE_BATTERY_APP
    /* Debug/Tune battery powered audio pipeline */
    tune_battery_powered_audio();
#endif /* DEBUG_TUNE_BATTERY_APP */

    /* Enable CM55. */
    /* CY_CM55_APP_BOOT_ADDR must be updated if CM55 memory layout is changed.*/
    Cy_SysEnableCM55(MXCM55, CY_CM55_APP_BOOT_ADDR, 
                     CM55_BOOT_WAIT_TIME_USEC);

    app_log_print("\r\n: Enabling CM55 Done\r\n");
#if defined (BATTERY_POWERED_LOCAL_VOICE_APP) || defined (DEBUG_TUNE_BATTERY_APP)
    /* Start the RTOS Scheduler */
    vTaskStartScheduler();
#else
    /* For mains powered configuration, put CM33 in deep sleep. */
    for(;;)
    {
        // Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
    }
#endif /* BATTERY_POWERED_LOCAL_VOICE_APP / DEBUG_TUNE_BATTERY_APP */

    return CY_RSLT_SUCCESS; 

}


/*******************************************************************************
* Function Name: app_retarget_io_init
********************************************************************************
* Summary:
* This function initializes Debug UART
*
* Parameters:
*  none
*
* Return :
*  none
*
*******************************************************************************/
void app_retarget_io_init()
{
    cy_rslt_t result;

    result = (cy_rslt_t)Cy_SCB_UART_Init(CYBSP_DEBUG_UART_HW, &CYBSP_DEBUG_UART_config, &DEBUG_UART_context);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    Cy_SCB_UART_Enable(CYBSP_DEBUG_UART_HW);
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &CYBSP_DEBUG_UART_hal_config, &DEBUG_UART_context, NULL);
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);
    
}

#ifdef BATTERY_POWERED_LOCAL_VOICE_APP
/*******************************************************************************
* Function Name: lptimer_interrupt_handler
********************************************************************************
* Summary:
* Interrupt handler function for LPTimer instance. 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void lptimer_interrupt_handler(void)
{
    mtb_hal_lptimer_process_interrupt(&lptimer_obj);
}

/*******************************************************************************
* Function Name: setup_tickless_idle_timer
********************************************************************************
* Summary:
* 1. This function first configures and initializes an interrupt for LPTimer.
* 2. Then it initializes the LPTimer HAL object to be used in the RTOS 
*    tickless idle mode implementation to allow the device enter deep sleep 
*    when idle task runs. LPTIMER_0 instance is configured for CM33 CPU.
* 3. It then passes the LPTimer object to abstraction RTOS library that 
*    implements tickless idle mode
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void setup_tickless_idle_timer(void)
{
    /* Interrupt configuration structure for LPTimer */
    cy_stc_sysint_t lptimer_intr_cfg =
    {
        .intrSrc = CYBSP_CM33_LPTIMER_0_IRQ,
        .intrPriority = APP_LPTIMER_INTERRUPT_PRIORITY
    };

    /* Initialize the LPTimer interrupt and specify the interrupt handler. */
    cy_en_sysint_status_t interrupt_init_status = 
                                    Cy_SysInt_Init(&lptimer_intr_cfg, 
                                                    lptimer_interrupt_handler);
    
    /* LPTimer interrupt initialization failed. Stop program execution. */
    if(CY_SYSINT_SUCCESS != interrupt_init_status)
    {
        handle_error();
    }

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(lptimer_intr_cfg.intrSrc);

    /* Initialize the MCWDT block */
    cy_en_mcwdt_status_t mcwdt_init_status = 
                                    Cy_MCWDT_Init(CYBSP_CM33_LPTIMER_0_HW, 
                                                &CYBSP_CM33_LPTIMER_0_config);

    /* MCWDT initialization failed. Stop program execution. */
    if(CY_MCWDT_SUCCESS != mcwdt_init_status)
    {
        handle_error();
    }
  
    /* Enable MCWDT instance */
    Cy_MCWDT_Enable(CYBSP_CM33_LPTIMER_0_HW,
                    CY_MCWDT_CTR_Msk, 
                    LPTIMER_0_WAIT_TIME_USEC);

    /* Setup LPTimer using the HAL object and desired configuration as defined
     * in the device configurator. */
    cy_rslt_t result = mtb_hal_lptimer_setup(&lptimer_obj, 
                                            &CYBSP_CM33_LPTIMER_0_hal_config);
    
    /* LPTimer setup failed. Stop program execution. */
    if(CY_RSLT_SUCCESS != result)
    {
        handle_error();
    }

    /* Pass the LPTimer object to abstraction RTOS library that implements 
     * tickless idle mode 
     */
    cyabs_rtos_set_lptimer(&lptimer_obj);
}

/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
static void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}
#endif /* BATTERY_POWERED_LOCAL_VOICE */

/* [] END OF FILE */
