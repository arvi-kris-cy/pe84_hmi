/******************************************************************************
 * File Name : sensory_inferencing.c
 *
 * Description :
 * Source file for Sensory inferencing.
 ********************************************************************************
 *******************************************************************************/

#include "sensory_inferencing.h"

#include <string.h>
#include <stdlib.h>
#include "cybsp.h"

#include "cyabs_rtos.h"
#include "mtb_ml.h"
#include "cy_time.h"

/*****************************************************************************
 * Macros
 *****************************************************************************/
#define MTB_ML_PRIORITY                  (2)
#define ACTIVATION_BUF_SZ                (3800000) // Use with 4MB SOCMEMSRAM_CM55NS_DATA_SIZE (leaves room for stack).
#define PUSH_DURATION_LIMIT              (7)

static uint8_t sensoryHeap[ACTIVATION_BUF_SZ] __attribute__((aligned(16), section(".cy_socmem_data")));

#ifdef SENSORY_U55_TFLM

const char* modelName = "music_player";

extern SnsrCodeModel voicehub;
SnsrCodeModel *model = &voicehub;

#else

const char *modelName = "tnl_intent";

#endif /* SENSORY_U55_TFLM */

static char gotResult = 0;
static int tnlAudioBufferHigh = 0;
static uint32_t tnlPushHigh = 0;
int counter=0;
static cy_time_t start_time_ms, stop_time_ms;

static uint64_t clockFunc(void) {
    cy_time_t my_time;
    cy_rtos_get_time(&my_time);
    return (uint64_t) my_time;
}

/* Outputs recognition result */
static SnsrRC printResult(SnsrSession s, int final) {
    SnsrRC r = SNSR_RC_OK;
    const char *phrase = NULL;
    snsrGetString(s, SNSR_RES_TEXT, &phrase);

    if (final) {
        double begin, end, score = -1.0;
        gotResult = 1;
        snsrGetDouble(s, SNSR_RES_BEGIN_MS, &begin);
        snsrGetDouble(s, SNSR_RES_END_MS, &end);
        r = snsrGetDouble(s, SNSR_RES_SCORE, &score);
        if (r == SNSR_RC_SETTING_NOT_FOUND)
            snsrClearRC(s);

        printf("Command detected : %s \r\n", phrase);
        if (!strcmp(phrase,"<no-match/>"))
        {
            snsr_command_to_id(INF_TIMEOUT);
        } else
        {
            snsr_command_to_id(phrase);
        }
    } else if (strlen(phrase) > 0) {
        if (!strcmp(phrase,"ok infineon"))
        {
            /*printf("Wake Word Detected \r\n");*/
        }
    }
    return snsrRC(s);
}


static SnsrRC listenBeginEvent(SnsrSession const s, const char *key, void *const privateData)
{
  counter++;
  snsr_command_to_id (INF_WAKE_WORD);


  return SNSR_RC_OK;
}

static SnsrRC listenEndEvent(SnsrSession const s, const char *key, void *const privateData)
{
  return SNSR_RC_OK;
}


/* Handler for partial recognition result. */
static SnsrRC partialResultEvent(SnsrSession s, const char *key,
        void *privateData) {
    (void) key;
    (void) privateData;
    SnsrRC rc = SNSR_RC_OK;
    rc = printResult(s, 0);
    return rc;
}

/* Handler for recognition result. Supports different levels of verbosity */
static SnsrRC finalResultEvent(SnsrSession s, const char *key,
        void *privateData) {
    (void) key;
    (void) privateData;
    printResult(s, 1);
    return SNSR_RC_OK; // halts recognition
}

