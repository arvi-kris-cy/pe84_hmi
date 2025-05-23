/******************************************************************************
* File Name: ifx_itsi.c
*
* Description: This file contains functions to initialize and run frame by frame
*              Infineon ITSI process functions.
*
*
*******************************************************************************
* (c) 2024, Infineon Technologies Company. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Infineon Technologies Company (Infineon) or one of its
* subsidiaries and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Infineon hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Infineon's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Infineon.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Infineon
* reserves the right to make changes to the Software without notice. Infineon
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Infineon does
* not authorize its products for use in any products where a malfunction or
* failure of the Infineon product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Infineon's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Infineon against all liability.
*******************************************************************************/


/*******************************************************************************
* Include header file
******************************************************************************/
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "ifx_pre_post_process.h"
#include "ifx_sp_common_priv.h"
#include "ifx_sp_utils.h"
#include "ifx_am_predict.h"
#include "ifx_itsi_config_prms.h"
#include "ifx_itsi_prms.h"
#include "ifx_itsi.h"
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
#include "cy_profiler.h"
int cycle0 = 0, cycle1 = 0, cycle2 = 0, cycle3 = 0, cycle4 = 0, cycle5 = 0;
int sod_fm = 0, featurefm = 0, wwdfm = 0, cmdfm = 0;
#endif

/* Application team is responsible to implementation push buttom and set global variable ptt_flag */
bool ifx_push_to_talk_enable()
{/* if buttom is pushed, ptt_flag = true (i.e. 1), otherwise false (i.e. 0) */
    return ptt_flag;
}

//////////////////////// MTB ML ////////////////////////

#if !defined(_MSC_VER) && defined(USE_MTB_ML)
#include "mtb_ml.h"
#include "mtb_ml_model_16x8.h"
#include "cy_result.h"
#include "ITSI_AM_LSTM_tflm_model_int16x8.h"
#define NPU_PRIORITY 3

/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Model structure */
static mtb_ml_model_16x8_t* itsi_model_obj;

static mtb_ml_model_bin_t itsi_model_bin = { MTB_ML_MODEL_BIN_DATA(ITSI_AM_LSTM) };

/* Following itsi_tensor_arena has been counted as part of persistent memory total szie */
/* Tensor_arena buffer must be in SOCMEM and aligned by 16 which are required by U55 */
static uint8_t itsi_tensor_arena[ITSI_AM_LSTM_ARENA_SIZE] __attribute__((aligned(16)))
                                          __attribute__((section(".cy_socmem_data")));

static mtb_ml_model_buffer_t itsi_model_buffer =
{
    .tensor_arena = itsi_tensor_arena,
    .tensor_arena_size = ITSI_AM_LSTM_ARENA_SIZE
};

static int16_t data_feed_int[N_SEQ * FEATURE_BUF_SZ] __attribute__((aligned(16)));

static float mtb_ml_input_buffer[N_SEQ * FEATURE_BUF_SZ];
#endif

//////////////////////// ITSI Data Container Pointers ////////////////////////
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
void* pre_proc_hpf_obj = NULL;
#endif
void* sod_obj = NULL;
void* feature_obj = NULL;
void* dfww_obj = NULL;
void* dfcmd_obj = NULL;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
void* am_obj = NULL;
#endif

#ifdef EMBEDDED_NO_FILE
#undef SAVE_FEATURES
#undef SAVE_LL_SCORES_TO_FILE
#else
typedef char modelmem_t;
modelmem_t WWmodeldata[MAX_WW_MODEL_SIZE];
modelmem_t CMDmodeldata[MAX_CMD_MODEL_SIZE];
modelmem_t NMBmodeldata[MAX_NMB_MODEL_SIZE];
#endif // EMBEDDED_NO_FILE

#if defined(AM_LOOKBACK)
/* To avoid compiler warning, AM_LOOKBACK is redefined below because it is coverted from float number. Please make sure this
   redefination deos not change its value. */
