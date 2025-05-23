/******************************************************************************
* File Name : profiler.c
*
* Description :
* Code for MIPS profiler
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
#include "cy_retarget_io.h"
#include "cyabs_rtos.h"
#include "cy_device.h"
#include "profiler.h"
#include "cy_profiler.h"
//#define DISABLE_MCPS 1

#define PROFILE_USE_DWT_COUNT 1
/*******************************************************************************
* Macros
*******************************************************************************/

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void profiler_init(void);
#ifndef PROFILE_USE_DWT_COUNT
#ifndef DISABLE_MCPS
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);
#endif
#endif

/*******************************************************************************
* Global Variables
*******************************************************************************/

#ifndef PROFILE_USE_DWT_COUNT
/* Timer object used for blinking the LED */
static cyhal_timer_t led_blink_timer;
#endif

/* LED timer tick counter */
uint32_t timer_tick_cnt = 0;

/* Timer clock HZ */
uint32_t timer_hz;

/* CM4 system clock vs. Peripheral clock ratio */
int CM4_clk_ratio;

#ifndef DISABLE_MCPS
#ifndef PROFILE_USE_DWT_COUNT
static uint32_t start_time_val;
#endif
#endif
static uint32_t stop_time_val;
#ifdef MILLI_SEC_PROFILE
static cy_time_t start_time_ms, stop_time_ms;
#endif


#define RESET_CYCLE_CNT (DWT->CYCCNT=0)
#define GET_CYCLE_CNT (DWT->CYCCNT)

void DWTCyCNTInit(void)
{
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;
}

uint32_t Cy_Reset_Cycles(void)
{
    /* Call DWTCyCNTInit before first call */
    return RESET_CYCLE_CNT;
}

uint32_t Cy_Get_Cycles(void)
{
    /* Call DWTCyCNTInit before first call */
    return GET_CYCLE_CNT;
}

#if !PROFILE_USE_DWT_COUNT
/*******************************************************************************
* Function Name: get_led_timer_cnt
********************************************************************************
* Summary:
* This function get the led blink timer counter.
*
* Parameters:
*  none
*
*******************************************************************************/
uint32_t get_led_timer_cnt()
{
    return timer_tick_cnt;
}