static SnsrRC nluEvent(SnsrSession s, const char *key, void *privateData) {
    (void) key;
    SnsrRC r = SNSR_RC_OK;
    const char *name, *parentPath = (char*) privateData, *value;
    char *path =NULL;
    size_t pathLen = 0;

    snsrGetString(s, SNSR_RES_NLU_SLOT_NAME, &name);
    r = snsrGetString(s, SNSR_RES_NLU_SLOT_VALUE, &value);
    if (r != SNSR_RC_OK)
        return r;

    pathLen = strlen(SNSR_RES_NLU_SLOT_VALUE) + strlen(name) + 1;
    if (parentPath)
        pathLen += strlen(parentPath) + 1;
    path = (char*) malloc(pathLen);
    if (path==NULL)
    {
        printf("Sensory:Path is NULL");
    }
    if (!path)
        return SNSR_RC_NO_MEMORY;
    if (!parentPath) {
        strcpy(path, name);
    } else {
        strcpy(path, parentPath);
        strcat(path, ".");
        strcat(path, name);
    }
    if (r != SNSR_RC_OK)
        snsrClearRC(s);

    /* Enable only for intents printf("NLU: %s = %s\n", path, value); */

    r = snsrForEach(s, SNSR_NLU_SLOT_LIST, snsrCallback(nluEvent, NULL, path));
    free(path);
    return r;
}

/* Print SDK version */
int sensoryInfo() {
    const char *str;
    printf("\n");
    printf("Sensory TrulyNatural Recognition Demo\n");
    snsrGetString(NULL, SNSR_LIBRARY_INFO, &str);
    printf("%s\n", str);
    snsrRelease(str);
    printf("©2024 Sensory Inc. All rights reserved.\n");
    printf("\n");
    return 1;
}

/* Release the session. */
int sensoryClose(SnsrSession s) {
    snsrRelease(s);
    return 1;
}

static void paramSetInt(SnsrSession s, const char *param, int val) {
    SnsrRC rc = snsrSetInt(s, param, val);
    if (rc != SNSR_RC_OK) {
        printf("WARNING: paramSetInt: Failed to set parameter: %s. %s\n", param,
                snsrRCMessage(rc));
        snsrClearRC(s);
    }
}

static void paramSetDouble(SnsrSession s, const char *param, double val) {
    SnsrRC rc = snsrSetDouble(s, param, val);
    if (rc != SNSR_RC_OK) {
        printf("WARNING: paramSetDouble: Failed to set parameter: %s. %s\n",
                param, snsrRCMessage(rc));
        snsrClearRC(s);
    }
}

static void paramPrintInt(SnsrSession s, const char *param) {
    int val;
    SnsrRC rc = snsrGetInt(s, param, &val);
    if (rc == SNSR_RC_OK) {
        printf("%s = %i\n", param, val);
    } else {
        printf("WARNING: paramPrintInt: Failed to get parameter: %s. %s\n",
                param, snsrRCMessage(rc));
        snsrClearRC(s);
    }
}

static void paramPrintDouble(SnsrSession s, const char *param) {
    double val;
    SnsrRC rc = snsrGetDouble(s, param, &val);
    if (rc == SNSR_RC_OK) {
        printf("%s = %f\n", param, val);
    } else {
        printf("WARNING: paramPrintDouble: Failed to get parameter: %s. %s\n",
                param, snsrRCMessage(rc));
        snsrClearRC(s);
    }
}

static void paramPrintString(SnsrSession s, const char *param) {
    const char *val;
    SnsrRC rc = snsrGetString(s, param, &val);
    if (rc == SNSR_RC_OK) {
        printf("%s = %s\n", param, val);
    } else {
        printf("WARNING: paramPrintString: Failed to get parameter: %s. %s\n",
                param, snsrRCMessage(rc));
        snsrClearRC(s);
    }
}


static void setHandler(SnsrSession s, const char *key, SnsrCallback c) {

    SnsrRC rc = SNSR_RC_OK;
    rc=snsrSetHandler(s, key, c);
    if (rc != SNSR_RC_OK) {
        printf("WARNING: setHandler %s. %s\n", key, snsrRCMessage(rc));
        snsrClearRC(s);
    } else
    {
        printf("Set handler passed \r\n");
    }
}