#undef AM_LOOKBACK
#define AM_LOOKBACK 5
#else
#define AM_LOOKBACK 5
#endif
#ifdef _MSC_VER
float xIn[FRAME_SIZE_16K];
float features[FEATURE_BUF_SZ];
float output_scores[(N_PHONEMES +1) * AM_LOOKBACK];
#else
float xIn[FRAME_SIZE_16K] __attribute__((section(".itsi_socmem_data3")));
float features[FEATURE_BUF_SZ] __attribute__((section(".itsi_socmem_data4")));
float output_scores[(N_PHONEMES +1) * (1 + AM_LOOKBACK)] __attribute__((section(".itsi_socmem_data5")));
#endif

ifx_scratch_mem_t itsi_scratch;   //ITSI scratch memory
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
#ifdef _MSC_VER
float am_buffer[AM_OUT_SZ_FOR_BUFFER * N_SEQ];
float am_buffer_copy[AM_OUT_SZ_FOR_BUFFER * N_SEQ];
#else
float am_buffer[AM_OUT_SZ_FOR_BUFFER * N_SEQ] __attribute__((section(".cy_itcm")));
float am_buffer_copy[AM_OUT_SZ_FOR_BUFFER * N_SEQ] __attribute__((section(".cy_itcm")));
#endif
#endif

int sodtrigcnt;
int32_t WWDresult; // -2 = init (or reset), -1 = no (i.e. rejected), 1 = yes (i.e. detected), 0 = indecision
int32_t CMDresult; // -2 = init (or reset), -1 = no (i.e. rejected), 1 = yes (i.e. detected), 0 = indecision
int32_t feature_buf_cnt;

// for debugging
#ifdef ITSI_DEBUG
int frame_cnt;
int SODcnt;
#endif

#ifdef SAVE_FEATURES
extern FILE* features_bin_file;
extern int num_features_hops;
#endif

#if !defined(_MSC_VER) && defined(USE_MTB_ML)
/*******************************************************************************
* Function Name: mtb_ml_inference_init
********************************************************************************
* Summary:
*  Initializes the neural network.
*
* Parameters:
*  None
*
* Returns:
*  The status of the initialization.
*
*******************************************************************************/
cy_rslt_t mtb_ml_inference_init(void)
{
    cy_rslt_t result;

    /* Initialize the Neural Network */
    result = mtb_ml_model_16x8_init(&itsi_model_bin, &itsi_model_buffer, itsi_model_obj);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }
    result = mtb_ml_model_16x8_rnn_reset_all_parameters(itsi_model_obj);
    if (CY_RSLT_SUCCESS != result)
    {
        return result;
    }
    itsi_model_obj->profiling = MTB_ML_PROFILE_DISABLE;
    /* Set the priority of NPU interrupt handler */
    mtb_ml_init(NPU_PRIORITY);

    return result;
}
#endif

#ifndef EMBEDDED_NO_FILE
int read_itsi_models(char* itsi_ww_model_file_path, char* itsi_cmd_model_file_path, char* itsi_nmb_model_file_path)
{
    if (itsi_ww_model_file_path != NULL)
    {
        FILE* fmodel = fopen(itsi_ww_model_file_path, "rb");
        if (fmodel == NULL)
        {
            printf("Could not read %s ITSI WW model.\n", itsi_ww_model_file_path);
            return -1;
        }
        fread((modelmem_t*)WWmodeldata, sizeof(char), MAX_WW_MODEL_SIZE, fmodel);
        fclose(fmodel);

        if (itsi_cmd_model_file_path != NULL)
        {
            FILE* fcmdmodel = fopen(itsi_cmd_model_file_path, "rb");
            if (fcmdmodel == NULL)
            {
                printf("Could not read %s ITSI CMD model.\n", itsi_cmd_model_file_path);
                return -1;
            }
            fread((modelmem_t*)CMDmodeldata, sizeof(char), MAX_CMD_MODEL_SIZE, fcmdmodel);
            fclose(fcmdmodel);
        }

        if (itsi_nmb_model_file_path != NULL)
        {
            FILE* fnmbmodel = fopen(itsi_nmb_model_file_path, "rb");
            if (fnmbmodel == NULL)
            {
                printf("Could not read %s ITSI NMB model.\n", itsi_nmb_model_file_path);
                return -1;
            }
            fread((modelmem_t*)NMBmodeldata, sizeof(char), MAX_NMB_MODEL_SIZE, fnmbmodel);
            fclose(fnmbmodel);
        }
    }

    return 0;
}
#endif

