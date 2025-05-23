/******************************************************************************
* File Name : inferencing_interface.h
*
* Description :
* Header file for Inference processing on CM55.
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

#ifndef __INFERENCING_INTERFACE_H__
#define __INFERENCING_INTERFACE_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef BATTERY_POWERED_LOCAL_VOICE
#include "battery_powered_inferencing.h"
#endif /* BATTERY_POWERED_LOCAL_VOICE */

#ifdef MAINS_POWERED_LOCAL_VOICE
#include "mains_powered_inferencing.h"
#endif /* MAINS_POWERED_LOCAL_VOICE */

#ifdef DEBUG_TUNE_MAINS_APP
#include "tune_inferencing_mains.h"
#endif /* DEBUG_TUNE_MAINS_APP */

/*******************************************************************************
* Macros
*******************************************************************************/
#define OK_INFINEON_CMD_ID                              (101)
#define INFERENCING_TIMEOUT                             (128000)
#define WAKEWORD_CMD_ID                                 (101)

/*******************************************************************************
 * Type definitions
 ******************************************************************************/

typedef void (*inferencing_callback)(const char *function_name, char *message, char *parameter);

typedef enum cy_grp_2_asr_cmds_music
{
    PLAY_MUSIC_CMD_ID = 401,
    STOP_MUSIC_CMD_ID,
    NEXT_TRACK_CMD_ID,
    PREVIOUS_TRACK_CMD_ID,
    INCREASE_VOL_CMD_ID,
    DECREASE_VOL_CMD_ID,
} cy_grp_2_asr_cmds_music_t;


#define INF_WAKE_WORD                "WAKEWORD"

#define INF_OK_INFINEON              "Okay Infineon"
#define INF_PLAY_MUSIC               "play music"
#define INF_STOP_MUSIC               "stop music"
#define INF_INCREASE_VOLUME          "increase volume"
#define INF_DECREASE_VOLUME          "decrease volume"
#define INF_NEXT_TRACK               "next track"
#define INF_PREVIOUS_TRACK           "previous track"
#define INF_TIMEOUT                  "timeout"

/*******************************************************************************
* Functions Prototypes
*******************************************************************************/

int inferencing_engine_init(inferencing_callback callback_function);
void inferencing_engine_deinit();
void inference_processing(short *data, int num_samples);
int is_inference_license_valid();

#ifdef BATTERY_POWERED_LOCAL_VOICE
int get_current_inference_stage();
int set_current_inference_stage(int stage);
int switch_to_wwd_inferencing();
int switch_to_asr_inferencing();
int switch_to_wwd_inferencing_streaming();
int switch_to_asr_inferencing_streaming();
int switch_model_to_wwd();
int switch_model_to_asr();

void get_inferencing_frame_counters(unsigned int *current_feed_counter_stage1,unsigned int *current_feed_counter_stage2 );
void inference_reset_wwd_feed_timeout_counter();
void inference_timeout_check();
#endif /* BATTERY_POWERED_LOCAL_VOICE */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INFERENCING_INTERFACE_H__ */

/* [] END OF FILE */
