/******************************************************************************
* File Name : battery_powered_svc_hp.h
*
* Description :
* Header file for Staged Voice Control on CM55.
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
#ifndef __BATTERY_POWERED_SVC_HP_H__
#define __BATTERY_POWERED_SVC_HP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cybsp.h"
#include "cy_retarget_io.h"

#include "stdlib.h"
#include "cy_log.h"
#include "cy_staged_voice_control.h"
#ifdef COMPONENT_QBUF_POOL
#include "cy_qbuf_pool.h"
#endif /* COMPONENT_QBUF_POOL*/


/*******************************************************************************
* Macros
*******************************************************************************/
#define APP_SVC_HP_LOG_ENABLE                (0)

#if APP_SVC_HP_LOG_ENABLE
#define app_svc_log_hp(format,...)            printf(format "\r\n",##__VA_ARGS__);
#else
#define app_svc_log_hp(format,...)
#endif /* APP_SVC_HP_LOG_ENABLE */

/*******************************************************************************
* Global Variables
*******************************************************************************/

typedef struct __attribute__((packed, aligned(4)))
{
    uint8_t     cmd_id;                     /* Byte 1       - Command identifier    */
    uint8_t     dbg_message_counter;        /* Byte 2       - message counter       */
    uint8_t     bitmask;                    /* Byte 3       - bit mask  for any purpose */
    uint8_t     reserved;                   /* Byte 4                               */
    cy_svc_buffer_info_t     buffer_info;   /* Byte 5 - 8  */
    uint8_t      *data_pointer;             /* Byte 9 - 12   - Data pointer          */
    uint32_t    frame_count;                /* Byte 13 - 16  - Frame count           */
} svc_hp_app_cmd_q_data_t ;

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

void app_svc_hp_app_init(void);
void app_svc_hp_clear_queue(void);

#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */

#endif /* __BATTERY_POWERED_SVC_HP_H__ */

/* [] END OF FILE */

