/******************************************************************************
* File Name : cm33_intercore_pipe.c
*
* Description :
* Intercore IPC pipe on CM33 core for Staged Voice Control.
*
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
#include "cycfg.h"
#include <stdio.h>
#include "cy_ipc_pipe.h"
#include "app_logger.h"
#include "intercore_ipc.h"

/*******************************************************************************
* Global Variables
*******************************************************************************/

cy_stc_scb_uart_context_t UART_context;

typedef struct
{
    uint8_t  clientID;      /**< Client ID */
    uint8_t  pktType;       /**< Message Type */
    uint16_t intrRelMask;   /**< Mask */
} cy_stc_ipc_testmsg_t;

bool cm33_pipe2_msg_executed = false;
bool cm33_pipe2_msg_received = false;

bool cm55_pipe2_msg_executed = false;
bool cm55_pipe2_msg_received = false;

CY_SECTION_SHAREDMEM
cy_stc_ipc_testmsg_t cm33MsgData1;
CY_SECTION_SHAREDMEM
cy_stc_ipc_testmsg_t cm55MsgData1;

typedef enum
{
    CY_IPC_PKT_FROM_CM33_TO_CM55,
    CY_IPC_PKT_FROM_CM55_TO_CM33,
} cy_en_ipc_pktType_t;

/*******************************************************************************
* Function Name: Cy_SysIpcPipeIsrCm33
********************************************************************************
* Summary:
*  This is the interrupt service routine for the system pipe.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Cy_SysIpcPipeIsrCm33(void)
{

    Cy_IPC_Pipe_ExecuteCallback(CY_IPC_EP_CYPIPE_CM33_ADDR);
}

/*******************************************************************************
* Function Name: Pipe2_cm33_RecvMsgCallback
********************************************************************************
* Summary:
*  Called when the Pipe2 endpoint-1 (CM33) has received a message.
*
* Parameters:
*  msgData - Message data.
*
* Return:
*  None
*
*******************************************************************************/

void Pipe2_cm33_RecvMsgCallback(uint32_t * msgData)
{
    /* The release callback will be automatically called on the sender side
     * just after returning from this callback
     */
    cy_stc_ipc_testmsg_t *pData = (cy_stc_ipc_testmsg_t*)msgData;
    Cy_SysLib_Delay(10);

    app_log_print ("[Pipe2][EP1][CM33] RecvMsgCallback ---> \r\n\n");
    app_log_print("[Pipe2][EP1][CM33] Received data for Client[%d] message Type is %d \r\n", pData->clientID, pData->pktType);
    app_log_print ("<--- [Pipe2][EP1][CM33] RecvMsgCallback\r\n\n");

    cm33_pipe2_msg_received = true;
    Cy_SysLib_Delay(10);
    CY_UNUSED_PARAMETER(msgData);
}

/*******************************************************************************
* Function Name: Pipe2_cm33_ReleaseCallback
********************************************************************************
* Summary:
*  Release callback
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Pipe2_cm33_ReleaseCallback(void)
{
    /* The sent data is already processed on the receiver side,
     * the pipe is ready for next transactions
     */

    Cy_SysLib_Delay(10);
    app_log_print ("[Pipe2][EP1][CM33] ReleaseCallback ---> \r\n\n");
    app_log_print ("<--- [Pipe2][EP1][CM33] ReleaseCallback\r\n\n");

    cm33_pipe2_msg_executed = true;
    Cy_SysLib_Delay(10);
}

/*******************************************************************************
* Function Name: Pipe2_cm33_DefaultReleaseCallback
********************************************************************************
* Summary:
*  Release callback (default)
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void Pipe2_cm33_DefaultReleaseCallback(void)
{
    /* The Cy_IPC_Pipe_SendMessage was called without a pointer to specific callback,
     * threfore this one is called.
     * The sent data is already processed on the receiver side,
     * the pipe is ready for next transactions
     */

    Cy_SysLib_Delay(10);
    app_log_print ("[Pipe2][EP1][CM33] DefaultReleaseCallback ---> \r\n\n");
    app_log_print ("<--- [Pipe2][EP1][CM33] DefaultReleaseCallback\r\n\n");

    Cy_SysLib_Delay(10);
}