void ifx_reset_feature_buf(void)
{
#if !defined(_MSC_VER) && defined(USE_MTB_ML)
    memset(mtb_ml_input_buffer, 0, sizeof(float) * (FEATURE_BUF_SZ * N_SEQ));
#else
    memset(am_buffer, 0, sizeof(float) * (AM_OUT_SZ_FOR_BUFFER * N_SEQ));
    memset(am_buffer_copy, 0, sizeof(float) * (AM_OUT_SZ_FOR_BUFFER * N_SEQ));
#endif
}

#ifdef EMBEDDED_NO_FILE
uint32_t ifx_init_itsi(const char* ww_model_ptr, const char* cmd_model_ptr, const char* nmb_model_ptr)
#else
uint32_t ifx_init_itsi(char* itsi_am_prms_file_path, char* itsi_am_wts_file_path, char* its_am_bias_file_path,
    char* itsi_ww_model_file_path, char* itsi_cmd_model_file_path, char* itsi_nmb_model_file_path)
#endif
{
    int ErrorHdl = 0;
    ifx_stc_pre_post_process_info_t pre_proc_hpf_info;
    ifx_stc_pre_post_process_info_t sod_info;
    ifx_stc_pre_post_process_info_t feature_info;
    ifx_stc_pre_post_process_info_t dfww_info;
    ifx_stc_pre_post_process_info_t dfcmd_info;
    int32_t scrtach_sz, am_predict_sz, persistent_mem_size = 0;
    (void)persistent_mem_size;
    /* Step 1: Parse and get required memory from configurations */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    ErrorHdl = ifx_pre_post_process_parse(pre_proc_hpf_prms, &pre_proc_hpf_info);
#endif
    ErrorHdl = ifx_pre_post_process_parse(sod_prms, &sod_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    mem_info_t am_mem_info;
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = ifx_am_predict_calculate_persistent_mem_size(NULL, NULL, NULL, &am_predict_sz);
#else
    ErrorHdl = ifx_am_predict_calculate_persistent_mem_size(itsi_am_prms_file_path, itsi_am_wts_file_path, its_am_bias_file_path, &am_predict_sz);
#endif
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    persistent_mem_size = am_predict_sz;
    am_mem_info.persistent_mem = am_predict_sz;
    am_mem_info.persistent_mem_pt = (char*)malloc(am_mem_info.persistent_mem);
    if (am_mem_info.persistent_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_AMPREDICT, IFX_SP_ENH_ERR_PERSISTEMT_MEM);
    }
    scrtach_sz = ifx_am_predict_calculate_scratch_mem_size();
    am_mem_info.scratch_mem = scrtach_sz;
#elif !defined(_MSC_VER) && defined(USE_MTB_ML)
    //AM NN model target for U55
    am_predict_sz = sizeof(mtb_ml_model_16x8_t);
    itsi_model_obj = (mtb_ml_model_16x8_t*)malloc(am_predict_sz);
    if (itsi_model_obj == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_AMPREDICT, IFX_SP_ENH_ERR_PERSISTEMT_MEM);
    }
    /* MTB-ML global memory should be counted as persistent memory */
    am_predict_sz += ITSI_AM_LSTM_ARENA_SIZE + sizeof(mtb_ml_model_buffer_t) + N_SEQ * FEATURE_BUF_SZ + N_SEQ * FEATURE_BUF_SZ * sizeof(float);
    ErrorHdl = mtb_ml_inference_init();
    if (ErrorHdl) return ErrorHdl;
#if defined(ENABLE_TRACE)
    printf("MTB-ML U55 init done with status = %x.\r\n", ErrorHdl);
#endif
    /* Above persistent memory has already been defined as global buffers so no need to allocate here */
    persistent_mem_size = am_predict_sz;
    scrtach_sz = 0;