/* Initializes Sensory recognition session */
SnsrSession sensoryInit() {

    SnsrRC r = SNSR_RC_OK;
    SnsrSession s = NULL;

    /* Create SDK session */
    printf("Create SDK session \r\n");
    r = snsrNew(&s);
    if (r != SNSR_RC_OK) {
        printf("Creating Sensory instance failed\r\n");
        printf("ERROR XXX: %s\n", snsrRCMessage(r));
        sensoryClose(s);
        return NULL;
    }

    sensoryInfo();

    /* Load model */
    printf("Loading model: %s\r\n", modelName);

    // Loads from RAM
    extern SnsrCodeModel voicehub;
    r = snsrLoad(s, snsrStreamFromCode(voicehub));


    if (r != SNSR_RC_OK) {
        printf("Loading Sensory failed \r\n");
        printf("ERROR: (rc=%d) %s %s\r\n", r, snsrErrorDetail(s),
                snsrRCMessage(r));
        sensoryClose(s);
        CY_ASSERT(0);
        return NULL;
    }

    printf("Sensory model loaded \r\n");
    paramPrintString(s, SNSR_TASK_NAME);
    paramPrintString(s, SNSR_TASK_TYPE);

    // Configure parameters
    paramSetInt(s, SNSR_OPERATING_POINT, 15); // Operating point (range: 1-21) changes FA/FR behavior. Higher value => more FA, less FR
    paramSetDouble(s, SNSR_PARTIAL_RESULT_INTERVAL, 240); // partial result frequency
    paramSetDouble(s, SNSR_PUSH_DURATION_LIMIT, PUSH_DURATION_LIMIT); // limit max processing time (ms)
    paramSetInt(s, SNSR_PUSH_BUFFER_SIZE, 10*16000*2); // internal audio buffer (10 secs)


    // Configure handlers
    setHandler(s, SNSR_LISTEN_BEGIN_EVENT, snsrCallback(listenBeginEvent, NULL, NULL));
    setHandler(s, SNSR_LISTEN_END_EVENT, snsrCallback(listenEndEvent, NULL, NULL));
    setHandler(s, SNSR_RESULT_EVENT, snsrCallback(finalResultEvent, NULL, NULL));
    setHandler(s, SNSR_PARTIAL_RESULT_EVENT, snsrCallback(partialResultEvent, NULL, NULL));
    setHandler(s, SNSR_NLU_SLOT_EVENT, snsrCallback(nluEvent, NULL, NULL));

    // Print parameters
    paramPrintInt(s, SNSR_OPERATING_POINT);
    paramPrintDouble(s, SNSR_PARTIAL_RESULT_INTERVAL);
    paramPrintDouble(s, SNSR_PUSH_DURATION_LIMIT);

    fprintf(stderr, "Init: RC: %i, MSG: %s\r\n", r, snsrErrorDetail(s));

    if (r == SNSR_RC_SETTING_NOT_FOUND)
        snsrClearRC(s);

    fprintf(stdout, "sensoryInit success \r\n"); fflush(stdout);

    // Print instructions
    const char* slotType =NULL;

    r=snsrGetString(s, "0:task-type", &slotType);
    if (!slotType || !strcmp(slotType, "phrasespot")) {
        printf("Say Wake Word\r\n");
    } else {
        printf("Say Command\r\n");
    }

    snsrClearRC(s);

    return s;
}


/* Performs recognition on specified audio file.
 Audio is fed into recognizer in chunks to approximate streaming behavior.
 */

int sensoryRecog(SnsrSession s, int16_t *audio_data, int32_t length) {
    SnsrRC r = SNSR_RC_OK;
    uint32_t start, end;
    start = end = 0;

    time_t seconds;

    cy_rtos_get_time(&start_time_ms);
    r = snsrPush(s, SNSR_SOURCE_AUDIO_PCM, audio_data, length);

    if (r != SNSR_RC_OK) {
        printf("Failed to push ERROR: (rc=%d) %s\n", r, snsrErrorDetail(s));
        snsrReset(s); // reset and start over
        return 0;
    }

    cy_rtos_get_time(&stop_time_ms);
    uint32_t elapsed = (stop_time_ms-start_time_ms);

    seconds=time(NULL);

    if (elapsed > 13) { // print duration of frames that take longer than 13ms.
        //printf("_%lu_, ", elapsed);
    }
    if (elapsed > tnlPushHigh)
        tnlPushHigh = elapsed;

    int backlog = 0;
    r = snsrGetInt(s, SNSR_RES_PUSH_BUFFER_BACKLOG, &backlog);
    backlog = backlog / 32; // bytes to ms
    if (backlog > tnlAudioBufferHigh)
        tnlAudioBufferHigh = backlog;

    if (r != SNSR_RC_OK) {
        printf("ERROR: (rc=%d) %s\n", r, snsrErrorDetail(s));
        snsrReset(s); // reset and start over
        return 0;
    }

    return 1;
}

