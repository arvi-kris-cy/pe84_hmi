/******************************************************************************
* File Name : profiler.h
*
* Description :
* Configuration Header for MIPS profiler
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
#ifndef _PROFILER_H_
#define _PROFILER_H_

#include <inttypes.h>
#ifndef COMPONENT_CM33
//#include <malloc.h>
#endif
#include <stdlib.h>
#include <string.h>

#ifndef COMPONENT_CM33
//void display_mallinfo(void);
#endif


//#define MILLI_SEC_PROFILE (1)

/* Peripheral clock dividier */
#define CPU_PERI_CLOCK_DIV (2)
#define PROFILE_SW_CODEC 1
//#define MEASURE_STACK_SIZE 1
//#define MEASURE_HEAP_SIZE 1

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif


/* ----- Deprecated Start - Use new APIs from cy_profiler.h ---------- */
//void profiler_init(void);
//void start_time(void);
//void stop_time(void);
//uint32_t get_time(void);
//uint32_t get_time_ms(void);
/* ----- Deprecated end --------------------------- */


#endif /* _PROFILER_H_ */


/* [] END OF FILE */
