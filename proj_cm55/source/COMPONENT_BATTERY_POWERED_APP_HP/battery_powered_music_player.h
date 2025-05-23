/******************************************************************************
* File Name: battery_powered_music_player.h
*
* Description: Header file for batter powered music player
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
#ifndef __BATTERY_POWERED_MUSIC_PLAYER_H__
#define __BATTERY_POWERED_MUSIC_PLAYER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "cybsp.h"
#include "semphr.h"

#ifdef AUDIO_OUT
#include "i2s_playback.h"
#endif /* AUDIO_OUT */
/*******************************************************************************
* Macros
********************************************************************************/
/* Task priority and stack size for the music player task */
#define MUSIC_PLAYER_TASK_PRIORITY                 (4u)
#define MUSIC_PLAYER_TASK_STACK_SIZE               (2 * 1024u)

/*******************************************************************************
* Data structure and enumeration
********************************************************************************/
/* Data-type for Music Player task commands */
typedef enum
{
    I2S_PLAYBACK_DEFAULT_STATE=0x00,
    MUSIC_PLAYER_PRELOAD_DATA,
    MUSIC_PLAYER_I2S_TRIGGER,
    MUSIC_PLAYER_FLUSH_DATA,

    I2S_PLAYBACK_PLAY_MUSIC,
    I2S_PLAYBACK_RESUME_MUSIC,
    I2S_PLAYBACK_STOP_MUSIC,
    I2S_PLAYBACK_PAUSE_MUSIC,
    I2S_PLAYBACK_NEXT_TRACK,
    I2S_PLAYBACK_PREV_TRACK,
    I2S_PLAYBACK_INC_VOL,
    I2S_PLAYBACK_DEC_VOL,
    I2S_PLAYBACK_WWD,

    MUSIC_PLAYER_SOURCE_FLASH_INIT,
    MUSIC_PLAYER_SOURCE_FLASH_DEINIT,
} music_player_cmd_t;

/* Data-type for Music Player task's queue data */
typedef struct
{
    music_player_cmd_t cmd;
    uint32_t data_len;
    uint8_t* data;
} music_player_q_data_t;

/* Data-type for I2S playback task's queue data */
typedef struct
{
    uint32_t data_len;
    int16_t* data;
} i2s_playback_q_data_t;

/* Data-type for Music Player's Audio Source */
typedef enum
{
    MUSIC_PLAYER_SOURCE_FLASH,
    MUSIC_PLAYER_SOURCE_NOT_CONFIGURED,
} music_player_audio_source_t;


/*******************************************************************************
* Extern Variables
********************************************************************************/
extern TaskHandle_t music_player_task_handle;
extern QueueHandle_t music_player_task_q;

extern bool music_player_active;
extern music_player_audio_source_t music_player_audio_source;
/*******************************************************************************
* Function Prototypes
********************************************************************************/
cy_rslt_t create_music_player_task(void);
void configure_music_player_audio_source(music_player_audio_source_t audio_source);
bool is_music_player_active(void);
bool is_music_player_paused(void);

/*******************************************************************************
* Inline Function Definitions
********************************************************************************/
__STATIC_INLINE void enable_music_player(void)
{
    music_player_active = true;
    configure_music_player_audio_source(MUSIC_PLAYER_SOURCE_FLASH);
}

__STATIC_INLINE void disable_music_player(void)
{
    music_player_active = false;
    configure_music_player_audio_source(MUSIC_PLAYER_SOURCE_NOT_CONFIGURED);
}

#ifdef __cplusplus
} /* extern C */
#endif /* __cplusplus */
#endif /* __BATTERY_POWERED_MUSIC_PLAYER_H__ */

/* [] END OF FILE */