cy_rslt_t set_cpu_clock ( uint32_t freq )
{
    cy_rslt_t ret = CY_RSLT_SUCCESS;
#if !defined COMPONENT_43907
    uint32_t old_freq;
    cyhal_clock_t clock_pll, clock_hf0 , clock_peri;

#if (CYHAL_API_VERSION >= 2)
    /* Support for HAL latest-v2.x */
    /* Take ownership of HF0 resource */
    cyhal_clock_reserve(&clock_hf0, &CYHAL_CLOCK_HF[0]);
#else
    /* Support for HAL latest-v1.x */
    /* Get the HF0 resource */
    cyhal_clock_get(&clock_hf0 , &CYHAL_CLOCK_HF[0] );
#endif

    old_freq = cyhal_clock_get_frequency(&clock_hf0);
    if ( freq != old_freq )
    {
#if (CYHAL_API_VERSION >= 2)
        /* Take ownership of PLL and PERI resource */
        CHECK_APP_RETURN(cyhal_clock_reserve(&clock_pll , &CYHAL_CLOCK_PLL[0]));
        CHECK_APP_RETURN(cyhal_clock_reserve(&clock_peri, &CYHAL_CLOCK_PERI));
#else
        /* Get the PLL and PERI resource */
        cyhal_clock_get(&clock_pll,  &CYHAL_CLOCK_PLL[0]);
        cyhal_clock_get(&clock_peri, &CYHAL_CLOCK_PERI);

        /* Initialize, take ownership of, the PLL instance */
        cyhal_clock_init(&clock_pll);
#endif
        /* Set CPU clock to freq */
        cyhal_clock_set_frequency(&clock_pll, freq, NULL);

        /* If the PLL is not already enabled, enable it */
        if (!cyhal_clock_is_enabled(&clock_pll))
        {
            cyhal_clock_set_enabled(&clock_pll, true, true);
        }

#if (CYHAL_API_VERSION < 2)
        /* Initialize, take ownership of, the PERI instance */
        cyhal_clock_init(&clock_peri);

        /* Initialize, take ownership of, the HF0 instance */
        cyhal_clock_init(&clock_hf0);
#endif
        /* Set peri clock divider */
        cyhal_clock_set_divider(&clock_peri, CPU_PERI_CLOCK_DIV);

        /* set HF0 Clock source to PLL */
        cyhal_clock_set_source(&clock_hf0, &clock_pll);

        cyhal_clock_free(&clock_pll);
        cyhal_clock_free(&clock_peri);
    }
    cyhal_clock_free(&clock_hf0);
#else
    cyhal_clock_t clk_cpu;
    cyhal_clock_get(&clk_cpu, &CYHAL_CLOCK_RSC_CPU);
    cyhal_clock_set_frequency(&clk_cpu, 320000000, 0);
#endif
    return ret;
}
#endif
/*******************************************************************************
* Function Name: profiler_init
********************************************************************************
* Summary:
* This function creates and configures a Timer object. The timer ticks
* continuously and produces a periodic interrupt on every terminal count
* event. The period is defined by the 'period' and 'compare_value' of the
* timer configuration structure 'led_blink_timer_cfg'. Without any changes,
* this application is designed to produce an interrupt every 1 second.
*
* Parameters:
*  none
*
*******************************************************************************/


 static void profiler_init(void)
 {
#if PROFILE_USE_DWT_COUNT
    DWTCyCNTInit();
#else
#ifndef DISABLE_MCPS
    cy_rslt_t result;
    cyhal_timer_cfg_t led_blink_timer_cfg =
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = 0,                        /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };

    uint32_t peri_bus_freq = Cy_SysClk_ClkPeriGetFrequency();
    CM4_clk_ratio = SystemCoreClock / peri_bus_freq;
    printf("Peripheral Bus Frequency = %"PRIu32", CM4 clock = %"PRIu32", CM4 clock ratio = %d\n", peri_bus_freq, SystemCoreClock, CM4_clk_ratio);

    /* Set timer clock equal to peripheral bus frequency which is half of CM4 maximum clock frequency */
    timer_hz = peri_bus_freq;
    led_blink_timer_cfg.period = timer_hz - 1;

    /* Initialize the timer object. Does not use pin output ('pin' is NC) and
     * does not use a pre-configured clock source ('clk' is NULL). */
    result = cyhal_timer_init(&led_blink_timer, NC, NULL);

    /* timer init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Configure timer period and operation mode such as count direction,
       duration */
    cyhal_timer_configure(&led_blink_timer, &led_blink_timer_cfg);

    /* Set the frequency of timer's clock source */
    cyhal_timer_set_frequency(&led_blink_timer, timer_hz);

    /* Assign the ISR to execute on timer interrupt */
    cyhal_timer_register_callback(&led_blink_timer, isr_timer, NULL);

    /* Set the event on which timer interrupt occurs and enable it */
    cyhal_timer_enable_event(&led_blink_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                              7, true);

    /* Start the timer with the configured settings */
    cyhal_timer_start(&led_blink_timer);
#endif /*  DISABLE_MCPS */

#endif /* PROFILE_USE_DWT_COUNT */
 }

#if !PROFILE_USE_DWT_COUNT
/*******************************************************************************
* Function Name: isr_timer
********************************************************************************
* Summary:
* This is the interrupt handler function for the timer interrupt and toggle the LED .
*
* Parameters:
*    callback_arg    Arguments passed to the interrupt callback
*    event            Timer/counter interrupt triggers
*
*******************************************************************************/
static void isr_timer(void *callback_arg, cyhal_timer_event_t event)
{
    (void) callback_arg;
    (void) event;

    timer_tick_cnt++;  //count # of seconds
    /* Invert the USER LED state */
    cyhal_gpio_toggle(CYBSP_USER_LED);
}
#endif
void Cy_SysLib_ProcessingFault(void)
{

    printf("\r\nCORE FAULT!!\r\n");
    printf("SCB->CFSR  = 0x%08"PRIx32"\r\n", (uint32_t) cy_faultFrame.cfsr.cfsrReg);
    printf("SCB->HFSR  = 0x%08"PRIx32"\r\n", (uint32_t) cy_faultFrame.hfsr.hfsrReg);
    printf("SCB->SHCSR = 0x%08"PRIx32"\r\n", (uint32_t) cy_faultFrame.shcsr.shcsrReg);

    /* If MemManage fault valid bit is set to 1, print MemManage fault address */
    if ((cy_faultFrame.cfsr.cfsrReg & SCB_CFSR_MMARVALID_Msk)
            == SCB_CFSR_MMARVALID_Msk)
    {
        printf("MemManage Fault! Fault address = 0x%08"PRIx32"\r\n", SCB->MMFAR);
    }

    /* If Bus Fault valid bit is set to 1, print BusFault Address */
    if ((cy_faultFrame.cfsr.cfsrReg & SCB_CFSR_BFARVALID_Msk)
            == SCB_CFSR_BFARVALID_Msk)
    {
        printf("Bus Fault! \r\nFault address = 0x%08"PRIx32"\r\n", SCB->BFAR);
    }

    /* Print Fault Frame */
    printf("r0  = 0x%08"PRIx32"\r\n", cy_faultFrame.r0);
    printf("r1  = 0x%08"PRIx32"\r\n", cy_faultFrame.r1);
    printf("r2  = 0x%08"PRIx32"\r\n", cy_faultFrame.r2);
    printf("r3  = 0x%08"PRIx32"\r\n", cy_faultFrame.r3);
    printf("r12 = 0x%08"PRIx32"\r\n", cy_faultFrame.r12);
    printf("lr  = 0x%08"PRIx32"\r\n", cy_faultFrame.lr);
    printf("pc  = 0x%08"PRIx32"\r\n", cy_faultFrame.pc);
    printf("psr = 0x%08"PRIx32"\r\n", cy_faultFrame.psr);

    while (1);
}