#endif
    ErrorHdl = ifx_pre_post_process_parse(denoise_prms, &feature_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_pre_post_process_parse(dfww_prms, &dfww_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_pre_post_process_parse(dfcmd_prms, &dfcmd_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;

    /* Step 2: Allocate memory (AMPREDICT persistent memory allocated in previous step) */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    pre_proc_hpf_info.memory.persistent_mem_pt = (char*)malloc(pre_proc_hpf_info.memory.persistent_mem);
#else
    pre_proc_hpf_info.memory.persistent_mem_pt = NULL;
    pre_proc_hpf_info.memory.persistent_mem = 0;
    pre_proc_hpf_info.memory.scratch_mem = 0;
#endif
    sod_info.memory.persistent_mem_pt = (char*)malloc(sod_info.memory.persistent_mem);
    feature_info.memory.persistent_mem_pt = (char*)malloc(feature_info.memory.persistent_mem);
    dfww_info.memory.persistent_mem_pt = (char*)malloc(dfww_info.memory.persistent_mem);
    dfcmd_info.memory.persistent_mem_pt = (char*)malloc(dfcmd_info.memory.persistent_mem);
    if (dfcmd_info.memory.persistent_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_POST_PROCESS_IP_COMPONENT_DFCMD, IFX_SP_ENH_ERR_PERSISTEMT_MEM);
    }
    persistent_mem_size += feature_info.memory.persistent_mem + dfww_info.memory.persistent_mem + dfcmd_info.memory.persistent_mem +
                              pre_proc_hpf_info.memory.persistent_mem + sod_info.memory.persistent_mem;

    /* Get ITSI maximum scratch memory size */
    scrtach_sz = MAX(pre_proc_hpf_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(sod_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(feature_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(dfww_info.memory.scratch_mem, scrtach_sz);
    scrtach_sz = MAX(dfcmd_info.memory.scratch_mem, scrtach_sz);
    /* Allocate total scratch memory & shared with all components */
    feature_info.memory.scratch_mem_pt = (char*)malloc(scrtach_sz);
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    am_mem_info.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
#endif
    itsi_scratch.scratch_pad = feature_info.memory.scratch_mem_pt;
    itsi_scratch.scratch_size = scrtach_sz;
    itsi_scratch.scratch_cnt = 0;
    dfww_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    dfcmd_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    pre_proc_hpf_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    sod_info.memory.scratch_mem_pt = feature_info.memory.scratch_mem_pt;
    if (feature_info.memory.scratch_mem_pt == NULL)
    {
        return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_DENOISE, IFX_SP_ENH_ERR_SCRATCH_MEM_NULL);
    }

    /* Step 3: Initialize itsi containers/objects */
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    ErrorHdl = ifx_pre_post_process_init(pre_proc_hpf_prms, &pre_proc_hpf_obj, &pre_proc_hpf_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif
    ErrorHdl = ifx_pre_post_process_init(sod_prms, &sod_obj, &sod_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_pre_post_process_init(denoise_prms, &feature_obj, &feature_info);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = itsi_dfwwd_init(ww_model_ptr, &dfww_obj, &(dfww_info.memory), dfww_prms[DFWW_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = itsi_dfcmd_init(cmd_model_ptr, nmb_model_ptr, &dfcmd_obj, &(dfcmd_info.memory), dfcmd_prms[DFCMD_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#else
    ErrorHdl = read_itsi_models(itsi_ww_model_file_path, itsi_cmd_model_file_path, itsi_nmb_model_file_path);
    if (ErrorHdl)
    {
        printf("ITSI read model failed, returning");
        return ErrorHdl;
    }
    ErrorHdl = itsi_dfwwd_init(WWmodeldata, &dfww_obj, &(dfww_info.memory), dfww_prms[DFWW_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = itsi_dfcmd_init(CMDmodeldata, NMBmodeldata, &dfcmd_obj, &(dfcmd_info.memory), dfcmd_prms[DFCMD_PARM_INDEX]);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif

#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    // AM NN model target for M55
#ifdef EMBEDDED_NO_FILE
    ErrorHdl = ifx_am_predict_init(&am_obj, &am_mem_info, FEATURE_BUF_SZ, N_SEQ, FRAMES_HOP, N_PHONEMES,
        NULL, NULL, NULL);
#else
    ErrorHdl = ifx_am_predict_init(&am_obj, &am_mem_info, FEATURE_BUF_SZ, N_SEQ, FRAMES_HOP, N_PHONEMES,
        itsi_am_prms_file_path, itsi_am_wts_file_path, its_am_bias_file_path);
#endif
    if (ErrorHdl) return ErrorHdl;
#endif
#if defined(ENABLE_TRACE)
    printf("ITSI persistent memory size: HPF=%d, SOD=%d, FEATURE=%d, AM Predict=%d, WW=%d, CMD=%d, Total=%d; Scratch total=%d\r\n", pre_proc_hpf_info.memory.persistent_mem,
        sod_info.memory.persistent_mem, feature_info.memory.persistent_mem, am_predict_sz, dfww_info.memory.persistent_mem, dfcmd_info.memory.persistent_mem,
        persistent_mem_size, scrtach_sz);
#endif
    printf("ITSI persistent memory size: HPF=%d, SOD=%d, FEATURE=%d, AM Predict=%d, WW=%d, CMD=%d, Total=%d; Scratch total=%d\r\n", pre_proc_hpf_info.memory.persistent_mem,
        sod_info.memory.persistent_mem, feature_info.memory.persistent_mem, am_predict_sz, dfww_info.memory.persistent_mem, dfcmd_info.memory.persistent_mem,
        persistent_mem_size, scrtach_sz);
    ErrorHdl = ifx_reset_itsi();
    return ErrorHdl;
}

uint32_t ifx_reset_itsi(void)
{
    int ErrorHdl = 0;

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    //ErrorHdl = ifx_afe_hpf_reset(pre_proc_hpf_obj);
    //if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
#endif
    ErrorHdl = speech_utils_sod_reset(sod_prms, sod_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_itsi_feature_reset(feature_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_dfww_state_reset(dfww_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    ErrorHdl = ifx_dfcmd_state_reset(dfcmd_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;

    ifx_reset_feature_buf();
    // Clear output_scores buffer
    memset(output_scores, 0, sizeof(float) * ((N_PHONEMES + 1) * AM_LOOKBACK));

    // counters and flags
    sodtrigcnt = -100;
    WWDresult = -2; // initial non-WWD state
    CMDresult = -2; // initial non-CMD state
    feature_buf_cnt = 0;

#ifdef ITSI_DEBUG
    frame_cnt = 0;
    SODcnt = 0;
#endif

    return IFX_SP_ENH_SUCCESS;
}

uint32_t ifx_free_itsi()
{
    int ErrorHdl = 0;

    // free ITSI persistent memory
#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    free(pre_proc_hpf_obj);
    pre_proc_hpf_obj = NULL;
#endif
    free(sod_obj);
    sod_obj = NULL;
    ErrorHdl = itsi_feature_deinit(feature_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    free(feature_obj);
    feature_obj = NULL;
    free(dfww_obj);
    dfww_obj = NULL;
    free(dfcmd_obj);
    dfcmd_obj = NULL;
#if defined(_MSC_VER) || !defined(USE_MTB_ML)
    ErrorHdl = ifx_am_predict_deinit(am_obj);
    if (ErrorHdl != IFX_SP_ENH_SUCCESS) return ErrorHdl;
    free(am_obj);
    am_obj = NULL;
#elif !defined(_MSC_VER) && defined(USE_MTB_ML)
    free(itsi_model_obj);
    itsi_model_obj = NULL;
#endif
    /* free scratch memory */
    ifx_mem_reset(&itsi_scratch);
    free(itsi_scratch.scratch_pad);
    itsi_scratch.scratch_pad = NULL;
    return IFX_SP_ENH_SUCCESS;
}

uint32_t ifx_run_itsi(short* mic_frame, bool eof, int* ww_decison, int* cmd_decision)
{
    uint32_t ret;

    // run SOD
    bool is_sod_detected;
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_start();
#endif
    ret = ifx_sod_process((int16_t*)mic_frame, sod_obj, &is_sod_detected);
    if (ret != IFX_SP_ENH_SUCCESS) return ret;
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_stop();
    cycle0 += cy_profiler_get_cycles();
    sod_fm++;
#endif
#ifdef ITSI_DEBUG
    frame_cnt++;
#endif

    if (is_sod_detected)
    {
        if (CMDresult == -2)
        {
            if (WWDresult == -2)
            {
                sodtrigcnt = 0;
            }
            WWDresult = 0;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
#ifdef ITSI_DEBUG
        SODcnt++;
#if defined(ENABLE_TRACE)
        printf("ITSI SOD Detected: %d\n", SODcnt);
#endif
#endif
    }

#ifdef ENABLE_IFX_PRE_PROCESS_HPF
    /* run pre_process_hpf */
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_start();
#endif
    ifx_time_pre_process(mic_frame, NULL, pre_proc_hpf_obj, IFX_PRE_PROCESS_IP_COMPONENT_HPF, mic_frame, NULL);
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_stop();
    cycle1 += cy_profiler_get_cycles();
#endif
#endif

#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_start();
#endif
    /* do features and AM every frame */
    for (int i = 0; i < FRAME_SIZE_16K; i++)
    {
        xIn[i] = (float)mic_frame[i];
    }

    //2. Compute Features
    ret = itsi_feature_process_frame(xIn, feature_obj, features);
    if (ret != IFX_SP_ENH_SUCCESS)
    {
        return ret;
    }
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
    cy_profiler_stop();
    cycle2 += cy_profiler_get_cycles();
#endif
#ifdef SAVE_FEATURES
    fwrite(features, sizeof(float), FEATURE_BUF_SZ, features_bin_file);
    num_features_hops++;
#endif

    //3. inference
#if !defined(_MSC_VER) && defined(USE_MTB_ML)
    float* buf_pt_base = mtb_ml_input_buffer;
    int i;
    // update input buffer
    int hop = 1;
    int total_values = N_SEQ * FEATURE_BUF_SZ;
    int value_to_move = (N_SEQ - hop) * FEATURE_BUF_SZ;
    int offset = hop * FEATURE_BUF_SZ;
    for (i = offset; i < total_values; i++)
    {
        buf_pt_base[i - offset] = buf_pt_base[i];
    }
    for (i = 0; i < offset; i++)
    {
        buf_pt_base[i + value_to_move] = features[i];
    }

    feature_buf_cnt++;
    if (feature_buf_cnt == FRAMES_HOP)
    {
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_start();
#endif
        /* shift the output_scores */
        for (i = (N_PHONEMES + 1) * AM_LOOKBACK - 1; i >= (N_PHONEMES + 1); i--)
        {
            output_scores[i] = output_scores[i - (N_PHONEMES + 1)];
        }

        int16_t* qntz_out = data_feed_int;
        float val;
        float factor = 1.0f / itsi_model_obj->input_scale;
        i = itsi_model_obj->input_size;
#if defined(COMPONENT_CMSIS_DSP)
        while (i > 0)
        {
            val = (*buf_pt_base++) * factor + itsi_model_obj->input_zero_point;
            val += val > 0.0f ? 0.5f : -0.5f; /* Rounding make it more close to float model */
            *qntz_out++ = (int16_t)(__SSAT((int32_t)(val), 16));
            i--;
        }
#else
        while (i > 0)
        {
            val = (*buf_pt_base++) * factor + itsi_model_obj->input_zero_point;
            val += val > 0.0f ? 0.5f : -0.5f; /* Rounding make it more close to float model */
            if ((int32_t)val > SHRT_MAX)
            {
                *qntz_out++ = SHRT_MAX;
            }
            else if ((int32_t)val < SHRT_MIN)
            {
                *qntz_out++ = SHRT_MIN;
            }
            else
            {
                *qntz_out++ = (int16_t)(val);
            }
            i--;
}
#endif /* COMPONENT_CMSIS_DSP */

        ret = mtb_ml_model_16x8_rnn_reset_all_parameters(itsi_model_obj);
        if (CY_RSLT_SUCCESS != ret)
        {
            return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_AMPREDICT, IFX_SP_ENH_ERR_INFERERCE);
        }
        /* Feed the Model */
        ret = mtb_ml_model_16x8_run(itsi_model_obj, data_feed_int);
        if (CY_RSLT_SUCCESS != ret)
        {
            return IFX_SP_ENH_ERROR(IFX_PRE_PROCESS_IP_COMPONENT_AMPREDICT, IFX_SP_ENH_ERR_INFERERCE);
        }

        // convert result_buffer to output_scores
        {//convert inference output to float32
            int offset = itsi_model_obj->output_zero_point;
            factor = itsi_model_obj->output_scale;
            for (i = 0; i < itsi_model_obj->output_size; i++)
            {
                output_scores[i] = ((int)(itsi_model_obj->output[i]) - offset) * factor;
            }
        }

        feature_buf_cnt = 0;
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_stop();
        featurefm++;
        cycle3 += cy_profiler_get_cycles();
#endif
    }
#else
    ret = ifx_am_predict_run(am_obj, &itsi_scratch, features, &feature_buf_cnt, output_scores, am_buffer_copy, eof);
    if (ret != IFX_SP_ENH_SUCCESS) return ret;
#endif

    // post-inference processing
    * ww_decison = WWDresult;
    if ((WWDresult == 0) && (sodtrigcnt == 0))
    {// this has to be called every FRAMES_HOP !!
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_start();
#endif
        WWDresult = ifx_push_to_talk_enable(); /* ifx_push_to_talk_enable() return 1 to enable push to talk(to detect Command). */
        if (WWDresult == 0)
        {
            ret = ifx_wwd(dfww_obj, &output_scores[(N_PHONEMES + 1) * (AM_LOOKBACK - 1)], &WWDresult);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
        *ww_decison = WWDresult;
        if (WWDresult == 1)
        {
            WWDresult = -2;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
            ret = ifx_reset_dfcmd(dfcmd_obj, -1);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
            CMDresult = 0;
        }
        else if (WWDresult == -1)
        {
            WWDresult = -2;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_stop();
        cycle4 += cy_profiler_get_cycles();
        wwdfm++;
#endif
    }

    // post-WWD CMD processing
    *cmd_decision = CMDresult;
    if ((CMDresult == 0) && (sodtrigcnt == 0))
    {// this has to be called every FRAMES_HOP !!
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_start();
#endif
        ret = ifx_dfcmd(dfww_obj, dfcmd_obj, &output_scores[(N_PHONEMES +1) * (AM_LOOKBACK - 2)], &CMDresult);
        *cmd_decision = CMDresult;
        if (ret != IFX_SP_ENH_SUCCESS) return ret;
        if (CMDresult == 1)
        {
            CMDresult = -2;
        }
        if (CMDresult == -1)
        {
            CMDresult = -2;
            ret = ifx_reset_dfww(dfww_obj);
            if (ret != IFX_SP_ENH_SUCCESS) return ret;
        }
#if defined(DETAILED_PROFILE) && !defined(_MSC_VER)
        cy_profiler_stop();
        cycle5 += cy_profiler_get_cycles();
        cmdfm++;
#endif
    }
    sodtrigcnt++;
    if (sodtrigcnt == FRAMES_HOP)
    {/* this trigger DFWW/DFCMD to be called every FRAMES_HOP !! */
        sodtrigcnt = 0;
    }

    /* flush out the unused softmax, just in case the file doesn't have enough noise at the end */
    if (eof)
    {
        for (int i = 0; i < AM_LOOKBACK - 2; i++)
        {
            if (CMDresult == 0)
            {
                ret = ifx_dfcmd(dfww_obj, dfcmd_obj, &output_scores[(N_PHONEMES + 1) * (AM_LOOKBACK - 3 - i)], &CMDresult);
                *cmd_decision = CMDresult;
                if (ret != IFX_SP_ENH_SUCCESS) return ret;
                if (CMDresult == 1)
                {
                    CMDresult = -2;
#if defined(ENABLE_TRACE)
                    printf("Detect a CMD at EOF!\n");
#endif
                }
                if (CMDresult == -1)
                {
                    CMDresult = -2;
                    ret = ifx_reset_dfww(dfww_obj);
                    if (ret != IFX_SP_ENH_SUCCESS) return ret;
                }
            }
        }
    }
    return 0;
}
