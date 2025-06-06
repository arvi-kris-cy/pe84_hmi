/***************************************************************************//**
* \file ns_system_pse84.c
* \version 1.0
*
* The device system-source file.
*
********************************************************************************
* \copyright
* Copyright 2016-2025 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include <stdbool.h>
#include "system_pse84.h"
#include "cy_syslib.h"
#include "cy_wdt.h"
#include "cy_sysclk.h"
#include "cy_syspm.h"
#include "cy_syspm_pdcm.h"
#if defined(CY_USE_RPC_CALL) && (CY_USE_RPC_CALL == 1)
#include "cy_secure_services.h"
#endif
/*******************************************************************************
* SystemCoreClockUpdate()
*******************************************************************************/

/** Default HFClk frequency in Hz */
#define CY_CLK_HFCLK0_FREQ_HZ_DEFAULT       (50000000UL)

/** Default PeriClk frequency in Hz */
#define CY_CLK_PERICLK_FREQ_HZ_DEFAULT      (50000000UL)

/** Default system core frequency in Hz */
#ifdef PSE84_PSVP
#define CY_CLK_SYSTEM_FREQ_HZ_DEFAULT       (12000000UL)
#else
#define CY_CLK_SYSTEM_FREQ_HZ_DEFAULT       (50000000UL)
#endif

#define RRAM_NVM_PROTECTED_BLOCKS_TO_CONFIG 32 // 128KB/4KB  4KB block size

/**
* Holds the (Cortex-M33) system core clock,
* which is the system clock frequency supplied to the SysTick timer and the
* processor core clock.
* This variable implements CMSIS Core global variable.
* Refer to the [CMSIS documentation]
* (http://www.keil.com/pack/doc/CMSIS/Core/html/group__system__init__gr.html "System and Clock Configuration")
* for more details.
* This variable can be used by debuggers to query the frequency
* of the debug timer or to configure the trace clock speed.
*
* \attention Compilers must be configured to avoid removing this variable in case
* the application program is not using it. Debugging systems require the variable
* to be physically present in memory so that it can be examined to configure the debugger. */
uint32_t SystemCoreClock = CY_CLK_SYSTEM_FREQ_HZ_DEFAULT;

/** Holds the HFClk0 clock frequency. Updated by \ref SystemCoreClockUpdate(). */
uint32_t cy_Hfclk0FreqHz  = CY_CLK_HFCLK0_FREQ_HZ_DEFAULT;

/** Holds the PeriClk clock frequency. Updated by \ref SystemCoreClockUpdate(). */
uint32_t cy_PeriClkFreqHz = CY_CLK_PERICLK_FREQ_HZ_DEFAULT;

/** Holds the AHB frequency. Updated by \ref SystemCoreClockUpdate(). */
uint32_t cy_AhbFreqHz = CY_CLK_SYSTEM_FREQ_HZ_DEFAULT;

/*******************************************************************************
* SystemCoreClockUpdate (void)
*******************************************************************************/

/* Do not use these definitions directly in your application */
#define CY_DELAY_MS_OVERFLOW_THRESHOLD  (0x8000u)
#define CY_DELAY_1K_THRESHOLD           (1000u)
#define CY_DELAY_1K_MINUS_1_THRESHOLD   (CY_DELAY_1K_THRESHOLD - 1u)
#define CY_DELAY_1M_THRESHOLD           (1000000u)
#define CY_DELAY_1M_MINUS_1_THRESHOLD   (CY_DELAY_1M_THRESHOLD - 1u)
uint32_t cy_delayFreqHz   = CY_CLK_SYSTEM_FREQ_HZ_DEFAULT;

uint32_t cy_delayFreqKhz  = (CY_CLK_SYSTEM_FREQ_HZ_DEFAULT + CY_DELAY_1K_MINUS_1_THRESHOLD) /
                            CY_DELAY_1K_THRESHOLD;

uint32_t cy_delayFreqMhz  = (uint8_t)((CY_CLK_SYSTEM_FREQ_HZ_DEFAULT + CY_DELAY_1M_MINUS_1_THRESHOLD) /
                            CY_DELAY_1M_THRESHOLD);


#define CY_ROOT_PATH_SRC_IMO                (0UL)
#define CY_ROOT_PATH_SRC_EXT                (1UL)
#if (SRSS_ECO_PRESENT == 1U)
    #define CY_ROOT_PATH_SRC_ECO            (2UL)
#endif /* (SRSS_ECO_PRESENT == 1U) */
#if (SRSS_ALTHF_PRESENT == 1U)
    #define CY_ROOT_PATH_SRC_ALTHF          (3UL)
#endif /* (SRSS_ALTHF_PRESENT == 1U) */
#define CY_ROOT_PATH_SRC_DSI_MUX            (4UL)
#define CY_ROOT_PATH_SRC_DSI_MUX_HVILO      (16UL)
#define CY_ROOT_PATH_SRC_DSI_MUX_WCO        (17UL)
#if (SRSS_ALTLF_PRESENT == 1U)
    #define CY_ROOT_PATH_SRC_DSI_MUX_ALTLF  (18UL)
#endif /* (SRSS_ALTLF_PRESENT == 1U) */
#if (SRSS_PILO_PRESENT == 1U)
    #define CY_ROOT_PATH_SRC_DSI_MUX_PILO   (19UL)
#endif /* (SRSS_PILO_PRESENT == 1U) */