#if 0
int Cy_ML_Profile_Get_Tsc(uint32_t *val)
{
    uint32_t secs, cycles;
    secs = get_led_timer_cnt();
    cycles = cyhal_timer_read(&led_blink_timer) + timer_hz * secs;
    * val = CM4_clk_ratio * cycles; //convert to CM4 clock cycle number which is twice of peripheral bus clock cycles
    return 0;
}
#endif



static void start_time(void)
{
#ifndef DISABLE_MCPS
    #if PROFILE_USE_DWT_COUNT
        Cy_Reset_Cycles();
    #else
        Cy_ML_Profile_Get_Tsc(&start_time_val);
    #endif /* PROFILE_USE_DWT_COUNT */
#endif
#ifdef MILLI_SEC_PROFILE
    cy_rtos_get_time(&start_time_ms);
//  printf("Start ms :%lu\r\n",start_time_ms);
#endif

}

static void stop_time(void)
{
#ifndef DISABLE_MCPS
    #if PROFILE_USE_DWT_COUNT
        stop_time_val = Cy_Get_Cycles();
    #else
        Cy_ML_Profile_Get_Tsc(&stop_time_val);
    #endif /* PROFILE_USE_DWT_COUNT */
#endif

#ifdef MILLI_SEC_PROFILE
    cy_rtos_get_time(&stop_time_ms);
//  printf("Stop ms :%lu\r\n",stop_time_ms);
#endif
}

static uint32_t get_time(void)
{
#ifndef DISABLE_MCPS
    #if PROFILE_USE_DWT_COUNT
        return stop_time_val;
    #else
        uint32_t elapsed = stop_time_val - start_time_val;
        return elapsed;
    #endif /* PROFILE_USE_DWT_COUNT */

#endif
    return 0;
}

static uint32_t get_time_ms(void)
{
    uint32_t elapsed = 0;
#ifdef MILLI_SEC_PROFILE
    elapsed = stop_time_ms - start_time_ms;
#endif
    return elapsed;
}

#ifndef COMPONENT_CM33
#if 0
void display_mallinfo(void)
{
   struct mallinfo mi;

   mi = mallinfo();

   printf("Total non-mmapped bytes (arena):       %u\n", mi.arena);
   printf("# of free chunks (ordblks):            %u\n", mi.ordblks);
   printf("# of free fastbin blocks (smblks):     %u\n", mi.smblks);
   printf("# of mapped regions (hblks):           %u\n", mi.hblks);
   printf("Bytes in mapped regions (hblkhd):      %u\n", mi.hblkhd);
   printf("Max. total allocated space (usmblks):  %u\n", mi.usmblks);
   printf("Free bytes held in fastbins (fsmblks): %u\n", mi.fsmblks);
   printf("Total allocated space (uordblks):      %u\n", mi.uordblks);
   printf("Total free space (fordblks):           %u\n", mi.fordblks);
   printf("Topmost releasable block (keepcost):   %u\n", mi.keepcost);
}
#endif
#endif

void cy_profiler_init(void)
{
    profiler_init();
}

void cy_profiler_start(void)
{
    start_time();
}

void cy_profiler_stop(void)
{
    stop_time();
}

uint32_t cy_profiler_get_cycles(void)
{
    return(get_time());
}

uint32_t cy_profiler_get_time_ms(void)
{
    return get_time_ms();
}
/* [] END OF FILE */
