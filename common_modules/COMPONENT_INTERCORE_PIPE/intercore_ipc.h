/******************************************************************************
* File Name : intercore_ipc.h
*
* Description :
* Header for intercore IPC - Staged Voice Control
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

#ifndef __INTERCORE_IPC_H__
#define __INTERCORE_IPC_H__
/* For PSOC Edge, there are 32 channels and 16 interrupts are available */

/*******************************************************************************
* Macros
*******************************************************************************/

/* change this macro for valid usecase */
#define CM55_PRINT_ON                           (1)

#define CY_IPC_MAX_ENDPOINTS                    (5UL) /* <<< 3 endpoints */
#define CY_IPC_CYPIPE_CLIENT_CNT                (8UL)

/* Channel 2 goes to IPC0 */
#define CY_IPC_CHAN_CYPIPE_EP1                  (CM33_NS_IPC0_CH_NUM) /* IPC data channel for CYPIPE EP1 */
#define CY_IPC_CYPIPE_CHAN_MASK_EP1             (CM33_NS_IPC0_CH_MASK)
#define CY_IPC_INTR_CYPIPE_EP1                  (CM33_NS_IPC0_INTR_NUM)   /* IPC Intr for EP1 */
#define CY_IPC_CYPIPE_INTR_MASK_EP1             (CM33_NS_IPC0_INTR_MASK)   /* IPC Intr Mask for EP0 */
#define CY_IPC_INTR_CYPIPE_PRIOR_EP1            (5UL)   /* Notifier Priority */
#define CY_IPC_INTR_CYPIPE_MUX_EP1              (CM33_NS_IPC0_INTR_MUX)   /* Intr Mux for CM33 */
#define CY_IPC_EP_CYPIPE_CM33_ADDR              (1UL)   /* EP1 Index of Endpoint Array */
#define CY_CLIENT_CYPIPE2_CM33_ID1              (3UL)   /* EP1 Pipe2 (CM33 <--> CM55) Index of client cb Array */

/* Channel 2 goes to IPC0 */
#define CY_IPC_CHAN_CYPIPE_EP2                  (CY_IPC_CHAN_CYPIPE_EP1+1) /* IPC data channel for CYPIPE EP2 */
#define CY_IPC_CYPIPE_CHAN_MASK_EP2             (CY_IPC_CH_MASK(CY_IPC_CHAN_CYPIPE_EP2))
#define CY_IPC_INTR_CYPIPE_EP2                  (CY_IPC_INTR_CYPIPE_EP1+1)
#define CY_IPC_CYPIPE_INTR_MASK_EP2             (CY_IPC_INTR_MASK(CY_IPC_INTR_CYPIPE_EP2))   /* IPC Intr Mask for EP2 */
#define CY_IPC_INTR_CYPIPE_PRIOR_EP2            (5UL)   /* Notifier Priority */
#define CY_IPC_INTR_CYPIPE_MUX_EP2              (CY_IPC0_INTR_MUX(CY_IPC_INTR_CYPIPE_EP2))   /* Intr Mux for CM33 */
#define CY_IPC_EP_CYPIPE_CM55_ADDR              (2UL)   /* EP2 Index of Endpoint Array */
#define CY_CLIENT_CYPIPE2_CM55_ID1              (5UL)   /* EP2 Pipe2 (CM55 <--> CM33) Index of client cb Array */


#define APP_LP_PIPE_END_POINT_ADDRESS           (CY_IPC_EP_CYPIPE_CM33_ADDR)
#define APP_LP_PIPE_END_POINT_CLIENT_ID_1       (CY_CLIENT_CYPIPE2_CM33_ID1)
#define APP_HP_PIPE_END_POINT_ADDRESS           (CY_IPC_EP_CYPIPE_CM55_ADDR)
#define APP_HP_PIPE_END_POINT_CLIENT_ID_2       (CY_CLIENT_CYPIPE2_CM55_ID1)

/*
For Explorer device:
      Channels   Interrupts
IPC0:   0 - 15     0 -  7
IPC1:  16 - 31     8 - 15
*/
#define CY_IPC_CYPIPE_INTR_MASK   ( CY_IPC_CYPIPE_CHAN_MASK_EP1 | CY_IPC_CYPIPE_CHAN_MASK_EP2)



#endif /* __INTERCORE_IPC_H__ */
