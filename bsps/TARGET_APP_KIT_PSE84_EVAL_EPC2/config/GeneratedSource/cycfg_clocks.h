/*******************************************************************************
 * File Name: cycfg_clocks.h
 *
 * Description:
 * Clock configuration
 * This file was automatically generated and should not be modified.
 * Configurator Backend 3.50.0
 * device-db 4.5.30.8214
 * mtb-device-support-pse8xxgp 1.0.200.195
 *
 *******************************************************************************
 * Copyright 2025 Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.
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
 ******************************************************************************/

#if !defined(CYCFG_CLOCKS_H)
#define CYCFG_CLOCKS_H

#include "cycfg_notices.h"
#include "cy_sysclk.h"

#if defined (CY_USING_HAL)
#include "cyhal_hwmgr.h"
#endif /* defined (CY_USING_HAL) */

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_HW CY_SYSCLK_DIV_16_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_NUM 0U
#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_I2C_CONTROLLER_0_CLK_DIV_GRP_NUM)
#define CYBSP_I2C_CONTROLLER_0_CLK_DIV_GRP_NUM CYBSP_I2C_CONTROLLER_0_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_I2C_CONTROLLER_0_CLK_DIV_GRP_NUM) */

#define CYBSP_DEBUG_UART_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_DEBUG_UART_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_DEBUG_UART_CLK_DIV_HW CY_SYSCLK_DIV_16_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_DEBUG_UART_CLK_DIV_NUM 1U
#define CYBSP_DEBUG_UART_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_DEBUG_UART_CLK_DIV_GRP_NUM)
#define CYBSP_DEBUG_UART_CLK_DIV_GRP_NUM CYBSP_DEBUG_UART_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_DEBUG_UART_CLK_DIV_GRP_NUM) */

#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_HW CY_SYSCLK_DIV_16_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_NUM 2U
#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_GRP_NUM)
#define CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_GRP_NUM CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_GRP_NUM) */

#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_HW CY_SYSCLK_DIV_16_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_NUM 3U
#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_GRP_NUM)
#define CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_GRP_NUM CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_GRP_NUM) */

#define CYBSP_BT_UART_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_BT_UART_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_BT_UART_CLK_DIV_HW CY_SYSCLK_DIV_16_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_BT_UART_CLK_DIV_NUM 0U
#define CYBSP_BT_UART_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_BT_UART_CLK_DIV_GRP_NUM)
#define CYBSP_BT_UART_CLK_DIV_GRP_NUM CYBSP_BT_UART_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_BT_UART_CLK_DIV_GRP_NUM) */

#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_16_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_HW CY_SYSCLK_DIV_16_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_NUM 1U
#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_GRP_NUM)
#define CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_GRP_NUM CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_GRP_NUM) */

#define CYBSP_SMART_IO_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_SMART_IO_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_24_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_SMART_IO_CLK_DIV_HW CY_SYSCLK_DIV_24_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_SMART_IO_CLK_DIV_NUM 0U
#define CYBSP_SMART_IO_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_SMART_IO_CLK_DIV_GRP_NUM)
#define CYBSP_SMART_IO_CLK_DIV_GRP_NUM CYBSP_SMART_IO_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_SMART_IO_CLK_DIV_GRP_NUM) */

#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_8BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_HW CY_SYSCLK_DIV_8_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_NUM 0U
#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_EZ_I2C_TARGET_5_CLK_DIV_GRP_NUM)
#define CYBSP_EZ_I2C_TARGET_5_CLK_DIV_GRP_NUM CYBSP_EZ_I2C_TARGET_5_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_EZ_I2C_TARGET_5_CLK_DIV_GRP_NUM) */

#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_8BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_HW CY_SYSCLK_DIV_8_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_NUM 1U
#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_SPI_CONTROLLER_10_CLK_DIV_GRP_NUM)
#define CYBSP_SPI_CONTROLLER_10_CLK_DIV_GRP_NUM CYBSP_SPI_CONTROLLER_10_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_SPI_CONTROLLER_10_CLK_DIV_GRP_NUM) */

#define CYBSP_USB_OS_TIMER_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_USB_OS_TIMER_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL1_8BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_USB_OS_TIMER_CLK_DIV_HW CY_SYSCLK_DIV_8_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_USB_OS_TIMER_CLK_DIV_NUM 2U
#define CYBSP_USB_OS_TIMER_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_USB_OS_TIMER_CLK_DIV_GRP_NUM)
#define CYBSP_USB_OS_TIMER_CLK_DIV_GRP_NUM CYBSP_USB_OS_TIMER_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_USB_OS_TIMER_CLK_DIV_GRP_NUM) */

