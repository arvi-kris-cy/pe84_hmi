/******************************************************************************
* File Name : tune_app_svc_lp.h
*
* Description :
* Header for CM33 non secure application - staged voice control in tuning mode.
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
#ifndef __TUNE_APP_SVC_LP_H__
#define __TUNE_APP_SVC_LP_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cybsp.h"
#include "cy_retarget_io.h"

#include "stdlib.h"
#include "cy_log.h"
#include "cy_buffer_pool.h"
#include "cy_staged_voice_control.h"

/*******************************************************************************
* Macros
*******************************************************************************/

#define ENABLE_APP_SVC_LOG                  (0)

#if ENABLE_APP_SVC_LOG
#define app_svc_log(format, ...)            printf(format "\r\n",##__VA_ARGS__);
#else
#define app_svc_log(format, ...)
#endif

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/
cy_rslt_t app_init_svc(void);
cy_rslt_t app_deinit_svc(void);


#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */

#endif /* __TUNE_APP_SVC_LP_H__ */

/* [] END OF FILE */