void sensory_processing(SnsrSession session, int16_t *audio_data,
        int32_t length) {

#ifdef SENSORY_PROFILE
    if (gotResult) {
        gotResult = 0;

        // NOTE: profiling is slow
        snsrProfile(session, snsrStreamFromFILE(stdout, SNSR_ST_MODE_WRITE));

        // RAM used
        size_t sz = snsrAllocPerfStats(NULL);
        sz *= 1.1; // NOTE: Allocate at least 10% larger to accommodate overhead
        printf("TNL RAM (estimate): %.0f bytes\n", (double) sz);

        // Print high-water marks useful for tuning TNL parameters.
        printf("TNL internal buffer (high-water): %i ms\n", tnlAudioBufferHigh);
        printf("TNL push duration (high-water): %lu ms\n==========\n", tnlPushHigh);
    }
#endif

    sensoryRecog(session, audio_data, length);

}

void sensory_deinit(SnsrSession session) {
    sensoryClose(session);
}

SnsrSession sensory_init() {

    if (mtb_ml_init(MTB_ML_PRIORITY) != MTB_ML_RESULT_SUCCESS)
    {
        printf("MTB ML Initialization failed!\r\n");
        CY_ASSERT(0);
    }

#ifdef SENSORY_PROFILE
    snsrConfig(SNSR_CONFIG_ALLOC, snsrAllocPerf(snsrAllocTLSF(sensoryHeap, sizeof(sensoryHeap))));  // Enables heap high-water mark tracking
#else
    snsrConfig(SNSR_CONFIG_ALLOC, snsrAllocTLSF(sensoryHeap, sizeof(sensoryHeap)));
#endif

    snsrConfig(SNSR_CONFIG_CLOCK_FUNC, clockFunc, 1e6); // clock function used to track processing time (ms)

    SnsrSession session = sensoryInit();
    return session;
}


void snsr_command_to_id (const char *command)
{
    int map_id=0;

    if (!strcmp(command,INF_WAKE_WORD))
    {
        map_id = WAKEWORD_CMD_ID;
    }
    else if (!strcmp(command,INF_PLAY_MUSIC))
    {
        map_id = PLAY_MUSIC_CMD_ID;
    }
    else if (!strcmp(command,INF_STOP_MUSIC))
    {
        map_id = STOP_MUSIC_CMD_ID;
    }
    else if (!strcmp(command,INF_INCREASE_VOLUME))
    {
        map_id = INCREASE_VOL_CMD_ID;
    }
    else if (!strcmp(command,INF_DECREASE_VOLUME))
    {
        map_id = DECREASE_VOL_CMD_ID;
    }
    else if (!strcmp(command,INF_NEXT_TRACK))
    {
        map_id = NEXT_TRACK_CMD_ID;
    }
    else if (!strcmp(command,INF_PREVIOUS_TRACK))
    {
        map_id = PREVIOUS_TRACK_CMD_ID;
    }
    else if (!strcmp(command, INF_TIMEOUT))
    {
        map_id = INFERENCING_TIMEOUT;
    }


#ifdef BATTERY_POWERED_LOCAL_VOICE
    battery_powered_post_processing(map_id);
#elif defined (MAINS_POWERED_LOCAL_VOICE) || defined (DEBUG_TUNE_MAINS_APP)
    mains_powered_post_processing(map_id);
#endif /* BATTERY_POWERED_LOCAL_VOICE/MAINS_POWERED_LOCAL_VOICE */

}