#define CYBSP_TRACE_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_TRACE_CLK_DIV_HW CYHAL_CLOCK_BLOCK0_PERIPHERAL7_16_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_TRACE_CLK_DIV_HW CY_SYSCLK_DIV_16_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_TRACE_CLK_DIV_NUM 0U
#define CYBSP_TRACE_CLK_DIV_GRP_NUM ((7U << PERI_PCLK_GR_NUM_Pos) | (0U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_TRACE_CLK_DIV_GRP_NUM)
#define CYBSP_TRACE_CLK_DIV_GRP_NUM CYBSP_TRACE_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_TRACE_CLK_DIV_GRP_NUM) */

#define peri_0_mmio_0_ENABLED 1U
#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_HW CYHAL_CLOCK_BLOCK1_PERIPHERAL1_16_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_HW CY_SYSCLK_DIV_16_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_NUM 0U
#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (1U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_TDM_CONTROLLER_0_CLK_DIV_GRP_NUM)
#define CYBSP_TDM_CONTROLLER_0_CLK_DIV_GRP_NUM CYBSP_TDM_CONTROLLER_0_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_TDM_CONTROLLER_0_CLK_DIV_GRP_NUM) */

#define CYBSP_PDM_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_PDM_CLK_DIV_HW CYHAL_CLOCK_BLOCK1_PERIPHERAL1_16_5BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_PDM_CLK_DIV_HW CY_SYSCLK_DIV_16_5_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_PDM_CLK_DIV_NUM 1U
#define CYBSP_PDM_CLK_DIV_GRP_NUM ((1U << PERI_PCLK_GR_NUM_Pos) | (1U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_PDM_CLK_DIV_GRP_NUM)
#define CYBSP_PDM_CLK_DIV_GRP_NUM CYBSP_PDM_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_PDM_CLK_DIV_GRP_NUM) */

#define CYBSP_SDHC_0_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_SDHC_0_CLK_DIV_HW CYHAL_CLOCK_BLOCK1_PERIPHERAL2_8BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_SDHC_0_CLK_DIV_HW CY_SYSCLK_DIV_8_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_SDHC_0_CLK_DIV_NUM 0U
#define CYBSP_SDHC_0_CLK_DIV_GRP_NUM ((2U << PERI_PCLK_GR_NUM_Pos) | (1U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_SDHC_0_CLK_DIV_GRP_NUM)
#define CYBSP_SDHC_0_CLK_DIV_GRP_NUM CYBSP_SDHC_0_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_SDHC_0_CLK_DIV_GRP_NUM) */

#define CYBSP_SDHC_1_CLK_DIV_ENABLED 1U

#if defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE)
#define CYBSP_SDHC_1_CLK_DIV_HW CYHAL_CLOCK_BLOCK1_PERIPHERAL3_8BIT
#endif /* defined (CY_USING_HAL) || defined (CY_USING_HAL_LITE) */

#if !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE)
#define CYBSP_SDHC_1_CLK_DIV_HW CY_SYSCLK_DIV_8_BIT
#endif /* !defined (CY_USING_HAL) && !defined (CY_USING_HAL_LITE) */

#define CYBSP_SDHC_1_CLK_DIV_NUM 0U
#define CYBSP_SDHC_1_CLK_DIV_GRP_NUM ((3U << PERI_PCLK_GR_NUM_Pos) | (1U << PERI_PCLK_INST_NUM_Pos))

#if !defined (CYBSP_SDHC_1_CLK_DIV_GRP_NUM)
#define CYBSP_SDHC_1_CLK_DIV_GRP_NUM CYBSP_SDHC_1_CLK_DIV_GRP_NUM
#endif /* !defined (CYBSP_SDHC_1_CLK_DIV_GRP_NUM) */

#define peri_1_mmio_0_ENABLED 1U

#if defined (CY_USING_HAL)
extern const cyhal_resource_inst_t CYBSP_I2C_CONTROLLER_0_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_DEBUG_UART_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_TCPWM_0_GRP_0_COUNTER_0_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_TCPWM_0_GRP_1_PWM_5_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_BT_UART_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_TCPWM_0_GRP_1_PWM_6_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_SMART_IO_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_EZ_I2C_TARGET_5_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_SPI_CONTROLLER_10_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_USB_OS_TIMER_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_TRACE_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_TDM_CONTROLLER_0_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_PDM_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_SDHC_0_CLK_DIV_obj;
extern const cyhal_resource_inst_t CYBSP_SDHC_1_CLK_DIV_obj;
#endif /* defined (CY_USING_HAL) */

void init_cycfg_clocks(void);
void reserve_cycfg_clocks(void);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CYCFG_CLOCKS_H */