/*******************************************************************************
* Function Name: ipc_pipe_usecase4_cm33ns_cm55
********************************************************************************
* Summary:
*  IPC pipe code
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void ipc_pipe_cm33ns_cm55(void)
{
/*    * 1. CM33 boots CM55
      * 2. CM55 waits for message from CM33 in Pipe-0
      * 3. CM33 sends message to CM55
      * 4. Once CM55 receives message from CM33 in Pipe-0 then it sends message to
      *     CM33 in Pipe-0.
*/

    /* Create an array of endpoint structures */
    static cy_stc_ipc_pipe_ep_t systemIpcPipeEpArray[CY_IPC_MAX_ENDPOINTS];

    Cy_IPC_Pipe_Config(systemIpcPipeEpArray);

    app_log_print("\r ============================START============================== \r\n");
    app_log_print("[CM33] IPC Application Main...\r\n");

    static cy_ipc_pipe_callback_ptr_t ep1CbArray[CY_IPC_CYPIPE_CLIENT_CNT]; /* CB Array for EP1 */

    /* Pipe-2 endpoint-1 and endpoint-2. CM33 <--> CM55 */
    static const cy_stc_ipc_pipe_config_t systemIpcPipe2ConfigCm33 =
    {
    /* receiver endpoint CM33 */
        {
            /* .ipcNotifierNumber     */  CY_IPC_INTR_CYPIPE_EP1,
            /* .ipcNotifierPriority  */  CY_IPC_INTR_CYPIPE_PRIOR_EP1,
            /* .ipcNotifierMuxNumber */  CY_IPC_INTR_CYPIPE_MUX_EP1,
            /* .epAddress             */  CY_IPC_EP_CYPIPE_CM33_ADDR,
            {
                /* .epChannel */CY_IPC_CHAN_CYPIPE_EP1,
                /* .epIntr */ CY_IPC_INTR_CYPIPE_EP1,
                /* .epIntrmask */ CY_IPC_CYPIPE_INTR_MASK
            }
        },
    /* sender endpoint CM55 */
        {
            /* .ipcNotifierNumber     */  CY_IPC_INTR_CYPIPE_EP2,
            /* .ipcNotifierPriority  */  CY_IPC_INTR_CYPIPE_PRIOR_EP2,
            /* .ipcNotifierMuxNumber */  CY_IPC_INTR_CYPIPE_MUX_EP2,
            /* .epAddress             */  CY_IPC_EP_CYPIPE_CM55_ADDR,
            {
                /* .epChannel */CY_IPC_CHAN_CYPIPE_EP2,
                /* .epIntr */ CY_IPC_INTR_CYPIPE_EP2,
                /* .epIntrmask */ CY_IPC_CYPIPE_INTR_MASK
            }
        },
    /* .endpointClientsCount     */  CY_IPC_CYPIPE_CLIENT_CNT,
    /* .endpointsCallbacksArray  */  ep1CbArray,
    /* .userPipeIsrHandler         */  &Cy_SysIpcPipeIsrCm33
    };

    /* Disable D-Cache */
    //SCB_DisableDCache();
#if defined (__ARMCC_VERSION)
    app_log_print ("[CM33][EP1] IPC Channel-%d address[0x%x] Intr-%d address[0x%x] \r\n", CY_IPC_CHAN_CYPIPE_EP1,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_CYPIPE_EP1),
                                                                                    CY_IPC_INTR_CYPIPE_EP1,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_CYPIPE_EP1));
#else
    app_log_print ("[CM33][EP1] IPC Channel-%d address[0x%lx] Intr-%d address[0x%lx] \r\n", CY_IPC_CHAN_CYPIPE_EP1,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_CYPIPE_EP1),
                                                                                    CY_IPC_INTR_CYPIPE_EP1,
                                                                                    (uint32_t)Cy_IPC_Drv_GetIntrBaseAddr(CY_IPC_INTR_CYPIPE_EP1));
#endif // __ARMCC_VERSION

    Cy_IPC_Pipe_Init(&systemIpcPipe2ConfigCm33); /* PIPE-2 EP1 <--> EP2 */
    
    app_log_print("\r ============================END============================== \r\n");
}

/*******************************************************************************
* Function Name: app_ipc_init_cm33
********************************************************************************
* Summary:
* Initializes IPC pipe on CM33 for Staged Voice Control.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/

void app_ipc_init_cm33(void)
{
    ipc_pipe_cm33ns_cm55();
}

/* [] END OF FILE */