/*******************************************************************************
* Function Name: SystemInit
****************************************************************************//**
* \cond
* Initializes the system:
* - Restores FLL registers to the default state for single core devices.
* - Unlocks and disables WDT.
* - Calls Cy_PDL_Init() function to define the driver library.
* - Calls the Cy_SystemInit() function.
* - Calls \ref SystemCoreClockUpdate().
* \endcond
*******************************************************************************/
void SystemInit(void)
{
    Cy_PDL_Init(CY_DEVICE_CFG);

    Cy_SystemInit();

#ifdef CY_DEVICE_FORCE_IP_ENABLE_IN_STARTUP
    (void)Cy_SysClk_PeriGroupSetSlaveCtl(PERI_1_GROUP_2, CY_SYSCLK_PERI_GROUP_SL_CTL, 0x3U);
#endif
#if  (!defined(CY_USE_FULL_PROTECTION) || (CY_USE_FULL_PROTECTION == 0))
    SystemCoreClockUpdate();
#endif
}

/*******************************************************************************
* Function Name: Cy_SystemInit
****************************************************************************//**
*
* The function is called during device startup.
*
*******************************************************************************/
__WEAK void Cy_SystemInit(void)
{
     /* Empty weak function.
     */
}


/*******************************************************************************
* Function Name: SystemCoreClockUpdate
****************************************************************************//**
*
* Gets core clock frequency and updates \ref SystemCoreClock, \ref
* cy_Hfclk0FreqHz, and \ref cy_PeriClkFreqHz.
*
* Updates global variables used by the \ref Cy_SysLib_Delay(), \ref
* Cy_SysLib_DelayUs(), and \ref Cy_SysLib_DelayCycles().
*
*******************************************************************************/
void SystemCoreClockUpdate (void)
{
#if (!defined(CY_USE_FULL_PROTECTION) || (CY_USE_FULL_PROTECTION == 0)) || defined (COMPONENT_SECURE_DEVICE)
    uint32_t pathFreqHz;
    uint32_t clkHfPath;
    /* Get frequency for the high-frequency clock # 0 */
    clkHfPath = CY_SYSCLK_CLK_CORE_HF_PATH_NUM;

    pathFreqHz = Cy_SysClk_ClkHfGetFrequency(clkHfPath);

    SystemCoreClock = pathFreqHz;

    cy_Hfclk0FreqHz = SystemCoreClock;

    /* Get frequency for the high-frequency clock # 2 , which is used for PERI PCLK*/
    clkHfPath = CY_SYSCLK_CLK_PERI_HF_PATH_NUM;

    pathFreqHz = Cy_SysClk_ClkHfGetFrequency(clkHfPath);

    cy_PeriClkFreqHz = pathFreqHz;

    /* Sets clock frequency for Delay API */
    cy_delayFreqHz = SystemCoreClock;
    cy_delayFreqMhz = (uint32_t)((cy_delayFreqHz + CY_DELAY_1M_MINUS_1_THRESHOLD) / CY_DELAY_1M_THRESHOLD);
    cy_delayFreqKhz = (cy_delayFreqHz + CY_DELAY_1K_MINUS_1_THRESHOLD) / CY_DELAY_1K_THRESHOLD;

    /* Get the frequency of AHB source, CLK HF0 is the source for AHB*/
    cy_AhbFreqHz = Cy_SysClk_ClkHfGetFrequency(0UL);
#else
    cy_rpc_service_args_t rpcInputArgs, rpcOutputArgs;
    rpcInputArgs.argc = 5;
    rpcInputArgs.argv[0] = (uint32_t)CY_SECURE_SERVICE_TYPE_SYSTEM;
    rpcInputArgs.argv[1] = (uint32_t)CY_SECURE_SERVICE_SYS_GET_CORE_CLK;
    /* Get frequency for the high-frequency clock # 0 */
    rpcInputArgs.argv[2] = (uint32_t)CY_SYSCLK_CLK_CORE_HF_PATH_NUM;
    /* Get frequency for the high-frequency clock # 2 , which is used for PERI PCLK*/
    rpcInputArgs.argv[3] = (uint32_t)CY_SYSCLK_CLK_PERI_HF_PATH_NUM;
    /* HF0 clk*/
    rpcInputArgs.argv[4] = 0;
    cy_rpc_invec_t in_vec[] = {
        { .base = &rpcInputArgs, .len = sizeof(rpcInputArgs) },
    };
    cy_rpc_outvec_t out_vec[] = {
            { .base = &rpcOutputArgs, .len = sizeof(rpcOutputArgs) },
    };
    Cy_SecureServices_RPC(in_vec, CY_RPC_IOVEC_LEN(in_vec), out_vec, CY_RPC_IOVEC_LEN(out_vec));

    if (rpcOutputArgs.argc == 4)
    {
      SystemCoreClock = rpcOutputArgs.argv[1];
      cy_PeriClkFreqHz = rpcOutputArgs.argv[2];
      /* Sets clock frequency for Delay API */
      cy_delayFreqHz = SystemCoreClock;
      cy_delayFreqMhz = (uint32_t)((cy_delayFreqHz + CY_DELAY_1M_MINUS_1_THRESHOLD) / CY_DELAY_1M_THRESHOLD);
      cy_delayFreqKhz = (cy_delayFreqHz + CY_DELAY_1K_MINUS_1_THRESHOLD) / CY_DELAY_1K_THRESHOLD;
      /* Get the frequency of AHB source, CLK HF0 is the source for AHB*/
      cy_AhbFreqHz = rpcOutputArgs.argv[3];
    }
#endif
}
/* [] END OF FILE */
