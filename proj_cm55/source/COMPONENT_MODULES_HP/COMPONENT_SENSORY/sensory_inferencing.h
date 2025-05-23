/******************************************************************************
* File Name : sensory_inferencing.h
*
* Description :
* Header file for Sensory inferencing.
********************************************************************************
*******************************************************************************/

#ifndef __SENSORY_INFERENCING_H__
#define __SENSORY_INFERENCING_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define SNSR_USE_SUBSET

#include <snsr.h> /* Sensory TrulyNatural SDK API */


#include "inferencing_interface.h"
#ifdef BATTERY_POWERED_LOCAL_VOICE
#include "battery_powered_inferencing.h"
#endif /* BATTERY_POWERED_LOCAL_VOICE */

#ifdef MAINS_POWERED_LOCAL_VOICE
#include "mains_powered_inferencing.h"
#endif /* MAINS_POWERED_LOCAL_VOICE */

#ifdef DEBUG_TUNE_MAINS_APP
#include "tune_inferencing_mains.h"
#endif /* DEBUG_TUNE_MAINS_APP */

SnsrSession sensory_init();
void sensory_processing(SnsrSession session, int16_t *audio_data, int32_t length);
void sensory_deinit(SnsrSession session);

void snsr_command_to_id (const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SENSORY_INFERENCING_H__ */
