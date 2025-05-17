/******************************************************************************
* File Name:   app_utils.h
*
* Description: This file contains macros and utility function prototypes that 
*              make debugging and application development easier. 
*
* Related Document: See Readme.md
*
*******************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
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

#ifndef __APP_UTILS_H__
#define __APP_UTILS_H__

/******************************************************************************
 * INCLUDES
 ******************************************************************************/
#include "wiced_bt_dev.h"
#include "wiced_bt_gatt.h"
#include "cy_wcm.h"

/******************************************************************************
 * Constants
 ******************************************************************************/
#define CASE_RETURN_STR(enum_val)          case enum_val: return #enum_val;
#define CASE_DEFAULT_STR default: printf( "%s: defalut case\n", __FUNCTION__ ); break;

/****************************************************************************
 * FUNCTION DECLARATIONS
 ***************************************************************************/
void print_bd_address(wiced_bt_device_address_t bdadr);
const char *get_btm_event_name(wiced_bt_management_evt_t event);
const char *get_bt_advert_mode_name(wiced_bt_ble_advert_mode_t mode);
const char *get_bt_gatt_disconn_reason_name(wiced_bt_gatt_disconn_reason_t
                                            reason);
const char *get_bt_gatt_status_name(wiced_bt_gatt_status_t status);
const char *get_bt_gatt_evt_name(wiced_bt_gatt_evt_t event);
const char *get_wifi_security_name(cy_wcm_security_t name);

#endif      /* __APP_UTILS_H__ */


/* [] END OF FILE */